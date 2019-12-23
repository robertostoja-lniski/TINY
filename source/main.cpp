
#include "LocalSystemHandler.h"
#include "UserRequestHandler.h"

#define EXAMPLE_UNIQUE_NODE_NAME "robert"
#define EXAMPLE_WORKSPACE_PATH "/home/robert"
#define EXAMPLE_FILE_TO_SYSTEM_PATH "/home/robert/file.txt"
#define EXAMPLE_FILENAME_IN_WORKSPACE "file.txt"

int main() {

    auto node = new P2PNode();
    LocalSystemHandler systemHandler(*node);
    UserRequestHandler requestHandler(systemHandler);
    requestHandler.waitForRequest();

    delete node;
    return 0;
}