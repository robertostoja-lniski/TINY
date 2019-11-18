

#include "../include/LocalSystemHandler.h"

LocalSystemHandler::LocalSystemHandler(P2PNode p2PNode) : p2PNode(p2PNode) {
    setDefaultWorkspace();
    restorePreviousState();
}
/*
 * API wzialem z
 * https://thispointer.com/c-check-if-given-path-is-a-file-or-directory-using-boost-c17-filesystem-library/
 */
FileOperationResult LocalSystemHandler::addFileToWorkspace(std::string filepath) {

    // sciezki powinne rozrozniac uzytkownikow
    // inne nie powinny byc akceptowane
    if(filepath[0] == '~') {
        std::cout << "Zamien ~ na pelna sciezke\n";
        return FILE_HOMEPATH_BROKEN;
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
    
    if(link(filepath.c_str(), (workspaceUpperDirPath + filename).c_str()) != 0) {
        std::cout << "Link sie nie powiodl, plik o takiej nazwie juz istnieje\n";
        return FILE_LINK_ERROR;
    }

    std::cout << filename << " jest dodany do folderu roboczego\n";
    return FILE_SUCCESS;
}

DirOperationResult LocalSystemHandler::setWorkspacePath(std::string workspacePrefix) {

    if(!filesys::exists(workspacePrefix)) {
        std::cout << "Nieistniejaca sciezka\n";
        return DIR_PREFIX_TO_NOWHERE;
    }

    if(!filesys::is_directory(workspacePrefix)) {
        std::cout << "To nie sciezka do folderu\n";
        return DIR_PREFIX_TO_NOT_DIRECTORY;
    }

    std::string fullWorkspacePath = workspacePrefix + workspaceDirName;

    // if workspace exists it is treated as success
    if(filesys::exists(fullWorkspacePath)) {
        std::cout << "Folder roboczy " << fullWorkspacePath << " juz istnieje\n";
        workspaceUpperDirPath = fullWorkspacePath + "/";
        return DIR_SUCCESS;
    }

    if(mkdir(fullWorkspacePath.c_str(), S_IRWXU) != 0 ){
        std::cout << "Nie mozna stowrzyc katalogu\n";
        return DIR_CANNOT_CREATE;
    }

    std::cout << "Utworzono folder roboczy: " << fullWorkspacePath << "\n";
    workspaceUpperDirPath = fullWorkspacePath;

    return  DIR_SUCCESS;
}

FileOperationResult LocalSystemHandler::showLocalFiles() {

    if(p2PNode.showLocalFiles() != ACTION_SUCCESS) {
        return FILE_SHOW_NAME_ERROR;
    }
}

FileOperationResult LocalSystemHandler::removeFileFromWorkspace(std::string fileToRemove) {

    FileOperationResult ret = removeFileFromSystem(fileToRemove);
    if( ret != FILE_SUCCESS){
        std::cout << "Usuwanie przerwane.\n";
        return ret;
    }

    std::string fullPathToFile = workspaceUpperDirPath + fileToRemove;
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
    std::cout << "Plik " << fileToRemove << " zostal usuniety z folderu roboczego ( i z systemu )\n";
    return FILE_SUCCESS;
}

FileOperationResult LocalSystemHandler::removeFileFromSystem(std::string fileToRevoke) {

    std::string fullPathToFile = workspaceUpperDirPath + fileToRevoke;
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
    p2PNode.broadcastFiles();
    return FILE_SUCCESS;
}

FileOperationResult LocalSystemHandler::download(std::string) {
    return FILE_OPERATION_NOT_HANDLED;
}

FileOperationResult LocalSystemHandler::showGlobalFiles(bool printOwner) {
    return FILE_OPERATION_NOT_HANDLED;
}

DirOperationResult LocalSystemHandler::showWorkspaceFiles() {

    std::string fullWorkspacePath = workspaceUpperDirPath + workspaceDirName;

    if(!filesys::exists(fullWorkspacePath)) {
        std::cout << "Folder roboczy nie istnieje!\n";
        return DIR_PREFIX_TO_NOWHERE;
    }

    if(!filesys::is_directory(fullWorkspacePath)) {
        std::cout << "Blad folderu roboczego\n";
        return DIR_PREFIX_TO_NOT_DIRECTORY;
    }

    for(const auto & file : filesys::directory_iterator(fullWorkspacePath)){
        // nie pokazuj plikow konfiguracyjnych
        if(file.path() != fullWorkspacePath + configFileName &&
            file.path() != fullWorkspacePath + ".tmp" + configFileName) {

            std::cout << file.path() << "\n";
        }
    }
    return DIR_SUCCESS;
}

DirOperationResult LocalSystemHandler::setDefaultWorkspace() {

    size_t maxUserNameLen = 32;
    char linuxName[maxUserNameLen];

    if(getlogin_r(linuxName, maxUserNameLen)){
        std::cout << "Nie udalo sie ustawic domyslnego folderu roboczego\n";
        return DIR_CANNOT_FIND_WORKSPACE_USER;
    }

    std::string workspacePathSufix(linuxName);
    std::string defaultWorkspacePath = "/home/" + workspacePathSufix + "/";

    if(!filesys::exists(defaultWorkspacePath)) {
        std::cout << "Nie udalo sie ustawic domyslnego folderu roboczego\n";
        return DIR_PREFIX_TO_NOWHERE;
    }

    if(!filesys::is_directory(defaultWorkspacePath)) {
        std::cout << "Nie udalo sie ustawic domyslnego folderu roboczego\n";
        return DIR_PREFIX_TO_NOT_DIRECTORY;
    }

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
    configFile.open(fullConfigFilePath, std::ios::out);

    while(std::getline(infile, configRecord)) {

        std::istringstream iss(configRecord);
        if(!(iss >> fileName)) {
            return FILE_CANNOT_READ;
        }

        p2PNode.uploadFile(fileName);
    }

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
                configFile << name << "\n";
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


