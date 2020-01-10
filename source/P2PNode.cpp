#include <iostream>
#include <utility>
#include <thread>
#include "P2PNode.h"
#include <netinet/in.h>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>
#include <vector>
#include <arpa/inet.h>
#include <sstream>
#include <cstring>
#include <stdexcept>

#include <LocalSystemHandler.h>
#include <algorithm>

P2PNode::P2PNode(int tcpPort, LocalSystemHandler& handler) : tcpPort(tcpPort), handler(handler) {

    handler.setDefaultWorkspace();
    std::vector<std::string> filesNamesToReUpload = handler.getPreviousState();

    for(auto fileName : filesNamesToReUpload) {
        size_t fileSize = handler.getFileSize(fileName);
        File restoredFile(fileName, handler.getUserName(), fileSize);
        localFiles.addFile(restoredFile);
    }

    startBroadcastingFiles();
    startReceivingBroadcastingFiles();
}

ActionResult P2PNode::uploadFile(std::string uploadFileName) {

    if(handler.addFileToLocalSystem(uploadFileName) != FILE_SUCCESS){
        return ACTION_FAILURE;
    }

    std::string systemFileName = handler.getLastTokenOf(uploadFileName);
    size_t fileSize = handler.getFileSize(uploadFileName);
    File file(systemFileName, handler.getUserName(), fileSize);
    // nie jest potrzebna tutaj synchronizacja,
    // poniewaz ten sam watek dodaje i usuwa pliki
    AddFileResult ret = localFiles.addFile(file);
    if (ret != ADD_SUCCESS) {
        return ACTION_FAILURE;
    }

    if(handler.updateConfig(systemFileName, CONFIG_ADD) != FILE_SUCCESS){
        return ACTION_FAILURE;
    }

    std::cout << "Plik " << uploadFileName << " zostal dodany do systemu\n";
    return ACTION_SUCCESS;
}

ActionResult P2PNode::showLocalFiles() {
    localFiles.print();
}

ActionResult P2PNode::removeFile(std::string fileName) {

    if(!handler.isNetworkFilePathCorrect(fileName)) {
        return ACTION_FAILURE;
    }
    size_t fileSize = handler.getFileSize(fileName);
    File tmp(fileName, handler.getUserName(), fileSize);

    if (localFiles.removeFile(tmp) != SUCCESS) {
        return ACTION_FAILURE;
    }

    globalFiles.revoke(tmp);
    globalFiles.addToFilesRevokedByMe(tmp);

    if(sendRevokeCommunicate(std::move(tmp)) != ACTION_SUCCESS){
        uploadFile(fileName);
        return ACTION_FAILURE;
    }

    if(handler.removeFileFromLocalSystem(fileName) != FILE_SUCCESS) {
        uploadFile(fileName);
        return ACTION_FAILURE;
    }

    return ACTION_SUCCESS;
}

