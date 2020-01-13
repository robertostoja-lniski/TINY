//
// Created by dawid on 1/9/20.
//

#ifndef TINY_COMMUNICATE_H
#define TINY_COMMUNICATE_H


#include <string>
#include <vector>
#include <cstring>
#include "FileBroadcastStruct.h"
#include "Defines.h"


/// @enum Typ komunikatu UDP. Pierwszy bajt komunikatu.
enum UDPCommunicateType {
    UDP_BROADCAST = 0,
    UDP_REVOKE = 1,
};

struct Communicate {
    char s[8] = "AAAAAAA";
    UDPCommunicateType type;
    char s2[8] = "AAAAAAA";
    size_t filesCount;
    char s3[8] = "AAAAAAA";

    char userName[MAX_USERNAME_LEN];
    char s4[8] = "AAAAAAA";
    FileBroadcastStruct files[MAX_FILES_IN_COM];
    char s5[8] = "AAAAAAA";

    explicit Communicate(size_t filesCount_, std::string userName_);
    Communicate(FileBroadcastStruct revokedFile, const std::string& userName);

    Communicate();

    std::string toString();

    void clearMemory();
};


#endif //TINY_COMMUNICATE_H
