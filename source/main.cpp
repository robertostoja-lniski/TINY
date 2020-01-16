#include <Exit.h>
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
        requestHandler.waitForRequest();
    }
    catch(Exit &e){
        return 0;
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}