

#include "../include/LocalSystemHandler.h"

LocalSystemHandler::LocalSystemHandler(P2PNode p2PNode) : p2PNode(p2PNode) {}
/*
 * API wzialem z
 * https://thispointer.com/c-check-if-given-path-is-a-file-or-directory-using-boost-c17-filesystem-library/
 */
FileOperationResult LocalSystemHandler::addFileToWorkspace(std::string filepath) {

    // sciezki powinne rozrozniac uzytkownikow
    // inne nie powinny byc akceptowane
    if(filepath[0] == '~') {
        std::cout << "Replace ~ with full name in path\n";
        return FILE_HOMEPATH_BROKEN;
    }

    if(!filesys::exists(filepath)) {
        std::cout << "Not a valid path\n";
        return FILE_DOES_NOT_EXIST;
    }

    if(!filesys::is_regular_file(filepath)) {
        std::cout << "Not a regular file\n";
        return FILE_NOT_REGULAR_FILE;
    }

    // w przeciwnym wypadku utworz link z folderu roboczego
    // ale najpierw sprawdz czy ten folder istnieje

    if(!filesys::exists(workspacePath)) {
        std::cout << "Workspace folder does not exists\n";
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
    
    if(link(filepath.c_str(), (workspacePath + "/" + filename).c_str()) != 0) {
        std::cout << "Cannot link file - it already exists\n";
        return FILE_LINK_ERROR;
    }

    std::cout << filename << " is added to your workspace\n";
    return FILE_SUCCESS;
}

DirOperationResult LocalSystemHandler::setWorkspacePath(std::string workspacePrefix) {

    if(!filesys::exists(workspacePrefix)) {
        std::cout << "Not a valid path\n";
        return WORKSPACE_PREFIX_TO_NOWHERE;
    }

    if(!filesys::is_directory(workspacePrefix)) {
        std::cout << "It is not a path to directory\n";
        return WORKSPACE_PREFIX_TO_NOT_DIRECTORY;
    }

    std::string fullWorkspacePath = workspacePrefix + workspaceSuffix;

    // if workspace exists it is treated as success
    if(filesys::exists(fullWorkspacePath)) {
        std::cout << "Workspace " << fullWorkspacePath << " already exists\n";
        workspacePath = fullWorkspacePath;
        return WORKSPACE_SUCCESS;
    }

    if(mkdir(fullWorkspacePath.c_str(), S_IRWXU) != 0 ){
        std::cout << "Cannot create workspace\n";
        return WORKSPACE_CANNOT_CREATE;
    }

    std::cout << "Created workspace: " << fullWorkspacePath << "\n";
    workspacePath = fullWorkspacePath;

    return  WORKSPACE_SUCCESS;
}

FileOperationResult LocalSystemHandler::removeFileFromWorkspace(std::string fileToRemove) {

    std::string fullPathToFile = workspacePath + "/" + fileToRemove;
    if(!filesys::exists(fullPathToFile)) {
        std::cout << "Not a valid path\n";
        return FILE_DOES_NOT_EXIST;
    }

    if(!filesys::is_regular_file(fullPathToFile)) {
        std::cout << "Not a regular file\n";
        return FILE_NOT_REGULAR_FILE;
    }

    // w przeciwnym wypadku usuwa plik z folderu

    if(p2PNode.revoke(fileToRemove) != ACTION_SUCCESS) {
        std::cout << "Cannot revoke file\n";
        return FILE_CANNOT_REMOVE_FROM_SYSTEM;
    }

    if(remove(fullPathToFile.c_str())){
        std::cout << "Cannot remove file\n";
        // jesli nie mozna usunac z workspace'a, przywroc do systemu
        p2PNode.uploadFile(fileToRemove);
        return FILE_CANNOT_REMOVE_FROM_WORKSPACE;
    }

    return FILE_SUCCESS;
}

void LocalSystemHandler::showLocalFiles() {
    p2PNode.showLocalFiles();
}

FileOperationResult LocalSystemHandler::removeFileFromSystem(std::string fileToRevoke) {

    std::string fullPathToFile = workspacePath + "/" + fileToRevoke;
    if(!filesys::exists(fullPathToFile)) {
        std::cout << "Not a valid path\n";
        return FILE_DOES_NOT_EXIST;
    }

    if(!filesys::is_regular_file(fullPathToFile)) {
        std::cout << "Not a regular file\n";
        return FILE_NOT_REGULAR_FILE;
    }

    if(p2PNode.revoke(fileToRevoke) != ACTION_SUCCESS){
        return FILE_CANNOT_REMOVE_FROM_SYSTEM;
    }
    return FILE_SUCCESS;
}

FileOperationResult LocalSystemHandler::addFileToSystem(std::string fileToAdd) {

    std::string fullPathToFile = workspacePath + "/" + fileToAdd;
    if(!filesys::exists(fullPathToFile)) {
        std::cout << "Not a valid path\n";
        return FILE_DOES_NOT_EXIST;
    }

    if(!filesys::is_regular_file(fullPathToFile)) {
        std::cout << "Not a regular file\n";
        return FILE_NOT_REGULAR_FILE;
    }

    if(p2PNode.uploadFile(fileToAdd) != ACTION_SUCCESS) {
        return FILE_CANNOT_ADD_TO_SYSTEM;
    }
    return FILE_SUCCESS;
}
