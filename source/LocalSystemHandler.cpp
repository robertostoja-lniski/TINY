#include "LocalSystemHandler.h"

LocalSystemHandler::LocalSystemHandler(P2PNode &node) : p2PNode(node){
    setDefaultWorkspace();
    restorePreviousState();
}
/*
 * API wzialem z
 * https://thispointer.com/c-check-if-given-path-is-a-file-or-directory-using-boost-c17-filesystem-library/
 */
FileOperationResult LocalSystemHandler::put(std::string filepath) {

    // sciezki powinne rozrozniac uzytkownikow
    // inne nie powinny byc akceptowane
    if(filepath[0] == '~') {

        std::string user;
        if(getUserName(user) != GET_USER_SUCCESS){
            return FILE_HOMEPATH_BROKEN;
        }
        filepath = "/home/" + user + filepath.substr(1, filepath.size());
    }

    if(!filesys::exists(filepath)) {
        std::cout << "Nieistniejaca sciezka\n";
        return FILE_DOES_NOT_EXIST;
    }

    if(!filesys::is_regular_file(filepath)) {
        std::cout << "To nie plik\n";
        return FILE_NOT_REGULAR_FILE;
    }

    // w przeciwnym wypadku utworz link z folderu roboczego
    // ale najpierw sprawdz czy ten folder istnieje

    if(!filesys::exists(workspaceUpperDirPath)) {
        std::cout << "Folder roboczy nie istnieje\n";
        return FILE_WORKSPACE_NOT_EXIST;
    }

    // znajdz nazwe pliku do utworzenia sciezki
    // na poczatku filename i filepath to to samo
    // filename zostanie przyciety
    std::string filename = filepath;
    const size_t cut_index = filepath.find_last_of("\\/");
    if(std::string::npos != cut_index) {
        filename = filename.erase(0, cut_index + 1);
    }
    
    if(link(filepath.c_str(), (workspaceUpperDirPath + workspaceDirName + filename).c_str()) != 0) {
        std::cout << "Link sie nie powiodl, plik o takiej nazwie juz istnieje\n";
        return FILE_LINK_ERROR;
    }

    // dodaj do systemu
    FileOperationResult ret = upload(filename);
    if(ret != FILE_SUCCESS) {
        return ret;
    }

    return FILE_SUCCESS;
}

FileOperationResult LocalSystemHandler::showLocalFiles() {

    if(p2PNode.showLocalFiles() != ACTION_SUCCESS) {
        return FILE_SHOW_NAME_ERROR;
    }
}

FileOperationResult LocalSystemHandler::removeFile(std::string fileToRemove) {

    FileOperationResult ret = removeFileFromSystem(fileToRemove);
    if( ret != FILE_SUCCESS){
        std::cout << "Usuwanie przerwane.\n";
        return ret;
    }

    std::string fullPathToFile = workspaceUpperDirPath + workspaceDirName + fileToRemove;
    // w przeciwnym wypadku usuwa plik z folderu

    if(p2PNode.revoke(fileToRemove) != ACTION_SUCCESS) {
        std::cout << "Uniewaznienie nie powiodlo sie\n";
        return FILE_CANNOT_REMOVE_FROM_SYSTEM;
    }

    if(remove(fullPathToFile.c_str())){
        std::cout << "Usuniecie nie powiodlo sie\n";
        // jesli nie mozna usunac z workspace'a, przywroc do systemu
        p2PNode.uploadFile(fileToRemove);
        return FILE_CANNOT_REMOVE_FROM_WORKSPACE;
    }

    // usuwa z pliku konfiguracyjnego
    std::cout << "Dowiazanie do pliku zostalo usuniete \n";
    return FILE_SUCCESS;
}

FileOperationResult LocalSystemHandler::removeFileFromSystem(std::string fileToRevoke) {

    std::string fullPathToFile = workspaceUpperDirPath + workspaceDirName + fileToRevoke;
    if(!filesys::exists(fullPathToFile)) {
        std::cout << "Nieistniejaca sciezka\n";
        return FILE_DOES_NOT_EXIST;
    }

    if(!filesys::is_regular_file(fullPathToFile)) {
        std::cout << "To nie plik\n";
        return FILE_NOT_REGULAR_FILE;
    }

    if(p2PNode.revoke(fileToRevoke) != ACTION_SUCCESS){
        return FILE_CANNOT_REMOVE_FROM_SYSTEM;
    }

    if( updateConfig(fileToRevoke, CONFIG_REMOVE) != FILE_SUCCESS ){
        std::cout << "Plik nie mogl zostac usuniety z pliku konfiguracyjengo\n";
        std::cout << "Usuwanie nie powiodlo sie\n";

        p2PNode.uploadFile(fileToRevoke);
        return FILE_CANNOT_REMOVE_FROM_CONFIG;
    }

    std::cout << "Plik " << fileToRevoke << " zostal usuniety z systemu\n";
    return FILE_SUCCESS;
}

