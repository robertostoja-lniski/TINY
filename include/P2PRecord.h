//
// Created by robert on 15.11.2019.
//

#ifndef TINY_P2PRECORD_H
#define TINY_P2PRECORD_H


#include <string>
#include <set>
#include <iostream>
#include <stdio.h>
#include "File.h"

// do przechodzenia po plikach powinien sluzyc iterator, (lub petla for each)

enum RecordOperationResult{

    SUCCESS = 0,
    FILE_NOT_FOUND = 1,

};
class P2PRecord {

private:
    std::set< File > fileSet;

public:
    void addFile(File);
    RecordOperationResult removeFile(File);
    // potem dodam przeciazony operator
    void print();
};


#endif //TINY_P2PRECORD_H
