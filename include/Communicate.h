//
// Created by dawid on 1/9/20.
//

#ifndef TINY_COMMUNICATE_H
#define TINY_COMMUNICATE_H


#include <string>
#include "P2PNode.h"

struct Communicate {
    UDPCommunicateType type;
    char userName[64];
    std::vector<FileBroadcastStruct> files;
    File revokedFile;

    explicit Communicate(std::string userName_, UDPCommunicateType type_ = UDP_BROADCAST);
    Communicate(UDPCommunicateType type_ = UDP_BROADCAST);
    explicit Communicate(File revokedFile_);
};


#endif //TINY_COMMUNICATE_H
