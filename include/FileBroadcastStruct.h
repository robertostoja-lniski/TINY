#ifndef TINY_FILEBROADCASTSTRUCT_H
#define TINY_FILEBROADCASTSTRUCT_H

#include <cstdio>
#include <string>

struct FileBroadcastStruct {
    char name[64];
    char owner[64];
    size_t size;

    FileBroadcastStruct(std::string name_, std::string owner_, size_t size);
    FileBroadcastStruct();
    void setValues(std::string name_, std::string owner_, size_t size);
};


#endif //TINY_FILEBROADCASTSTRUCT_H
