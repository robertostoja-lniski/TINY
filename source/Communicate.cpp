//
// Created by dawid on 1/9/20.
//

#include <Communicate.h>

Communicate::Communicate(std::string userName_, UDPCommunicateType type_) {
    strncpy(userName, userName_.c_str(), 63);
    type = type_;
}

Communicate::Communicate(UDPCommunicateType type_) {
    type = type_;

}

Communicate::Communicate(FileBroadcastStruct revokedFile_, std::string userName_) {
    revokedFile = revokedFile_;
    type = UDP_REVOKE;
    strncpy(userName, userName_.c_str(), 63);

}

Communicate::Communicate() {
    type = UDP_BROADCAST;
    files.reserve(256);
}

