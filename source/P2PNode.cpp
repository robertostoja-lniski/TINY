#include <iostream>
#include <vector>
#include "P2PNode.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <utility>


P2PNode::P2PNode(const std::string& name) : name(name) {
    // Nazwa nie może być dłuższa niż 255 bajtów,
    // ponieważ jednostka plikowa wynosi 258 bajtów (nazwa(128)+\t+owner(128)+\n),
    // więc jednostka header też powinna wynosić max 258 bajtów (typ(1)+nazwa(255)+ilość plików(2))
    if(name.length() > 255){
        this->name = name.substr(0, 255);
    }

    startBroadcastingFiles();
    startReceivingBroadcastingFiles();
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

    if(localFiles.removeFile(tmp) != SUCCESS){
        return ACTION_FAILURE;
    }

    globalFiles.revoke(tmp);
    globalFiles.addToFilesRevokedByMe(tmp);

    return sendRevokeCommunicate(std::move(tmp));
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

    broadcast.sendThread = std::thread ([this](){
        int failuresCount = 0;
        std::future wantsToExit = broadcast.exit.get_future();
        while(wantsToExit.wait_for(broadcast.interval) != std::future_status::ready){

            auto communicates = localFiles.getBroadcastCommunicates();
            for(auto c: communicates) {
                char filesCount[2];
                std::memcpy(filesCount, &(c.first), 2);
                std::string str = (char)UDP_BROADCAST + name + '\n' + filesCount + c.second;
                while(send(broadcast.socketFd, str.c_str(), (int)str.length(), 0) < (int)str.length()) {
                    if (++failuresCount > 10) {
                        while (wantsToExit.wait_for(broadcast.interval) != std::future_status::ready &&
                               prepareForBroadcast(true) != ACTION_SUCCESS) {
                            std::this_thread::sleep_for(std::chrono::seconds(broadcast.restartConnectionInterval));
                        }
                        failuresCount = 0;
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

    // Wyślij wiadomość, że już chcę kończyć do broadcasta
    broadcast.exit.set_value(true);

    //Poczekaj aż broadczast się skończy
    broadcast.sendThread.join();
    broadcast.recvThread.join();

    // Zamknij gniazdo broadcastowe
    close(broadcast.socketFd);
}

ActionResult P2PNode::prepareForBroadcast(bool restart) {
    std::unique_lock<std::mutex> lk(broadcast.preparationMutex);

    if(restart){
        close(broadcast.socketFd);
        broadcast.socketFd = -1;
    }

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

ActionResult P2PNode::sendRevokeCommunicate(const File file) {
    // Przygotuj się na broadcast
    ActionResult actionResult;
    if((actionResult = prepareForBroadcast()) != ACTION_SUCCESS){
        // jeśli niepowodzenie, zwróć rezultat
        return actionResult;
    }

    std::string communicate = (char)UDP_REVOKE + file.getOwner() + '\n' + file.getName() + '\n';

    if(send(broadcast.socketFd, communicate.c_str(), (int)communicate.length(), 0) < (int)communicate.length()) {
        return ACTION_FAILURE;
    }
    return ACTION_SUCCESS;
}

ActionResult P2PNode::startReceivingBroadcastingFiles() {
    // Przygotuj się na broadcast
    ActionResult actionResult;
    if((actionResult = prepareForBroadcast()) != ACTION_SUCCESS){
        // jeśli niepowodzenie, zwróć rezultat
        return actionResult;
    }

    broadcast.sendThread = std::thread ([this](){
        std::future wantsToExit = broadcast.exit.get_future();
        char buf[16*1024];
        while(wantsToExit.wait_for(std::chrono::seconds(0)) != std::future_status::ready){
            if(recv(broadcast.socketFd, buf, sizeof(buf), 0) < 0){
                continue;
            }
            std::string str (buf);
            std::stringstream ss(str);
            char communicateType;
            ss >> communicateType;
            if(communicateType == UDP_BROADCAST){
                std::string sender, fileName, fileOwner;
                short number;
                ss >> sender >> number;
                for(int i = 0; i < number; ++i){
                    ss >> fileName >> fileOwner;
                    File tmp(std::move(fileName), std::move(fileOwner));
                    if(globalFiles.add( std::move(sender), std::move(tmp) ) == ADD_GLOBAL_REVOKED){
                        sendRevokeCommunicate(std::move(tmp));
                    }
                }
            }
            else{
                //revoke
                std::string name, owner;
                ss >> name >> owner;
                File tmp(name, owner);
                globalFiles.revoke(std::move(tmp));
                localFiles.removeFile(std::move(tmp));
                updateLocalFiles();
            }
        }
    });

    // wątek 'łapany' w destruktorze
    return ACTION_SUCCESS;
}

P2PNode& P2PNode::getInstance() {
    singletonMutex.lock();
    // stwórz jeśli nie istnieje
    if(node == nullptr){
        node = new P2PNode();
        setName();
    }
    singletonMutex.unlock();

    // zwróć instancję węzła
    return *node;
}

void P2PNode::setName() {
    // pozyskaj nazwę użytkownika z systemu UNIX
}

