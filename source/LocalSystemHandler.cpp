#include <fcntl.h>
#include "LocalSystemHandler.h"
#include "P2PNode.h"
#include <unistd.h>


LocalSystemHandler::LocalSystemHandler() {}

std::string LocalSystemHandler::getUserName() {
    char linuxName[MAX_USERNAME_LEN];
    memset(linuxName, 0x00, MAX_USERNAME_LEN);

    if (getlogin_r(linuxName, MAX_USERNAME_LEN)) {
        throw std::runtime_error("nie mozna pobrac nazwy uzytkownika unix");
    }

    return {linuxName};
}
/*
 * API wzialem z
 * https://thispointer.com/c-check-if-given-path-is-a-file-or-directory-using-boost-c17-filesystem-library/
 */

std::string LocalSystemHandler::getLastTokenOf(const std::string filepath) {
    std::string filename = filepath;
    const size_t cut_index = filepath.find_last_of("\\/");
    if (std::string::npos != cut_index) {
        filename = filename.erase(0, cut_index + 1);
    }
    return filename;
}
bool LocalSystemHandler::isFsFilePathCorrect(std::string filepath) {

    // sciezki powinne rozrozniac uzytkownikow
    // inne nie powinny byc akceptowane
    if (filepath[0] == '~') {

        filepath = "/home/" + getUserName() + filepath.substr(1, filepath.size());
    }

    if (!filesys::exists(filepath)) {
        std::cout << "Nieistniejaca sciezka\n";
        return false;
    }

    if (!filesys::is_regular_file(filepath)) {
        std::cout << "To nie plik\n";
        return false;
    }

    // w przeciwnym wypadku utworz link z folderu roboczego
    // ale najpierw sprawdz czy ten folder istnieje

    if (!filesys::exists(workspaceAbsoluteDirPath)) {
        std::cout << "Folder roboczy nie istnieje\n";
        return false;
    }

    return true;
}

bool LocalSystemHandler::isNetworkFilePathCorrect(std::string filepath) {

    std::string fullPathToFile = workspaceAbsoluteDirPath + filepath;
    if (!filesys::exists(fullPathToFile)) {
        std::cout << "Nieistniejaca sciezka\n";
        return false;
    }

    if (!filesys::is_regular_file(fullPathToFile)) {
        std::cout << "To nie plik\n";
        return false;
    }
    return true;
}

FileOperationResult LocalSystemHandler::addFileToLocalSystem(std::string filepath) {

    if(!isFsFilePathCorrect(filepath)) {
        return FILE_PATH_CORRUPTED;
    }

    // znajdz nazwe pliku do utworzenia sciezki
    // na poczatku filename i filepath to to samo
    // filename zostanie przyciety
    std::string filename = getLastTokenOf(filepath);

    std::string linkPath = workspaceAbsoluteDirPath + filename;
    std::string test = linkPath + "a";
    if (link(filepath.c_str(), linkPath.c_str()) != 0) {
        std::cout << "Link sie nie powiodl, taki plik istnieje albo wystapil blad w sciezce\n";
        return FILE_CANNOT_ADD_TO_WORKSPACE;
    }

    return FILE_SUCCESS;
    // should be uploaded
}


FileOperationResult LocalSystemHandler::removeFileFromLocalSystem(std::string fileToRemove) {

    std::string fullPathToFile = workspaceAbsoluteDirPath + workspaceDirName + fileToRemove;

    if (updateConfig(fileToRemove, CONFIG_REMOVE) != FILE_SUCCESS) {
        std::cout << "Plik nie mogl zostac usuniety z pliku konfiguracyjengo\n";
        std::cout << "Usuwanie nie powiodlo sie\n";
        return FILE_CANNOT_REMOVE_FROM_CONFIG;
    }

    std::cout << "Plik " << fileToRemove << " zostal usuniety z systemu\n";
    // w przeciwnym wypadku usuwa plik z folderu

    if (remove(fullPathToFile.c_str())) {
        std::cout << "Usuniecie nie powiodlo sie\n";
        // jesli nie mozna usunac z workspace'a, przywroc do systemu
        return FILE_CANNOT_REMOVE_FROM_WORKSPACE;
    }

    // usuwa z pliku konfiguracyjnego
    std::cout << "Dowiazanie do pliku zostalo usuniete \n";
    return FILE_SUCCESS;
}

