#ifndef TINY_FILEHANDLER_H
#define TINY_FILEHANDLER_H

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
};

enum DirOperationResult {
    WORKSPACE_SUCCESS = 0,
    WORKSPACE_PREFIX_TO_NOWHERE = 1,
    WORKSPACE_PREFIX_TO_NOT_DIRECTORY = 2,
    WORKSPACE_CANNOT_CREATE = 3,
};

class FileHandler{

private:
    const std::string workspaceSuffix = "/.P2Pworkspace";
    std::string workspacePath;


public:
    FileHandler() = default;

    FileOperationResult addFileToSystem(std::string);
    DirOperationResult setWorkspacePath(std::string);

};
#endif //TINY_FILEHANDLER_H
