#include <iostream>
#include "LocalSystemHandler.h"
#include "P2PNode.h"
#include "UserRequestHandler.h"

#define EXAMPLE_UNIQUE_NODE_NAME "robert"
#define EXAMPLE_WORKSPACE_PATH "/home/robert"
#define EXAMPLE_FILE_TO_SYSTEM_PATH "/home/robert/file.txt"
#define EXAMPLE_FILENAME_IN_WORKSPACE "file.txt"
P2PNode* P2PNode::node = 0;
std::mutex P2PNode::singletonMutex;

int main() {

    auto node = P2PNode::getInstance();
    LocalSystemHandler systemHandler(node);
    UserRequestHandler requestHandler(systemHandler);
    requestHandler.waitForRequest();

    delete &node;
    return 0;
}