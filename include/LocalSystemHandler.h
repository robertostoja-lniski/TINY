#ifndef TINY_LOCALSYSTEMHANDLER_H
#define TINY_LOCALSYSTEMHANDLER_H

#include <string>
#include <iostream>
#include <boost/filesystem.hpp>

#if __APPLE__
#include <filesystem>
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <fstream>


/// @namespace operacje na plikach.
/// Potrzeba, żeby sprawdzic poprawnosc sciezek do pliku
namespace filesys = boost::filesystem;

/// @enum Rezultat operacji na plikach
enum FileOperationResult {
    FILE_OPERATION_NOT_HANDLED = -1,
    FILE_SUCCESS = 0,
    FILE_PATH_CORRUPTED = 1,
    FILE_CANNOT_ADD_TO_WORKSPACE = 2,
    FILE_CANNOT_REMOVE_FROM_WORKSPACE = 3,
    FILE_CANNOT_READ = 4,
    FILE_CANNOT_UPDATE_CONFIG = 5,
    FILE_CANNOT_REMOVE_FROM_CONFIG = 6,

};

/// Rezultat operacji na katalogach
enum DirOperationResult {
    DIR_SUCCESS = 0,
    DIR_CANNOT_CREATE = 1,
};

/// @enum Operacja konfiguracyjna
enum ConfigOperation {
    CONFIG_ADD = 0,
    CONFIG_REMOVE = 1,
};


/**
 * @class
 * Zajmuje się operacjami na lokalnym systemie plików należących do systemu P2P.
 */
class LocalSystemHandler {

private:
    /// Nazwa katalogu roboczego systemu
    const std::string workspaceDirName = ".P2Pworkspace/";

    /**
     * Nazwa pliku konfiguracyjnego
     * W pliku konfiguracyjnym jest spis plików, które zostały udostępnione.
     */
    const std::string configFileName = ".p2P.config";

    /// Nazwa katalogu nadrzędnego katalogu roboczego
    std::string workspaceUpperDirPath;

    //// sprawdza czy sciezka z systemu plikow jest poprawna
    bool isFsFilePathCorrect(std::string filepath);

public:
    /// Konstruktor
    LocalSystemHandler();

    //// sprawdza czy w systemie istenieje plik o danej systemowej nazwie
    bool isNetworkFilePathCorrect(std::string filepath);

    //// usuwa plik z folderu roboczego
    FileOperationResult removeFileFromLocalSystem(std::string);

    //// dodaje plik do folderu roboczego
    FileOperationResult addFileToLocalSystem(std::string);

    FileOperationResult updateConfig(const std::string&, ConfigOperation);

    DirOperationResult setDefaultWorkspace();

    //// zwraca wektor nazw plikow, ktorych bylismy wlascicielem
    //// przed zamknieciem programu
    std::vector<std::string> getPreviousState();

    //// zwraca ostatni czlon sciezki w systemie plikow
    std::string getLastTokenOf(std::string);

    //// zwraca nazwe uzytkownika w FS
    std::string getUserName();

    int createAndOpenFile(std::string name);
};

#endif //TINY_LOCALSYSTEMHANDLER_H
