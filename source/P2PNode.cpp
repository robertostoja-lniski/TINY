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

P2PNode::P2PNode(int tcpPort) : tcpPort(tcpPort) {
    char linuxName[MAX_USERNAME_LEN];

    if (getlogin_r(linuxName, MAX_USERNAME_LEN)) {
        throw std::runtime_error("nie mozna pobrac nazwy uzytkownika unix");
    }
    userName = std::string(linuxName);

    startBroadcastingFiles();
    startReceivingBroadcastingFiles();
}

ActionResult P2PNode::uploadFile(std::string uploadFileName) {

    File file(std::move(uploadFileName), userName);
    // nie jest potrzebna tutaj synchronizacja,
    // poniewaz ten sam watek dodaje i usuwa pliki
    AddFileResult ret = localFiles.addFile(file);
    if (ret == ADD_ALREADY_EXISTS) {
        return ACTION_NO_EFFECT;
    }

    if (ret == ADD_SUCCESS) {
        return ACTION_SUCCESS;
    }
}

ActionResult P2PNode::showLocalFiles() {
    localFiles.print();
}

ActionResult P2PNode::revoke(std::string revokeFileName) {
    File tmp(std::move(revokeFileName), userName);

    if (localFiles.removeFile(tmp) != SUCCESS) {
        return ACTION_FAILURE;
    }

    globalFiles.revoke(tmp);
    globalFiles.addToFilesRevokedByMe(tmp);

    return sendRevokeCommunicate(std::move(tmp));
}

ActionResult P2PNode::downloadFile(const std::string &) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::updateLocalFiles(void) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::showGlobalFiles(void) {
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

            auto communicates = localFiles.getBroadcastCommunicates();
            for (auto c: communicates) {

                std::string str = (char) UDP_BROADCAST + userName + '\n' + std::to_string(c.first) + c.second;
                // jeżeli jest niezerowa liczba plików, to wysyłaj
                if (c.first > 0) {
                    while (send(broadcast.socketFd, str.c_str(), (int) str.length(), 0) < (int) str.length()) {
                        if (++failuresCount > 10) {
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
                }
            }
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
    close(broadcast.socketFd);
}


void P2PNode::sendFile(int fileFD, int clientFD, unsigned long long offset, unsigned long long bytesCount) {
    std::cout << "send file, jeszcze nie zaimplementowane\n";
}

void P2PNode::handleDownloadRequests() {
// socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("tcp socket creation failed...\n");
        exit(0);
    }
    struct sockaddr_in servaddr{};

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(this->tcpPort);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, MAX_TCP_CONNECTIONS)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {

        struct sockaddr_in cli{};
        socklen_t len = sizeof(cli);

        int clientFD = accept(sockfd, (sockaddr *) &cli, &len);
        if (clientFD < 0) {
            printf("server acccept failed...\n");
            exit(0);
        }

        char buff[sizeof(fileRequest)];

        if (recv(clientFD, buff, sizeof(buff), 0) <= 0) {
            perror("recv() ERROR");
            exit(5);
        }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
        auto request = (fileRequest *) buff;
#pragma clang diagnostic pop
        std::cout << " " << request->fileName << " " << request->bytesCount << " " << request->offset << std::endl;

        char path[200];
        strcpy(path, "/home/");
        strcat(path, userName.c_str());
        strcat(path, "/.P2Pworkspace/");
        strcat(path, request->fileName);


        int fileFD = open(path, 0);
        if (fileFD < 0) {
            printf("Zadany plik nie istnieje.\n");
            close(clientFD);
            continue;
        }

        std::thread t1(&P2PNode::sendFile, this, fileFD, clientFD, request->offset, request->bytesCount);
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

ActionResult P2PNode::requestFile(const std::string) {


    fileRequest request = {

    };
    return ACTION_NOT_HANDLED;
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

    std::string communicate = (char) UDP_REVOKE + file.getOwner() + '\n' + file.getName() + '\n';

    if (send(broadcast.socketFd, communicate.c_str(), (int) communicate.length(), 0) < (int) communicate.length()) {
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
        char buf[16 * 1024];
        while (true) {
            // Sprawdzenie warunku czy wychodzimy z petli
            broadcast.exitMutex.lock();
            if (broadcast.exit) {
                broadcast.exitMutex.unlock();
                break;
            }
            broadcast.exitMutex.unlock();

            if (recv(broadcast.socketFd, buf, sizeof(buf), 0) < 0) {
                continue;
            }
            std::string str(buf);
            std::stringstream ss(str);
            char communicateType;
            ss >> communicateType;
            if (communicateType == UDP_BROADCAST) {
                std::string sender, fileName, fileOwner;
                short number;
                ss >> sender >> number;
                for (int i = 0; i < number; ++i) {
                    ss >> fileName >> fileOwner;
                    File tmp(std::move(fileName), std::move(fileOwner));
                    if (globalFiles.add(std::move(sender), std::move(tmp)) == ADD_GLOBAL_REVOKED) {
                        sendRevokeCommunicate(
                                std::move(tmp)); // dlaczego tutaj uzywamy zmiennej ktora zostala przeniesiona?
                    }
                }
            } else {
                //revoke
                std::string name, owner;
                ss >> name >> owner;
                File tmp(name, owner);
                globalFiles.revoke(std::move(tmp));
                localFiles.removeFile(std::move(tmp)); // dlaczego tutaj uzywamy zmiennej ktora zostala przeniesiona?
                updateLocalFiles();
            }
        }
    });

    // wątek 'łapany' w destruktorze
    return ACTION_SUCCESS;
}

std::string P2PNode::getUserName() {
    return userName;
}



