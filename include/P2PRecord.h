//
// Created by robert on 15.11.2019.
//

#ifndef TINY_P2PRECORD_H
#define TINY_P2PRECORD_H


#include <string>
#include <set>
#include <iostream>
#include <cstdio>
#include <shared_mutex>
#include <vector>
#include "File.h"
#include "FileBroadcastStruct.h"
#include "Communicate.h"
#include "Defines.h"

/// @enum Rezultat operacji na rekordzie
enum RecordOperationResult {
    SUCCESS = 0,
    FILE_NOT_FOUND = 1,
};

/// @enum Rezultat dodawania pliku
enum AddFileResult {
    ADD_SUCCESS = 0,
    ADD_ALREADY_EXISTS = 1,
};

/// @class
/// Przechowuje set plików.
/// Umożliwia wypisywanie plików znajdujących się w secie.
/// Status: finished
/// @author Robert
class P2PRecord {

private:
    /// Set plików
    std::set<File> fileSet;

    /// Mutex używany do synchronizacji zapis-odczyt setu plików
    std::shared_mutex mutex;

public:

    /// Dodaje plik do setu plików
    /// @if plik już dodany @then nie dodaje i zwraca ADD_ALREADY_EXISTS
    /// @return @enum AddFileResult
    AddFileResult addFile(const File&);

    /// Usuwa plik z setu plików
    RecordOperationResult removeFile(const File&);

    void print();

    /**
     * @author Wojciech
     * Zwraca wektor komunikatów broadcastowych.
     * Są one później po kolei wysyłane
     * @return wektor par {ilość plików, treść (pliki)}
     * @synchronized
     */
    std::vector<Communicate> getBroadcastCommunicates(const std::string&);

};


#endif //TINY_P2PRECORD_H
