//
// Created by dawid on 1/9/20.
//

#ifndef TINY_COMMUNICATE_H
#define TINY_COMMUNICATE_H


#include <string>
#include <vector>
#include <cstring>
#include "FileBroadcastStruct.h"


/// @enum Typ komunikatu UDP. Pierwszy bajt komunikatu.
enum UDPCommunicateType {
    UDP_BROADCAST = 0,
    UDP_REVOKE = 1,
};

struct Communicate {
    const size_t filesCount;
    UDPCommunicateType type;
    char userName[64];
    FileBroadcastStruct revokedFile;
    static const int MAX_FILES_IN_COM = 64;
    FileBroadcastStruct files[MAX_FILES_IN_COM];


    explicit Communicate(std::string userName_, UDPCommunicateType type_ = UDP_BROADCAST);
    Communicate(UDPCommunicateType type_);
    Communicate(int filesCount_ = MAX_FILES_IN_COM);
    Communicate(FileBroadcastStruct revokedFile_, std::string userName);
};


#endif //TINY_COMMUNICATE_H
