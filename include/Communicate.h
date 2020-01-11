//
// Created by dawid on 1/9/20.
//

#ifndef TINY_COMMUNICATE_H
#define TINY_COMMUNICATE_H


#include <string>
#include <vector>
#include "FileBroadcastStruct.h"


/// @enum Typ komunikatu UDP. Pierwszy bajt komunikatu.
enum UDPCommunicateType {
    UDP_BROADCAST = 0,
    UDP_REVOKE = 1,
};

struct Communicate {
    UDPCommunicateType type;
    char userName[64];
    std::vector<FileBroadcastStruct> files;
    FileBroadcastStruct revokedFile;

    explicit Communicate(std::string userName_, UDPCommunicateType type_ = UDP_BROADCAST);
    Communicate(UDPCommunicateType type_);
    Communicate();
    Communicate(FileBroadcastStruct revokedFile_, std::string userName);
};


#endif //TINY_COMMUNICATE_H
