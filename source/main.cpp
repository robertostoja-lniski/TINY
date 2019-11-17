#include <iostream>
#include "../include/LocalSystemHandler.h"
#include "../include/P2PNode.h"

#define EXAMPLE_UNIQUE_NODE_NAME "robert"
#define EXAMPLE_WORKSPACE_PATH "/home/robert"
#define EXAMPLE_FILE_TO_SYSTEM_PATH "/home/robert/file.txt"
#define EXAMPLE_FILENAME_IN_WORKSPACE "file.txt"
int main() {

    //singleton niezaimplementowany
    P2PNode p2PNode(EXAMPLE_UNIQUE_NODE_NAME);
    LocalSystemHandler handler(p2PNode);


    if(handler.setWorkspacePath(EXAMPLE_WORKSPACE_PATH) != WORKSPACE_SUCCESS) {
        return 1;
    }

    if(handler.addFileToWorkspace(EXAMPLE_FILE_TO_SYSTEM_PATH) != FILE_SUCCESS){
        return 1;
    }

    if(handler.addFileToSystem(EXAMPLE_FILENAME_IN_WORKSPACE) != FILE_SUCCESS) {
        return 1;
    }

    handler.showLocalFiles();

    if(handler.removeFileFromWorkspace(EXAMPLE_FILENAME_IN_WORKSPACE) != FILE_SUCCESS) {
        handler.showLocalFiles();
        return 1;
    }

    handler.showLocalFiles();

    if(handler.addFileToWorkspace(EXAMPLE_FILE_TO_SYSTEM_PATH) != FILE_SUCCESS){
        return 1;
    }

    if(handler.addFileToSystem(EXAMPLE_FILENAME_IN_WORKSPACE) != FILE_SUCCESS) {
        return 1;
    }

    handler.showLocalFiles();

    if(handler.removeFileFromSystem(EXAMPLE_FILENAME_IN_WORKSPACE) != FILE_SUCCESS) {
        return 1;
    }

    handler.showLocalFiles();
    std::cout << "wpisz ls, plik powinien byc w folderze\n";
    return 0;
}