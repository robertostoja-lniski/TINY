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
#include <LocalSystemHandler.h>


P2PNode::P2PNode(int tcpPort):tcpPort(tcpPort){}

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

ActionResult P2PNode::BroadcastFilesFrequently(double period) {
    return ACTION_NOT_HANDLED;
}

ActionResult P2PNode::broadcastFiles() {
    return ACTION_NOT_HANDLED;
}

void P2PNode::setName() {
    // pozyskaj nazwę użytkownika z systemu UNIX
}

void P2PNode::sendFile(int fileFD, int clientFD, unsigned long long offset, unsigned long long bytesCount){
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
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
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

        int clientFD = accept(sockfd, (sockaddr*)&cli, &len);
        if (clientFD < 0) {
            printf("server acccept failed...\n");
            exit(0);
        }

        char buff[sizeof(fileRequest)];

        if( recv(clientFD, buff, sizeof(buff), 0) <= 0 )
        {
            perror( "recv() ERROR" );
            exit( 5 );
        }

        #pragma clang diagnostic push
        #pragma ide diagnostic ignored "OCDFAInspection"
        auto request = (fileRequest*) buff;
        #pragma clang diagnostic pop
        std::cout<<" "<<request->fileName<<" "<<request->bytesCount<<" "<<request->offset<<std::endl;

        std::string user;
        LocalSystemHandler::getUserName(user);
        char path[200];
        strcpy(path, "/home/");
        strcat(path, user.c_str());
        strcat(path, "/.P2Pworkspace/");
        strcat(path, request->fileName);


        int fileFD = open(path, 0);
        if(fileFD<0) {
            printf("Zadany plik nie istnieje.\n");
            close(clientFD);
            continue;
        }

        std::thread t1(&P2PNode::sendFile, this,fileFD, clientFD, request->offset, request->bytesCount);
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
