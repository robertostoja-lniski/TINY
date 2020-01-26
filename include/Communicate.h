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

struct Communicate {
    size_t filesCount;
    char userName[MAX_USERNAME_LEN];
    FileBroadcastStruct files[MAX_FILES_IN_COM];

    explicit Communicate(size_t filesCount_, std::string userName_);
    Communicate();


    std::string toString();
    void clearMemory();
};


#endif //TINY_COMMUNICATE_H
