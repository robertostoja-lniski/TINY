#include "LocalSystemHandler.h"
#include "UserRequestHandler.h"
#include "Logger.h"

int main() {

    logging::configure({ {"type", "file"}, {"file_name", "log.txt"}, {"reopen_interval", "1"} });
    logging::TRACE("Uruchomienie programu.");
    try {

        LocalSystemHandler systemHandler;
        P2PNode node(1234, systemHandler);
        UserRequestHandler requestHandler(node);
        node.startHandlingDownloadRequests();


//        fileRequest request = {};
//        request.offset = 0,
//        request.bytes = 10,
//        strcpy(request.fileName, "file.txt");
////        std::cout << request.fileName << "\n";
//        node.requestAndDownloadFileFragment(request, "192.168.43.242");

        requestHandler.waitForRequest();
    }
        // kazdy blad ktory jest krytyczny i uniemozliwia wykonanie programu bedzie tutaj zlapany
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}