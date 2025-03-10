#include <iostream>
#include <utility>
#include <thread>
#include "P2PNode.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>
#include <vector>
#include <arpa/inet.h>
#include <stdexcept>
#include "Logger.h"
#include <LocalSystemHandler.h>
#include <ifaddrs.h>

P2PNode::P2PNode(int tcpPort, LocalSystemHandler &handler) : tcpPort(tcpPort), handler(handler) {

    handler.setDefaultWorkspace();
    auto files = handler.getPreviousState();

    for (auto [name, owner, size] : files) {

        File restoredFile(name, owner, size);
        localFiles.addFile(restoredFile);
    }
    prepareForSendingBroadcast();
    prepareForReceivingBroadcast();
    startBroadcastingFiles();
    startReceivingBroadcastingFiles();
}

ActionResult P2PNode::uploadFile(const std::string &uploadFileName) {

    if (handler.addFileToLocalSystem(uploadFileName) != FILE_SUCCESS) {
        return ACTION_FAILURE;
    }

    std::string systemFileName = handler.getLastTokenOf(uploadFileName);
    size_t fileSize = handler.getFileSize(systemFileName);
    File file(systemFileName, handler.getUserName(), fileSize);
    // nie jest potrzebna tutaj synchronizacja,
    // poniewaz ten sam watek dodaje i usuwa pliki
    AddFileResult ret = localFiles.addFile(file);
    if (ret != ADD_SUCCESS) {
        return ACTION_FAILURE;
    }

    if (handler.addToConfig(file.getName(), file.getOwner(), file.getSize()) != FILE_SUCCESS) {
        return ACTION_FAILURE;
    }

    std::cout << "Plik " << uploadFileName << " zostal dodany do systemu\n";
    return ACTION_SUCCESS;
}

void P2PNode::showLocalFiles() {
    localFiles.print();
}

ActionResult P2PNode::removeFile(std::string fileName) {

    if (!handler.isNetworkFilePathCorrect(fileName)) {
        return ACTION_FAILURE;
    }
    size_t fileSize = handler.getFileSize(fileName);
    File tmp(fileName, handler.getUserName(), fileSize);

    if (localFiles.removeFile(tmp) != SUCCESS) {
        return ACTION_FAILURE;
    }

//    globalFiles.revoke(tmp);
//    globalFiles.addToFilesRevokedByMe(tmp);

    if (sendRevokeCommunicate(std::move(tmp)) != ACTION_SUCCESS) {
        uploadFile(fileName);
        return ACTION_FAILURE;
    }

    if (handler.removeFileFromLocalSystem(fileName) != FILE_SUCCESS) {
        uploadFile(fileName);
        return ACTION_FAILURE;
    }

    return ACTION_SUCCESS;
}

ActionResult P2PNode::updateLocalFiles(void) {
    return ACTION_NOT_HANDLED;
}

void P2PNode::showGlobalFiles() {
    globalFiles.showFiles();
}


P2PNode::~P2PNode() {
    //OBSLUGA BROADCASTU

    // Oznacz że chcę wyjśc z wątkow
    broadcast.exitMutex.lock();
    broadcast.exit = true;
    broadcast.exitMutex.unlock();

    // Poczekaj aż broadczast się skończy
    broadcast.sendThread.join();
    broadcast.recvThread.join();

    // Zamknij gniazdo broadcastowe
    close(broadcast.sendSocketFd);
    close(broadcast.recvSocketFd);
}


