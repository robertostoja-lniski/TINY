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

#include "P2PNode.h"

/// @namespace operacje na plikach.
/// Potrzeba, żeby sprawdzic poprawnosc sciezek do pliku
namespace filesys = boost::filesystem;

/// @enum Rezultat operacji na plikach
enum FileOperationResult {
    FILE_OPERATION_NOT_HANDLED = -1,
    FILE_SUCCESS = 0,
    FILE_DOES_NOT_EXIST = 1,
    FILE_NOT_REGULAR_FILE = 2,
    FILE_WORKSPACE_NOT_EXIST = 3,
    FILE_HOMEPATH_BROKEN = 4,
    FILE_LINK_ERROR = 5,
    FILE_CANNOT_REMOVE_FROM_SYSTEM = 6,
    FILE_CANNOT_REMOVE_FROM_WORKSPACE = 7,
    FILE_CANNOT_ADD_TO_SYSTEM = 8,
    FILE_SHOW_NAME_ERROR = 9,
    FILE_CANNOT_OPEN = 10,
    FILE_CANNOT_READ = 11,
    FILE_CANNOT_WRITE = 12,
    FILE_CANNOT_REMOVE_FROM_CONFIG = 13,
    FILE_CANNOT_UPDATE_CONFIG = 14,
};

/// Rezultat operacji na katalogach
enum DirOperationResult {
    DIR_SUCCESS = 0,
    DIR_PREFIX_TO_NOWHERE = 1,
    DIR_PREFIX_TO_NOT_DIRECTORY = 2,
    DIR_CANNOT_CREATE = 3,
    DIR_CANNOT_FIND_WORKSPACE_USER = 4,
};

/// @enum Operacja konfiguracyjna
enum ConfigOperation{
    CONFIG_ADD = 0,
    CONFIG_REMOVE = 1,
};

/// @enum Rezultat operacji pobrania użytkownika
enum GetUser {
    GET_USER_SUCCESS = 0,
    GET_USER_FAIL = 1,
};

/**
 * @class
 * Zajmuje się operacjami na lokalnym systemie plików należących do systemu P2P.
 */
class LocalSystemHandler{

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

    /// Węzeł lokalny
    P2PNode p2PNode;

    /// Ustawia katalog roboczy użytkownika jako katalog główny systemu. (jeśli nie istnieje to tworzy go)
    DirOperationResult setDefaultWorkspace();

    /// Przywraca stan na podstawie pliku konfiguracyjnego
    FileOperationResult restorePreviousState();

    /// Nadpisuje konfigurację po wprowadzeniu zmian
    FileOperationResult updateConfig(const std::string&, ConfigOperation);

    /// Pobiera plik, jesli jest w workspace
    FileOperationResult upload(std::string);

public:
    /// Konstruktor
    LocalSystemHandler(P2PNode&);
    // tylko do oczytu
    // wyswietla zasoby lokalne
    FileOperationResult showLocalFiles();

    static GetUser getUserName(std::string&);

    /**
     * Wyświetla zasoby globalne
     * @param printOwners czy wyświetlić właścicieli
     * //TODO
     */
    FileOperationResult showGlobalFiles(bool);

    /**
     * Pobiera plik z globalnych zasobów ( jesli mozliwe to rownolegle z kilku zrodel )
     * @param fileName nazwa pliku do pobrania
     * //TODO
     */
    FileOperationResult download(std::string);

    /// Dodaje do katalogu roboczego, a następnie uploaduje plik do lokalnego setu
    FileOperationResult put(std::string);

    /// Usuwa plik z systemu, ale nie z workspace
    FileOperationResult removeFileFromSystem(std::string);

    /// Usuwa plik z workspace, jesli taki istnieje
    FileOperationResult removeFile(std::string);

};
#endif //TINY_LOCALSYSTEMHANDLER_H
