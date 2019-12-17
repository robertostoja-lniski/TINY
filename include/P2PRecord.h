//
// Created by robert on 15.11.2019.
//

#ifndef TINY_P2PRECORD_H
#define TINY_P2PRECORD_H


#include <string>
#include <set>
#include <iostream>
#include <stdio.h>
#include <shared_mutex>
#include "File.h"

// do przechodzenia po plikach powinien sluzyc iterator, (lub petla for each)

enum RecordOperationResult{
    SUCCESS = 0,
    FILE_NOT_FOUND = 1,
};

enum AddFileResult {
    ADD_SUCCESS = 0,
    ADD_ALREADY_EXISTS = 1,
};
class P2PRecord {

private:
    std::set< File > fileSet;

    /// Mutex używany do synchronizacji zapis-odczyt setu plików
    std::shared_mutex mutex;

public:
    AddFileResult addFile(File);
    RecordOperationResult removeFile(File);
    // potem dodam przeciazony operator
    void print();

    /**
     * @author Wojciech
     * Zwraca wektor komunikatów broadcastowych.
     * Są one później po kolei wysyłane
     * @return wektor par {ilość plików, treść (pliki)}
     * @synchronized
     */
    std::vector<std::pair<u_short, std::string>> getBroadcastCommunicates();
};


#endif //TINY_P2PRECORD_H