void P2PNode::sendFile(int fileFD, int clientFD, unsigned long long offset, unsigned long long bytes) {

    char buff[4096 * 1024];
    unsigned long long bytesSent = 0;

    if (lseek(fileFD, offset, SEEK_SET) < 0) {
        throw std::runtime_error("lseek failed\n");
    }

    while (bytesSent < bytes) {
        ssize_t bytesToRead = std::min(bytes - bytesSent, (unsigned long long) 4096 * 1024);
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
        catch (...) {
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
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        throw std::runtime_error("connection with the server failed...\n");
    }

    // std::cout << sizeof(request);
    if (write(sockfd, &request, sizeof(request)) < sizeof(request)) {
        throw std::runtime_error("write failed\n");
    }

    // std::cout << request.fileName << "\n";
    int fd = handler.createAndOpenFileInWorkspace(request.fileName);

    if (lseek(fd, request.offset, SEEK_SET) < request.offset) {
        throw std::runtime_error("lseek failed\n");
    }
    // 4 MB
    char buff[4096 * 1024];
    unsigned long long bytesReceived = 0;
    while (bytesReceived < request.bytes) {
        ssize_t bytesToRead = std::min(request.bytes - bytesReceived, (unsigned long long) 4096 * 1024);
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

ActionResult P2PNode::performDownloadingFileFragment(fileRequest request, std::string ip_addr){
    try{
        requestAndDownloadFileFragment(request, ip_addr);
    }
    catch(std::runtime_error &e){
        return ACTION_FAILURE;
    }
    return ACTION_SUCCESS;
}

void P2PNode::prepareForReceivingBroadcast() {
    int recvFd;
    broadcast.recvAddress.sin_family = AF_INET;
    broadcast.recvAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    broadcast.recvAddress.sin_port = htons(broadcast.UDP_BROADCAST_PORT);

    if ((recvFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        throw std::runtime_error("Nie udalo sie otworzyc gniazda. Numer bledu: " + std::to_string(errno));
    }

    int sockoptVar = 1;
    if (setsockopt(recvFd, SOL_SOCKET, SO_BROADCAST, (void *) &sockoptVar, sizeof(sockoptVar)) < 0) {
        close(recvFd);
        throw std::runtime_error("Nie udalo sie setsockopt" + std::to_string(errno));
    }

    if (bind(recvFd, (struct sockaddr *) &broadcast.recvAddress, sizeof(broadcast.recvAddress)) < 0) {
        throw std::runtime_error("Nie udalo bind w odbieraniu. Errno: " + std::to_string(errno));
    }

    this->broadcast.recvSocketFd = recvFd;
}

void P2PNode::prepareForSendingBroadcast() {
    int sendFd;

    broadcast.sendAddress.sin_family = AF_INET;
    broadcast.sendAddress.sin_addr.s_addr = inet_addr(broadcast.UDP_BROADCAST_IP);
    broadcast.sendAddress.sin_port = htons(broadcast.UDP_BROADCAST_PORT);


    if ((sendFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw std::runtime_error("Nie udalo sie otworzyc gniazda. Numer bledu: " + std::to_string(errno));
    }

//    if (bind(sendFd, (struct sockaddr *) &broadcast.sendAddress, sizeof(broadcast.sendAddress)) < 0) {
//        throw std::runtime_error("Nie udalo bind w przygotowanie na wysylanie. Errno: " + std::to_string(errno));
//    }

    int sockoptVar = 1;
    if (setsockopt(sendFd, SOL_SOCKET, SO_BROADCAST, (void *) &sockoptVar, sizeof(sockoptVar)) < 0) {
        close(sendFd);
        throw std::runtime_error("Nie udalo sie setsockopt przy wysylaniu" + std::to_string(errno));
    }

    this->broadcast.sendSocketFd = sendFd;
}

ActionResult P2PNode::sendRevokeCommunicate(File file) {
    Communicate communicate(FileBroadcastStruct(file.getName(), file.getOwner(), file.getSize()),
                            handler.getUserName());

    if (send(broadcast.sendSocketFd, (void *) &communicate, (int) sizeof(communicate), 0) < (int) sizeof(communicate)) {
        return ACTION_FAILURE;
    }
    return ACTION_SUCCESS;
}


ActionResult P2PNode::startBroadcastingFiles() {
    broadcast.sendThread = std::thread([this]() {
        while (true) {
            // Sprawdzenie warunku czy wychodzimy z petli
            broadcast.exitMutex.lock();
            if (broadcast.exit) {
                broadcast.exitMutex.unlock();
                break;
            }
            broadcast.exitMutex.unlock();

            auto communicates = localFiles.getBroadcastCommunicates(handler.getUserName());

            for (auto communicate: communicates) {
                size_t size = sizeof(communicate);
                logging::TRACE("Wysyłam komunikat: " + communicate.toString());

                if (sendto(broadcast.sendSocketFd, (void *) &communicate, size, 0,
                           (struct sockaddr *) &broadcast.sendAddress, sizeof(broadcast.sendAddress), 0.0) < size) {
                    logging::ERROR("Nie udalo sie sendto w broadcascie.");
                }
            }
            std::this_thread::sleep_for(broadcast.interval);
        }
    });

    // wątek 'łapany' w destruktorze
    return ACTION_SUCCESS;
}

ActionResult P2PNode::startReceivingBroadcastingFiles() {
    broadcast.recvThread = std::thread([this]() {
        Communicate communicate;
        while (true) {
            // Sprawdzenie warunku czy wychodzimy z petli
            broadcast.exitMutex.lock();
            if (broadcast.exit) {
                broadcast.exitMutex.unlock();
                break;
            }
            broadcast.exitMutex.unlock();

            // trzeba pobrać adres nadawacy i zapisać go gdzies (?)
            socklen_t addrlen = sizeof(broadcast.recvAddress);
            if (recvfrom(broadcast.recvSocketFd, (void *) &communicate, sizeof(communicate), 0, (struct sockaddr *) &broadcast.recvAddress, &addrlen) < 0) {
                logging::ERROR("Blad recive w otrzmywaniu komunikatow\n");
                continue;
            }
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(broadcast.recvAddress.sin_addr.s_addr), str, INET_ADDRSTRLEN);
            if(!isRemoteIp(std::string(str))) {
                continue;
            }

            logging::TRACE("Odebrałem komunikat: " + communicate.toString() + "Wysylajacy: " + std::string(str));

            P2PRecordPossessor filesPossessor(communicate.userName,std::string(str));

            if (communicate.type == UDP_BROADCAST) {
                for (int i = 0; i < communicate.filesCount; ++i) {
                    File broadcastFile(communicate.files[i]);
                    globalFiles.add(filesPossessor, broadcastFile);
                }
            } else if(communicate.type == UDP_REVOKE){
//                //revoke communicate type
//                removeFile(communicate.files[0].name);
//                globalFiles.revoke(File(communicate.files[0]));
//                localFiles.removeFile(File(communicate.files[0]));
//                updateLocalFiles();
            }
        }
    });

    // wątek 'łapany' w destruktorze
    return ACTION_SUCCESS;
}

bool P2PNode::isRemoteIp(const std::string& testedIp) {

    if(testedIp == "0.0.0.0"){
        return false;
    }

    struct ifaddrs *addrs;
    getifaddrs(&addrs);

    struct ifaddrs *toRemove = addrs;

    while (addrs)
    {
        if (addrs->ifa_addr && addrs->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)addrs->ifa_addr;
            if(inet_ntoa(pAddr->sin_addr) == testedIp) {
                freeifaddrs(toRemove);
                return false;
            }

        }

        addrs = addrs->ifa_next;
    }
    freeifaddrs(toRemove);
    return true;
}

ActionResult P2PNode::downloadFile(const std::string &fileName) {

    File file;
    try {
        file = globalFiles.getFileByName(fileName);
    }
    catch (std::runtime_error& error) {
            return ACTION_FAILURE;
    }

    size_t fileSize = file.getSize();
    auto possessors = globalFiles.getFilePossessors(fileName);

    std::string log = "Żądano pliku " + fileName + " Posiadaja go";
    for (auto &p: possessors) {
        log += " " + p.getName() + ":" + "(" + p.getIp() + ")";
    }
    logging::TRACE(log);

    if (!possessors.empty()){
        //pobranie ilosci bajtow do pobrania od kazdego wysylajacego
        size_t toDownloadFromEach = fileSize/possessors.size();
        size_t rest = fileSize%possessors.size();
        fileRequest request {};
        strcpy(request.fileName, fileName.c_str());
        request.bytes = toDownloadFromEach;
        //rezultaty wykonan poszczegolnych pobieran
        std::vector <std::future<ActionResult >> results;

        for (auto i = 0; i < possessors.size(); i++){
            request.offset = i * toDownloadFromEach;
            //ktos musi doslac fragment, ktory pozostanie z dzielenia.
            request.bytes += (i == possessors.size() -1 ) ? rest : 0;
            log = "Zaczynam pobieranie fragmentu od " + std::to_string(request.offset) + " do " + std::to_string(request.offset + request.bytes) + " ";
            log += "od " + possessors[i].getName();
            logging::TRACE(log);
            std::future<ActionResult> t = std::async(&P2PNode::performDownloadingFileFragment, this, request, possessors[i].getIp());
            results.push_back(std::move(t));
        }
        //sprawdzamy czy wszystkie pobierania wykonaly sie poprawnie
        for (int i = 0; i < results.size(); i++) {

            ActionResult result = results[i].get();
            //jezeli nie, to czekamy ? sekund na kolejny broadcast i ponownie sprawdzamy kto ma dany plik
            while (result == ACTION_FAILURE) {
                sleep(5);
                possessors = globalFiles.getFilePossessors(fileName);
                //jezeli okaze sie ze nikt nie ma pliku to zwracamy failure.
                if (possessors.empty()){
                    return ACTION_FAILURE;
                }
                //iterujemy po posiadaczach pliku i jezeli uda nam sie pobrac od kogos to breakujemy petle.
                for (const auto& possessor : possessors){

                    request.offset = i * toDownloadFromEach;
                    request.bytes = toDownloadFromEach + ((i == results.size() - 1) ? rest : 0);
                    std::future<ActionResult> t = std::async(&P2PNode::performDownloadingFileFragment, this, request, possessor.getIp());
                    result = t.get();
                    if (result == ACTION_SUCCESS) {
                        break;
                    }
                }
            }
        }
    }
    else {
        //nikt nie ma wskazanego pliku
        return ACTION_FAILURE;
    }

    // nie jest potrzebna tutaj synchronizacja,
    // poniewaz ten sam watek dodaje i usuwa pliki
    AddFileResult ret = localFiles.addFile(file);
    if (ret != ADD_SUCCESS) {
        logging::ERROR("p2pnode: nie udalo sie dodac pliku do systemu lokalnego");
        return ACTION_FAILURE;
    }
    if (handler.addToConfig(file.getName(), file.getOwner(), file.getSize()) != FILE_SUCCESS) {
        logging::ERROR("p2pnode: pobrano plik ale nie dodano do pliku konfiguracyjnego");
        return ACTION_FAILURE;
    }
    return ACTION_SUCCESS;
}