ActionResult P2PNode::downloadFile(const std::string &) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::updateLocalFiles(void) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::showGlobalFiles(SHOW_GLOBAL_FILE_TYPE type) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::startBroadcastingFiles() {
    // Przygotuj się na broadcast
    ActionResult actionResult;
    if ((actionResult = prepareForBroadcast()) != ACTION_SUCCESS) {
        // jeśli niepowodzenie, zwróć rezultat
        return actionResult;
    }

    broadcast.sendThread = std::thread([this]() {
        int failuresCount = 0;
        while (true) {
            // Sprawdzenie warunku czy wychodzimy z petli
            broadcast.exitMutex.lock();
            if (broadcast.exit) {
                broadcast.exitMutex.unlock();
                break;
            }
            broadcast.exitMutex.unlock();

            std::vector<File> communicates = localFiles.getFiles();
            size_t filesLeftToBroadcast = communicates.size();
            size_t fileId = 0;
            std::cout << "Broadcast " << filesLeftToBroadcast << " plikow." << std::endl;

            while(filesLeftToBroadcast > 0) {

                size_t filesToSendNow = std::min(filesLeftToBroadcast, (size_t)FILES_IN_ONE_DATAGRAM_LIMIT);

                // wiadomosc ma postac
                // [bajt - 0, lub 1 - typ wiadomosci][ 4 bajty - rozmiar ][ 64 bajty nazwa wysylajacego ][ 136 bajtow ][ 136 bajtow ][ 136 bajtow ] ...
                uint8_t* broadcastMsg = (uint8_t* )malloc(filesToSendNow * sizeof(BroadcastStruct) + sizeof(char) + sizeof(size_t) + 64 * sizeof(char));
                if(broadcastMsg == nullptr){
                    broadcast.exitMutex.lock();
                    if (broadcast.exit) {
                        broadcast.exitMutex.unlock();
                        break;
                    }
                    broadcast.exitMutex.unlock();
                }
                // dla rozglaszania
                *broadcastMsg = '0';
                std::cout << "typ: " << *broadcastMsg << "|";
                // ilosc plikow
                size_t* sizeInfoAddress = (size_t *)(broadcastMsg + sizeof(char));
                *sizeInfoAddress = filesToSendNow;
                std::cout << "ilosc: " << *sizeInfoAddress << "|";
                // dodanie 1 do typu size_t doda 4 bajty ( taki rozmiar ma size_t )
                char* senderInfoAddress = (char* )(sizeInfoAddress + 1);

                // copies sender name
                strncpy(senderInfoAddress, handler.getUserName().c_str(), MAX_USERNAME_LEN - 1);
                std::cout << "wysylajacy: " << std::string(senderInfoAddress) << "|";

                BroadcastStruct* broadcastStructAddress = (BroadcastStruct* )((uint8_t* )senderInfoAddress + sizeof(size_t) + 64*sizeof(char));
                for(int i = 0; i < filesToSendNow; i++){

                    BroadcastStruct msg {};
                    strncpy(msg.name, communicates[fileId].getName().c_str(), B_FILENAME_LEN - 1);
                    strncpy(msg.owner, communicates[fileId].getOwner().c_str(), B_OWNER_NAME_LEN - 1);
                    // tymczasowo
                    msg.size = 0;
                    fileId ++;
                    std::cout << "wielkosc: " << msg.size << "| nazwa: "  << msg.name << "| wlasciciel: " << msg.owner << "||";

                    *broadcastStructAddress = msg;
                }
                std::cout << "\n";

                size_t fullBroadcastMsgSize = sizeof(broadcastMsg);
                while (send(broadcast.socketFd, broadcastMsg, fullBroadcastMsgSize, 0) < fullBroadcastMsgSize) {
                    if (++failuresCount > BROADCAST_FAILURE_LIMIT) {
                        while (prepareForBroadcast(true) != ACTION_SUCCESS) {
                            // Sprawdzenie warunku czy wychodzimy z petli
                            broadcast.exitMutex.lock();
                            if (broadcast.exit) {
                                broadcast.exitMutex.unlock();
                                break;
                            }
                            broadcast.exitMutex.unlock();

                            std::this_thread::sleep_for(std::chrono::seconds(broadcast.restartConnectionInterval));
                        }
                        failuresCount = 0;
                    }
                }

                free (broadcastMsg);
                filesLeftToBroadcast -= filesToSendNow;
            }
            sleep(BROADCAST_PERIOD);
        }
    });

    // wątek 'łapany' w destruktorze
    return ACTION_SUCCESS;
}

P2PNode::~P2PNode() {
    //OBSLUGA BROADCASTU

    // Oznacz że chcę wyjśc z wątkow
    broadcast.exitMutex.lock();
    broadcast.exit = true;
    broadcast.exitMutex.unlock();

    // Poczekaj aż broadczast się skończy
//    broadcast.sendThread.join();
//    broadcast.recvThread.join();

    // Zamknij gniazdo broadcastowe
    close(broadcast.socketFd);
}


void P2PNode::sendFile(int fileFD, int clientFD, unsigned long long offset, unsigned long long bytes) {

    char buff[4096*1024];
    unsigned long long bytesSent = 0;

    if (lseek(fileFD, offset, SEEK_SET) < 0){
        throw std::runtime_error("lseek failed\n");
    }

    while (bytesSent < bytes) {
        ssize_t bytesToRead = std::min(bytes - bytesSent, (unsigned long long)4096 * 1024);
        ssize_t bytesRead = read(fileFD, buff, bytesToRead);
        if (bytesRead < 0) {
            throw std::runtime_error("read failed\n");
        }
        bytesSent += bytesRead;

        if (write(clientFD, buff, bytesRead) < bytesRead) {
            throw std::runtime_error("write failed\n");
        }

    }

    close(clientFD);
    close(fileFD);

}

