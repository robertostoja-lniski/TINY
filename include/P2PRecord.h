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

/// @enum Rezultat operacji na rekordzie
enum RecordOperationResult{
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
    std::set< File > fileSet;

public:

    /// Dodaje plik do setu plików
    /// @if plik już dodany @then nie dodaje i zwraca ADD_ALREADY_EXISTS
    /// @return @enum AddFileResult
    AddFileResult addFile(File);

    /// Usuwa plik z setu plików
    RecordOperationResult removeFile(File);

    // TODO przeciazony operator
    /// Wypisuje pliki
    void print();
};


#endif //TINY_P2PRECORD_H
