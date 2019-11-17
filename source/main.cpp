#include <iostream>
#include "../include/FileHandler.h"
#include "../include/P2PNode.h"

#define EXAMPLE_UNIQUE_NODE_NAME "robert"
#define EXAMPLE_WORKSPACE_PATH "/home/robert"
#define EXAMPLE_FILE_TO_SYSTEM_PATH "/home/robert/file.txt"
int main() {

    //singleton niezaimplementowany
    P2PNode p2PNode(EXAMPLE_UNIQUE_NODE_NAME);
    FileHandler fileHandler;

    if(fileHandler.setWorkspacePath(EXAMPLE_WORKSPACE_PATH) != WORKSPACE_SUCCESS) {
        return 1;
    }

    if(fileHandler.addFileToSystem(EXAMPLE_FILE_TO_SYSTEM_PATH) != FILE_SUCCESS){
        return 1;
    }

    return 0;
}