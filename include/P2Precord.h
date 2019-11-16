//
// Created by robert on 15.11.2019.
//

#ifndef TINY_P2PRECORD_H
#define TINY_P2PRECORD_H


#include <string>
#include <set>
#include "File.h"

// do przechodzenia po plikach powinien sluzyc iterator, (lub petla for each)
class P2Precord {

private:
    std::set< File > fileSet;

public:
    void addFile(File);
    void removeFile(File);
};


#endif //TINY_P2PRECORD_H