void P2PNode::handleDownloadRequests() {
// socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw std::runtime_error("socket creation failed...\n");
    }
    struct sockaddr_in servaddr{};

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(this->tcpPort);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) != 0) {
        throw std::runtime_error("socket bind failed...\n");
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, MAX_TCP_CONNECTIONS)) != 0) {
        throw std::runtime_error("Listen failed...\n");
    }

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {

        struct sockaddr_in cli{};
        socklen_t len = sizeof(cli);

        int clientFD = accept(sockfd, (sockaddr *) &cli, &len);
        if (clientFD < 0) {
            throw std::runtime_error("server acccept failed...\n");
        }

        char buff[sizeof(fileRequest)];

        if (recv(clientFD, buff, sizeof(buff), 0) <= 0) {
            throw std::runtime_error("recv() ERROR");
        }

        #pragma clang diagnostic push
        #pragma ide diagnostic ignored "OCDFAInspection"
        auto request = (fileRequest *) buff;
        #pragma clang diagnostic pop
        std::cout << request->fileName << " " << request->bytes << " " << request->offset << std::endl;

        // TODO moze local system handler bedzie ustawial prefix do workspace jako pole Node'a
        int fileFD;
        try {
            fileFD = handler.openFileFromWorkSpace(request->fileName);
        }
        catch(...){
            printf("Żądano pliku, który nie istnieje.\n");
            close(clientFD);
            continue;
        }


        std::cout << "Wysylam plik " << request->fileName << "\n";
        std::thread t1(&P2PNode::sendFile, this, fileFD, clientFD, request->offset, request->bytes);
        t1.detach();
    }
    #pragma clang diagnostic pop
}

// wowolujemy metode p2pnode.handleDownlaodRequests na nowym watku
// ta metoda tworzy socket i czeka na komunikat zadania pliku
// patrzymy czy jest plik o zadanej nazwie, jezeli tak to wywoluje metoda rozpoczecia przesylania na nowym watku (detached)
// jezeli nie to zamykamy socket klienta
// wywolujemy ponowanie waitForDownloadRequest i czekamy na nastepne zadanie pobierania
ActionResult P2PNode::startHandlingDownloadRequests() {
    std::thread t1(&P2PNode::handleDownloadRequests, this);
    t1.detach();
    return ACTION_SUCCESS;
}

void P2PNode::requestAndDownloadFileFragment(fileRequest request, std::string ip_addr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw std::runtime_error("socket creation failed...\n");
    }

    struct sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
    servaddr.sin_port = htons(tcpPort);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        throw std::runtime_error("connection with the server failed...\n");
    }

    // std::cout << sizeof(request);
    if(write(sockfd, &request, sizeof(request))  < sizeof(request)){
        throw std::runtime_error("write failed\n");
    }

    // std::cout << request.fileName << "\n";
    int fd = handler.createAndOpenFileInWorkspace(request.fileName);

    if(lseek(fd, request.offset, SEEK_SET) < request.offset){
        throw std::runtime_error("lseek failed\n");
    }
    // 4 MB
    char buff[4096*1024];
    unsigned long long bytesReceived = 0;
    while (bytesReceived < request.bytes) {
        ssize_t bytesToRead = std::min(request.bytes - bytesReceived, (unsigned long long)4096 * 1024);
        // tutaj jest mały problem, poniewaz jezeli nadawca nic nie wysyla (blad lub znika z sieci) to czekamy bez konca
        ssize_t bytesRead = read(sockfd, buff, bytesToRead);
        if (bytesRead < 0) {
            throw std::runtime_error("read failed\n");
        }
        bytesReceived += bytesRead;

        if (write(fd, buff, bytesRead) < bytesRead) {
            throw std::runtime_error("write failed\n");
        }
    }
}

