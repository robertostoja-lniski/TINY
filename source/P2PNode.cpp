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

P2PNode::P2PNode(int tcpPort, LocalSystemHandler &handler) : tcpPort(tcpPort), handler(handler) {

    handler.setDefaultWorkspace();
    std::vector<std::string> filesNamesToReUpload = handler.getPreviousState();

    for (auto fileName : filesNamesToReUpload) {
        size_t fileSize = handler.getFileSize(fileName);
        File restoredFile(fileName, handler.getUserName(), fileSize);
        localFiles.addFile(restoredFile);
    }
    prepareForBroadcast();
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

    if (handler.updateConfig(systemFileName, CONFIG_ADD) != FILE_SUCCESS) {
        return ACTION_FAILURE;
    }

    std::cout << "Plik " << uploadFileName << " zostal dodany do systemu\n";
    return ACTION_SUCCESS;
}

ActionResult P2PNode::showLocalFiles() {
    localFiles.print();
    return ACTION_SUCCESS;
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

    globalFiles.revoke(tmp);
    globalFiles.addToFilesRevokedByMe(tmp);

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

ActionResult P2PNode::downloadFile(const std::string &) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::updateLocalFiles(void) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::showGlobalFiles(SHOW_GLOBAL_FILE_TYPE type) {
    globalFiles.showFiles();
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
                logging::TRACE(communicate.toString());

                if (sendto(broadcast.sendSocketFd, (void *) &communicate, size, 0,
                              (struct sockaddr *) &broadcast.sendAddress, sizeof(broadcast.sendAddress)) < size) {
                    logging::ERROR("Nie udalo sie sendto w broadcascie.");

                }
            }
            std::this_thread::sleep_for(broadcast.interval);
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

ActionResult P2PNode::prepareForBroadcast() {
    std::unique_lock<std::mutex> lk(broadcast.preparationMutex);

    int sendFd = broadcast.sendSocketFd;
    int recvFd = broadcast.recvSocketFd;
    if (sendFd >= 0 && recvFd >= 0) {
        return ACTION_SUCCESS;
    }

    auto socketHandler = [this](int &fd, struct sockaddr_in &address) {
        // Otwórz gniazdo
        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            std::cout << "Nie udalo sie otworzyc gniazda. Numer bledu: " << errno << std::endl;
            return ACTION_FAILURE;
        }

        int sockoptVar = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (void *) &sockoptVar, sizeof(sockoptVar)) < 0) {
            std::cout << "Broadcast not allowed";
            close(fd);
            return ACTION_FAILURE;
        }

//        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &sockoptVar, sizeof(sockoptVar)) < 0) {
//            std::cout << "Reuse not allowed";
//            close(fd);
//            return ACTION_FAILURE;
//        }

//        // Connect adres do gniazda
//        if (connect(fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
//            close(fd);
//            std::cout << "Nie udalo sie przypisac adresu do gniazda. Numer bledu: " << errno << std::endl;
//            return ACTION_FAILURE;
//        }
        return ACTION_SUCCESS;
    };

    broadcast.sendAddress.sin_family = AF_INET;
    broadcast.sendAddress.sin_port = htons(broadcast.UDP_BROADCAST_PORT);
    broadcast.sendAddress.sin_addr.s_addr = inet_addr(broadcast.UDP_BROADCAST_IP);

    broadcast.recvAddress.sin_family = AF_INET;
    broadcast.recvAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    broadcast.recvAddress.sin_port = htons(broadcast.UDP_BROADCAST_PORT);


    ActionResult actionResult = socketHandler(sendFd, broadcast.sendAddress);
    if (actionResult != ACTION_SUCCESS) {
        return actionResult;
    }

    actionResult = socketHandler(recvFd, broadcast.recvAddress);
    if (actionResult != ACTION_SUCCESS) {
        close(sendFd);
        return actionResult;
    }

    this->broadcast.sendSocketFd = sendFd;
    this->broadcast.recvSocketFd = recvFd;

    return ACTION_SUCCESS;
}

ActionResult P2PNode::sendRevokeCommunicate(File file) {
    Communicate communicate(FileBroadcastStruct(file.getName(), file.getOwner(), file.getSize()),
                            handler.getUserName());

    if (send(broadcast.sendSocketFd, (void *) &communicate, (int) sizeof(communicate), 0) < (int) sizeof(communicate)) {
        return ACTION_FAILURE;
    }
    return ACTION_SUCCESS;
}

ActionResult P2PNode::startReceivingBroadcastingFiles() {
    broadcast.recvThread = std::thread([this]() {
        auto communicate = std::make_unique<Communicate>();
        while (true) {
            // Sprawdzenie warunku czy wychodzimy z petli
            broadcast.exitMutex.lock();
            if (broadcast.exit) {
                broadcast.exitMutex.unlock();
                break;
            }
            broadcast.exitMutex.unlock();

            if (recv(broadcast.recvSocketFd, (void *) communicate.get(), 1024 * 64, 0) < 0) {
                continue;
            }
            if (communicate->type == UDP_BROADCAST) {
                FileBroadcastStruct *file = communicate->files;
                for (int i = 0; i < communicate->filesCount; ++i, ++file) {
                    if (globalFiles.add(communicate->userName, File(*file)) == ADD_GLOBAL_REVOKED) {
                        sendRevokeCommunicate(File(*file));
                    }
                }
            } else {
                //revoke communicate type
                removeFile(communicate->files[0].name);
                globalFiles.revoke(File(communicate->files[0]));
                localFiles.removeFile(File(communicate->files[0]));
                updateLocalFiles();
            }
        }
    });

    // wątek 'łapany' w destruktorze
    return ACTION_SUCCESS;
}




