#include "LocalSystemHandler.h"
#include "UserRequestHandler.h"


int main() {
    try {

        P2PNode node(1234);
        LocalSystemHandler systemHandler(node);
        UserRequestHandler requestHandler(systemHandler);
        node.startHandlingDownloadRequests();
        requestHandler.waitForRequest();
    }
        // kazdy blad ktory jest krytyczny i uniemozliwia wykonanie programu bedzie tutaj zlapany
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}