ActionResult P2PNode::prepareForBroadcast(bool restart) {
    std::unique_lock<std::mutex> lk(broadcast.preparationMutex);

    if (restart) {
        close(broadcast.socketFd);
        broadcast.socketFd = -1;
    }

    int fd = broadcast.socketFd;
    if (fd >= 0) {
        return ACTION_SUCCESS;
    }

    // Otwórz gniazdo
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "Nie udalo sie otworzyc gniazda. Numer bledu: " << errno << std::endl;
        return ACTION_FAILURE;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(broadcast.UDP_BROADCAST_PORT);

    // Bind adres do gniazda
    if (bind(fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        close(fd);
        std::cout << "Nie udalo sie przypisac adresu do gniazda. Numer bledu: " << errno << std::endl;
        return ACTION_FAILURE;
    }

    this->broadcast.socketFd = fd;

    return ACTION_SUCCESS;
}

ActionResult P2PNode::sendRevokeCommunicate(const File file) {
    // Przygotuj się na broadcast
    ActionResult actionResult;
    if ((actionResult = prepareForBroadcast()) != ACTION_SUCCESS) {
        // jeśli niepowodzenie, zwróć rezultat
        return actionResult;
    }

    uint8_t* broadcastMsg = (uint8_t* )malloc( sizeof(BroadcastStruct)
            + sizeof(char) + sizeof(size_t) + 64 * sizeof(char));
    // dla revoke
    *broadcastMsg = '1';
    // ilosc plikow wynosi 1
    size_t* sizeInfoAddress = (size_t *)(broadcastMsg + sizeof(char));
    *sizeInfoAddress = 1;
    char* senderInfoAddress = (char* )(sizeInfoAddress + 1);

    // copies sender name
    strncpy(senderInfoAddress, handler.getUserName().c_str(), MAX_USERNAME_LEN - 1);

    BroadcastStruct* pRevokedFileAddress = (BroadcastStruct* )((uint8_t* )senderInfoAddress + 64 * sizeof(char));
    BroadcastStruct msg {};
    strncpy(msg.name, file.getName().c_str(), B_FILENAME_LEN - 1);
    strncpy(msg.owner, file.getOwner().c_str(), B_OWNER_NAME_LEN - 1);
    // tymczasowo
    msg.size = 0;
    *pRevokedFileAddress = msg;

    size_t msgSize = sizeof(broadcastMsg);
    if (send(broadcast.socketFd, broadcastMsg, msgSize, 0) < msgSize) {
        return ACTION_FAILURE;
    }
    return ACTION_SUCCESS;
}

ActionResult P2PNode::startReceivingBroadcastingFiles() {
    // Przygotuj się na broadcast
    ActionResult actionResult;
    if ((actionResult = prepareForBroadcast()) != ACTION_SUCCESS) {
        // jeśli niepowodzenie, zwróć rezultat
        return actionResult;
    }

    broadcast.recvThread = std::thread([this]() {
        // 256 - maks plikow w 1 wiadomosci
        // 132 - wielkosc jednej wiadomosci
        // 5 - jeden bajt na typ, 4 na ilosc danych
        // 64 na wysylajacego
        char buf[256 * sizeof(BroadcastStruct) + 5 + 64];
        unsigned long long bytesReceived = 0;
        while (true) {
            broadcast.exitMutex.lock();
            if (broadcast.exit) {
                broadcast.exitMutex.unlock();
                break;
            }
            broadcast.exitMutex.unlock();

            if (recv(broadcast.socketFd, buf, sizeof(buf), 0) < 0) {
                continue;
            }

            // typ komunikatu
            char type = buf[0];
            uint8_t* pBuf = (uint8_t* )buf;
            // miejsce rozmiaru
            size_t* pMsgSize = (size_t* )((uint8_t* )buf + sizeof(char));
            size_t msgNum = *pMsgSize;
            // sender
            char senderName[MAX_USERNAME_LEN];
            size_t* pSender = pMsgSize + 1;
            strncpy(senderName, (char* )pSender, MAX_USERNAME_LEN - 1);

            BroadcastStruct* currentStruct = (BroadcastStruct* )((uint8_t* )pSender + 64 * sizeof(char));
            for(int i = 0; i < msgNum; i++) {
                File tmp(std::move(std::string(currentStruct->name)),
                        std::move(std::string(currentStruct->owner)),
                        currentStruct->size);
                currentStruct++;

                if(type == '0') {
                    if (globalFiles.add(std::move(std::string(senderName)), std::move(tmp)) == ADD_GLOBAL_REVOKED) {
                        sendRevokeCommunicate(
                                std::move(tmp)); 
                    }
                    // warto sprawdzic co dla wartosci innych niz '0' i '1'
                } else {
                    globalFiles.revoke(tmp);
                    localFiles.removeFile(std::move(tmp));
                    updateLocalFiles();
                }
            }

        }
    });

    // wątek 'łapany' w destruktorze
    return ACTION_SUCCESS;
}