DirOperationResult LocalSystemHandler::setDefaultWorkspace() {
#if __APPLE__
    std::string defaultWorkspacePath = "/Users/";
#else
    std::string defaultWorkspacePath = "/home/";
#endif
    defaultWorkspacePath += getUserName() + "/" + workspaceDirName;
    std::cout << defaultWorkspacePath << std::endl;
    if (!filesys::exists(defaultWorkspacePath) && mkdir(defaultWorkspacePath.c_str(), S_IRWXU) != 0) {
        std::cout << "Nie mozna stowrzyc katalogu\n";
        return DIR_CANNOT_CREATE;
    }

    // std::cout << "Folder roboczy: " << defaultWorkspacePath << "\n";
    workspaceAbsoluteDirPath = defaultWorkspacePath;

    if(!filesys::exists(defaultWorkspacePath + configFileName)){
        std::ofstream conf(defaultWorkspacePath + configFileName);
        conf.close();
    }

    return DIR_SUCCESS;
}

std::vector<std::string> LocalSystemHandler::getPreviousState() {

    std::string fullConfigFilePath = workspaceAbsoluteDirPath + configFileName;
    if(!isFsFilePathCorrect(fullConfigFilePath)) {
        throw std::runtime_error("Cannot read config file");
    }

    std::ofstream configFile;

    // otwiera plik do odczytu
    std::ifstream infile(fullConfigFilePath);
    // wiersz z pliku
    std::string configRecord;
    // nazwa pliku zapisana w konfiguracji
    std::string fileName;

    // otwiera plik i iteruje po nim
    configFile.open(fullConfigFilePath, std::ios::in);
    std::vector<std::string> filesToUpload;

    while (std::getline(infile, configRecord)) {

        std::istringstream iss(configRecord);
        if (!(iss >> fileName)) {
            throw std::runtime_error("Wrong config file");
        }

        filesToUpload.push_back(fileName);
    }

    configFile.close();
    return filesToUpload;
}

FileOperationResult LocalSystemHandler::updateConfig(const std::string& name, ConfigOperation action) {

    std::string fullConfigFilePath = workspaceAbsoluteDirPath + configFileName;

    if(!isFsFilePathCorrect(fullConfigFilePath)){
        return FILE_PATH_CORRUPTED;
    }
    // dopisz plik do pliku konfiguracyjnego
    std::ofstream configFile;

    // dodaje nowy rekord
    if (action == CONFIG_ADD) {
        configFile.open(fullConfigFilePath, std::ios::in | std::ios::app);
        configFile << name << "\n";

    } else if (action == CONFIG_REMOVE) {

        /*
         * tworzy plik tymczasowy juz bez usunietego rekordu
         * nastepnie zastepuje nim konfiguracyjny
         */
        // otwiera plik do odczytu
        std::ifstream infile(fullConfigFilePath);
        // wiersz z pliku
        std::string configRecord;
        // nazwa pliku zapisana w konfiguracji
        std::string fileName;

        // tworzy i otwiera plik do zapisu
        std::string fullTmpConfigFilePath = workspaceAbsoluteDirPath + workspaceDirName + "/.tmp" + configFileName;
        configFile.open(fullTmpConfigFilePath, std::ios::out);

        while (std::getline(infile, configRecord)) {

            std::istringstream iss(configRecord);
            if (!(iss >> fileName)) {
                return FILE_CANNOT_READ;
            }

            if (fileName != name) {
                configFile << fileName << "\n";
            }
        }

        infile.close();
        configFile.close();

        // zamienia pliki
        if (remove(fullConfigFilePath.c_str())) {
            return FILE_CANNOT_UPDATE_CONFIG;
        }

        if (rename(fullTmpConfigFilePath.c_str(), fullConfigFilePath.c_str())) {
            // jak starczy czasu to dodam obsluge tej sytuacji
            std::cout << "W wyniku bledu, stracono plik konfiguracyjny\n";
            return FILE_CANNOT_UPDATE_CONFIG;
        }
    }
    return FILE_SUCCESS;
}

int LocalSystemHandler::createAndOpenFileInWorkspace(std::string name) {
    std::string path = workspaceAbsoluteDirPath + name;
    std::cout << path << "\n";
    int fd = open(path.c_str(), O_CREAT | O_WRONLY, S_IRWXU);
    if (fd < 0) {
        throw std::runtime_error("file create failed\n");
    }
    return fd;
}

int LocalSystemHandler::openFileFromWorkSpace(std::string name) {
    std::string path = workspaceAbsoluteDirPath + name;
    int fileFD = open(path.c_str(), 0);
    if (fileFD < 0) {
        throw std::runtime_error("file open failed\n");
    }
    return fileFD;
}

