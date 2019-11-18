#ifndef TINY_LOCALSYSTEMHANDLER_H
#define TINY_LOCALSYSTEMHANDLER_H

#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <experimental/filesystem>
#include "P2PNode.h"
#include <sys/stat.h>
#include <unistd.h>

// zeby sprawdzic poprawnosc sciezek do pliku
namespace filesys = std::experimental::filesystem;

enum FileOperationResult {
    FILE_SUCCESS = 0,
    FILE_DOES_NOT_EXIST = 1,
    FILE_NOT_REGULAR_FILE = 2,
    FILE_WORKSPACE_NOT_EXIST = 3,
    FILE_HOMEPATH_BROKEN = 4,
    FILE_LINK_ERROR = 5,
    FILE_CANNOT_REMOVE_FROM_SYSTEM = 6,
    FILE_CANNOT_REMOVE_FROM_WORKSPACE = 7,
    FILE_CANNOT_ADD_TO_SYSTEM = 8,
};

enum DirOperationResult {
    WORKSPACE_SUCCESS = 0,
    WORKSPACE_PREFIX_TO_NOWHERE = 1,
    WORKSPACE_PREFIX_TO_NOT_DIRECTORY = 2,
    WORKSPACE_CANNOT_CREATE = 3,
};

class LocalSystemHandler{

private:
    const std::string workspaceSuffix = "/.P2Pworkspace";
    std::string workspacePath;
    P2PNode p2PNode;
public:
    LocalSystemHandler(P2PNode);

    // dodaje do katalogu roboczego ale nie uploaduje
    FileOperationResult addFileToWorkspace(std::string);
    // usuwa plik z workspace, jesli taki istnieje
    FileOperationResult removeFileFromWorkspace(std::string);
    // uploaduje plik, jesli jest w workspace
    FileOperationResult addFileToSystem(std::string);
    // usuwa plik z systemu, ale nie z workspace
    FileOperationResult removeFileFromSystem(std::string);
    // ustawia workspace
    DirOperationResult setWorkspacePath(std::string);
    // pokazuje pliki lokalne
    void showLocalFiles();

};
#endif //TINY_LOCALSYSTEMHANDLER_H
