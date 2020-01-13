//
// Created by dawid on 1/9/20.
//

#include <Communicate.h>

void Communicate::clearMemory() {
    //wypelniamy wszystko zerami zeby nie wysylac smieci w komunikatach
    memset(userName, 0x00, MAX_USERNAME_LEN* sizeof(char));
    memset(files, 0x00, MAX_FILES_IN_COM* sizeof(FileBroadcastStruct));
}

Communicate::Communicate(FileBroadcastStruct revokedFile, const std::string& userName_)
        : filesCount(1) {
    clearMemory();
    files[0] = revokedFile;
    type = UDP_REVOKE;
    strncpy(userName, userName_.c_str(), MAX_USERNAME_LEN - 1);

}

Communicate::Communicate(size_t filesCount_, std::string userName_)
        : filesCount(filesCount_), type(UDP_BROADCAST){
    clearMemory();
    strncpy(userName, userName_.c_str(), 63);
}

std::string Communicate::toString() {
    std::string result;
    result += "Rozmiar komunikatu: ";
    result += std::to_string(sizeof(*this));
    result += " Pliki w komunikacie: ";

    for (int i = 0 ; i < filesCount; i++){
        result += std::string(files[i].name) + " ";
    }
    return result;
}

Communicate::Communicate() {
    clearMemory();
}

