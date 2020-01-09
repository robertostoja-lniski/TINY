//
// Created by robert on 09.01.2020.
//

#ifndef TINY_BROADCASTSTRUCT_H
#define TINY_BROADCASTSTRUCT_H

#include <cstdlib>

#define B_FILENAME_LEN 64
#define B_OWNER_NAME_LEN 64
#define BROADCAST_PERIOD 10

struct BroadcastStruct{

    char name[B_FILENAME_LEN];
    char owner[B_OWNER_NAME_LEN];
    size_t size;
};
#endif //TINY_BROADCASTSTRUCT_H
