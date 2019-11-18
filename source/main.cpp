#include <iostream>
#include "../include/LocalSystemHandler.h"
#include "../include/P2PNode.h"
#include "../include/UserRequestHandler.h"

#define EXAMPLE_UNIQUE_NODE_NAME "robert"
#define EXAMPLE_WORKSPACE_PATH "/home/robert"
#define EXAMPLE_FILE_TO_SYSTEM_PATH "/home/robert/file.txt"
#define EXAMPLE_FILENAME_IN_WORKSPACE "file.txt"
int main() {

    //singleton niezaimplementowany
    P2PNode p2PNode(EXAMPLE_UNIQUE_NODE_NAME);
    LocalSystemHandler systemHandler(p2PNode);
    UserRequestHandler requestHandler(systemHandler);
    requestHandler.waitForRequest();

    return 0;
}