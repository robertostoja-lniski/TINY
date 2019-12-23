#include "LocalSystemHandler.h"
#include "UserRequestHandler.h"

#define EXAMPLE_UNIQUE_NODE_NAME "robert"
#define EXAMPLE_WORKSPACE_PATH "/home/robert"
#define EXAMPLE_FILE_TO_SYSTEM_PATH "/home/robert/file.txt"
#define EXAMPLE_FILENAME_IN_WORKSPACE "file.txt"


int main() {
    try {

        P2PNode node(1234);
        LocalSystemHandler systemHandler(node);
        UserRequestHandler requestHandler(systemHandler);
        node.startHandlingDownloadRequests();
        requestHandler.waitForRequest();
    }
    // kazdy blad ktory jest krytyczny i uniemozliwia wykonanie programu bedzie tutaj zlapany
    catch (std::exception& e) {
        std::cout << e.what()<<std::endl;
    }
    return 0;
}