FileOperationResult LocalSystemHandler::upload(std::string fileToAdd) {

    // na wszelki wypadek sprawdz, czy plik dalej istnieje
    std::string fullPathToFile = workspaceUpperDirPath + fileToAdd;
    if(!filesys::exists(fullPathToFile)) {
        std::cout << "Nieistniejaca sciezka\n";
        return FILE_DOES_NOT_EXIST;
    }

    if(!filesys::is_regular_file(fullPathToFile)) {
        std::cout << "To nie plik\n";
        return FILE_NOT_REGULAR_FILE;
    }

    ActionResult ret = p2PNode.uploadFile(fileToAdd);

    if(ret == ACTION_NO_EFFECT) {
        std::cout << "Plik " << fileToAdd << " juz istnial w systemie\n";
        return FILE_SUCCESS;

    } else if(ret != ACTION_SUCCESS){
        return FILE_CANNOT_ADD_TO_SYSTEM;
    }

    // dodaje informacje do pliku konfiguracyjnego
    // o tym, ze plik zostanie udostepniony

    updateConfig(fileToAdd, CONFIG_ADD);
    std::cout << "Plik " << fileToAdd << " zostal dodany do systemu\n";
    return FILE_SUCCESS;
}

FileOperationResult LocalSystemHandler::download(std::string) {
    return FILE_OPERATION_NOT_HANDLED;
}

FileOperationResult LocalSystemHandler::showGlobalFiles(bool printOwner) {
    return FILE_OPERATION_NOT_HANDLED;
}

DirOperationResult LocalSystemHandler::setDefaultWorkspace() {

    std::string user;
    if(getUserName(user) != GET_USER_SUCCESS) {
        return DIR_CANNOT_FIND_WORKSPACE_USER;
    }

    std::string defaultWorkspacePath = "/home/" + user + "/";

    // if workspace exists it is treated as success
    if(filesys::exists(defaultWorkspacePath)) {
        workspaceUpperDirPath = defaultWorkspacePath;
        return DIR_SUCCESS;
    }

    if(mkdir(defaultWorkspacePath.c_str(), S_IRWXU) != 0 ){
        std::cout << "Nie mozna stowrzyc katalogu\n";
        return DIR_CANNOT_CREATE;
    }

    std::cout << "Utworzono folder roboczy: " << defaultWorkspacePath << "\n";
    workspaceUpperDirPath = defaultWorkspacePath;

    return DIR_CANNOT_CREATE;
}

FileOperationResult LocalSystemHandler::restorePreviousState() {

    std::string fullConfigFilePath = workspaceUpperDirPath + workspaceDirName + configFileName;
    if(!filesys::exists(fullConfigFilePath)) {
        // tworzy plik konfiguracyjny
        std::ofstream output(fullConfigFilePath);
        return FILE_SUCCESS;
    }

    if(!filesys::is_regular_file(fullConfigFilePath)) {
        std::cout << "Plik konfiguracyjny nie jest regularnym plikiem\n";
        return FILE_NOT_REGULAR_FILE;
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

    while(std::getline(infile, configRecord)) {

        std::istringstream iss(configRecord);
        if(!(iss >> fileName)) {
            return FILE_CANNOT_READ;
        }

        p2PNode.uploadFile(fileName);
    }

    configFile.close();

}

FileOperationResult LocalSystemHandler::updateConfig(const std::string& name, ConfigOperation action) {

    std::string fullConfigFilePath = workspaceUpperDirPath + workspaceDirName + configFileName;

    if(!filesys::exists(fullConfigFilePath)) {
        // jesli nie ma to stworz zamiast zglaszac blad.
        std::ofstream output(fullConfigFilePath);
    }

    if(!filesys::is_regular_file(fullConfigFilePath)) {
        std::cout << "Blad pliku konfiguracyjnego - zmiany nie zostana zapisane\n";
        return FILE_NOT_REGULAR_FILE;
    }

    // dopisz plik do pliku konfiguracyjnego
    std::ofstream configFile;

    // dodaje nowy rekord
    if(action == CONFIG_ADD) {
        configFile.open(fullConfigFilePath, std::ios::in | std::ios::app);
        configFile << name << "\n";

    } else if(action == CONFIG_REMOVE){

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
        std::string fullTmpConfigFilePath = workspaceUpperDirPath + workspaceDirName + "/.tmp" + configFileName;
        configFile.open(fullTmpConfigFilePath, std::ios::out);

        while(std::getline(infile, configRecord)) {

            std::istringstream iss(configRecord);
            if(!(iss >> fileName)) {
                return FILE_CANNOT_READ;
            }

            if(fileName != name) {
                configFile << fileName << "\n";
            }
        }

        infile.close();
        configFile.close();

        // zamienia pliki
        if(remove(fullConfigFilePath.c_str())) {
            return FILE_CANNOT_UPDATE_CONFIG;
        }

        if(rename(fullTmpConfigFilePath.c_str(), fullConfigFilePath.c_str())) {
            // jak starczy czasu to dodam obsluge tej sytuacji
            std::cout << "W wyniku bledu, stracono plik konfiguracyjny\n";
            return FILE_CANNOT_UPDATE_CONFIG;
        }
    }
    return FILE_SUCCESS;
}

GetUser LocalSystemHandler::getUserName(std::string &user) {

    size_t maxUserNameLen = 32;
    char linuxName[maxUserNameLen];

    if(getlogin_r(linuxName, maxUserNameLen)){
        std::cout << "Nie udalo sie ustawic domyslnego folderu roboczego\n";
        return GET_USER_FAIL;
    }

    std::string tmp(linuxName);
    user = tmp;
    return GET_USER_SUCCESS;
}


