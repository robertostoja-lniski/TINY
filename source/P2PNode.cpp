#include <iostream>
#include <utility>
#include <vector>
#include "../include/P2PNode.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <P2PNode.h>
#include <thread>
#include <cstring>


P2PNode::P2PNode(const std::string& name) : name(name) {
    // Nazwa nie może być dłuższa niż 255 bajtów,
    // ponieważ jednostka plikowa wynosi 258 bajtów (nazwa(128)+\t+owner(128)+\n),
    // więc jednostka header też powinna wynosić max 258 bajtów (typ(1)+nazwa(255)+ilość plików(2))
    if(name.length() > 255){
        this->name = name.substr(0, 255);
    }

    startBroadcastingFiles();
}

ActionResult P2PNode::uploadFile(std::string uploadFileName) {

    File file(std::move(uploadFileName), name);
    // nie jest potrzebna tutaj synchronizacja,
    // poniewaz ten sam watek dodaje i usuwa pliki
    AddFileResult ret = localFiles.addFile(file);
    if(ret == ADD_ALREADY_EXISTS){
        return ACTION_NO_EFFECT;
    }

    if(ret == ADD_SUCCESS) {
        return ACTION_SUCCESS;
    }
}

ActionResult P2PNode::showLocalFiles() {
    localFiles.print();
}

ActionResult P2PNode::revoke(std::string revokeFileName) {
    File tmp(std::move(revokeFileName), name);
    localFiles.removeFile(tmp);
//    BroadcastFiles();
    return ACTION_SUCCESS;
}

ActionResult P2PNode::downloadFile(const std::string&) {
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
    if((actionResult = prepareForBroadcast()) != ACTION_SUCCESS){
        // jeśli niepowodzenie, zwróć rezultat
        return actionResult;
    }

    // Utwórz wątek
    std::future wantsToExit = broadcast.exit.get_future();

    broadcast.thread = std::thread ([this](std::future<bool> wantsToExit){
        int failuresCount = 0;
        while(wantsToExit.wait_for(broadcast.interval) != std::future_status::ready){

            auto communicates = localFiles.getBroadcastCommunicates();
            for(auto c: communicates) {
                char filesCount[2];
                std::memcpy(filesCount, &(c.first), 2);
                std::string str = (char)UDP_BROADCAST + name + '\n' + filesCount + c.second;
                while(send(broadcast.socketFd, str.c_str(), (int)str.length(), 0) < (int)str.length()) {
                    if (++failuresCount > 10) {
                        // restart connection
                        close(broadcast.socketFd);
                        while (wantsToExit.wait_for(broadcast.interval) != std::future_status::ready &&
                               prepareForBroadcast() != ACTION_SUCCESS) {
                            std::this_thread::sleep_for(std::chrono::seconds(broadcast.restartConnectionInterval));
                        }
                        failuresCount = 0;
                    }
                }
            }
        }
    }, std::move(wantsToExit) );

    // wątek 'łapany' w destruktorze
    return ACTION_SUCCESS;
}

P2PNode::~P2PNode() {
    //OBSLUGA BROADCASTU

    // Wyślij wiadomość, że już chcę kończyć do broadcasta
    broadcast.exit.set_value(true);

    //Poczekaj aż broadczast się skończy
    broadcast.thread.join();

    // Zamknij gniazdo broadcastowe
    close(broadcast.socketFd);
}

ActionResult P2PNode::prepareForBroadcast() {
    int fd = broadcast.socketFd;
    if(fd >= 0){
        return ACTION_SUCCESS;
    }

    // Otwórz gniazdo
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        std::cout << "Nie udalo sie otworzyc gniazda. Numer bledu: " << errno << std::endl;
        return ACTION_FAILURE;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(broadcast.UDP_BROADCAST_PORT);

    // Bind adres do gniazda
    if(bind(fd, (struct sockaddr *) &address, sizeof(address)) < 0){
        close(fd);
        std::cout << "Nie udalo sie przypisac adresu do gniazda. Numer bledu: " << errno << std::endl;
        return ACTION_FAILURE;
    }

    this->broadcast.socketFd = fd;

    return ACTION_SUCCESS;
}

