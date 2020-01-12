//
// Created by dawid on 1/9/20.
//

#include <Communicate.h>

Communicate::Communicate(std::string userName_, UDPCommunicateType type_)
        : filesCount(MAX_FILES_IN_COM) {
    strncpy(userName, userName_.c_str(), 63);
    type = type_;
}

Communicate::Communicate(UDPCommunicateType type_)
    : filesCount(MAX_FILES_IN_COM){
    type = type_;

}

Communicate::Communicate(FileBroadcastStruct revokedFile_, std::string userName_)
        : filesCount(MAX_FILES_IN_COM) {
    revokedFile = revokedFile_;
    type = UDP_REVOKE;
    strncpy(userName, userName_.c_str(), 63);

}

Communicate::Communicate(int filesCount_)
        : filesCount(filesCount_), type(UDP_BROADCAST){
}

