#include "LocalSystemHandler.h"
#include "UserRequestHandler.h"


int main() {
    try {

        LocalSystemHandler systemHandler;
        P2PNode node(1234, systemHandler);
        UserRequestHandler requestHandler(node);
        node.startHandlingDownloadRequests();
        requestHandler.waitForRequest();
    }
        // kazdy blad ktory jest krytyczny i uniemozliwia wykonanie programu bedzie tutaj zlapany
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}