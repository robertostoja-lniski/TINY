

#include "../include/FileHandler.h"

/*
 * API wzialem z
 * https://thispointer.com/c-check-if-given-path-is-a-file-or-directory-using-boost-c17-filesystem-library/
 */
FileOperationResult FileHandler::addFileToSystem(std::string filepath) {

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
        std::cout << "Cannot link file\n";
        return FILE_LINK_ERROR;
    }

    std::cout << filename << " is added to your system\n";
    return FILE_SUCCESS;
}

DirOperationResult FileHandler::setWorkspacePath(std::string workspacePrefix) {

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
