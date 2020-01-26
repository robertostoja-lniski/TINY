#include <Exit.h>
#include "UserRequestHandler.h"

void UserRequestHandler::processRequest(const std::string &request) {

    ActionResult ret = ACTION_NO_EFFECT;
    if (request == "ls") {
        node.showLocalFiles();
        node.showGlobalFiles();
        ret = ACTION_SUCCESS;
    } else if (request == "help") {
        printHelp();
        return;
    } else if(request == "exit"){
        std::cout << "Exit program" << std::endl;
        throw Exit();
    }

    if (ret != ACTION_SUCCESS) {
        std::cout << "Polecenie " << request << " nie zostalo wykonane\n";
        return;
    }
}

void UserRequestHandler::processRequest(const std::string &requestPrefix, const std::string &requestSufix) {

    ActionResult ret = ACTION_NO_EFFECT;

    if (requestPrefix == "get") {
        ret = node.downloadFile(requestSufix);
        if(ret == ACTION_FILE_REVOKED) {
            std::cout << "Nie mozna pobrac uniewaznionego pliku\n";
            return;
        }
    } else if (requestPrefix == "put") {
        ret = node.uploadFile(requestSufix);

    } else if (requestPrefix == "rm") {
        ret = node.removeFile(requestSufix);
    } else if (requestPrefix == "revoke") {
        ret = node.revokeFile(requestSufix);
    }

    if (ret != ACTION_SUCCESS) {
        std::cout << "Nie udalo sie wykonac polecenia " << requestPrefix << "\n";
        return;
    }

    return;
}

void UserRequestHandler::printHelp() {

    std::cout << "Lista komend:\n";
    std::cout << "ls                                            \twyswietlanie plikow w systemie\n";
    std::cout << "get [nazwa_pliku]                             \tpobranie pilku\n";
    std::cout << "put [sciezka pliku]                           \tdodanie pliku do systemu\n";
    std::cout << "rm [nazwa_pliku]                              \tusuwa plik z systemu\n";
    std::cout << "revoke [nazwa_pliku]                          \tuniewaznia plik\n";
    std::cout << "exit                                          \twyjscie z programu\n";

}

void UserRequestHandler::waitForRequest() {

    printHelp();
    printPrompt();
    // dzieki std::ws nie mamy bialych znakow na poczatku wiersza
    std::string requestPrefix;

    while (std::cin>>requestPrefix) {

        RequestType requestType = SINGLE_REQUEST;
        std::string requestSufix;
        if(std::find(singleCommands.begin(), singleCommands.end(), requestPrefix) != singleCommands.end()){
            processRequest(requestPrefix);
        } else if(std::find(doubleCommandsPrefix.begin(), doubleCommandsPrefix.end(), requestPrefix) != doubleCommandsPrefix.end()){
            std::cin>>requestSufix;
            processRequest(requestPrefix, requestSufix);
        } else {
            printUnknownCommandMsg();
            printPrompt();
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::cin.sync();
        printPrompt();
    }
}

void UserRequestHandler::printUnknownCommandMsg() {
    std::cout << "Nieznana komenda - wybierz jedna z ponizszych\n";
}

UserRequestHandler::UserRequestHandler(P2PNode &node) : node(node) {

}

void UserRequestHandler::printPrompt() {
    std::cout << "TIN> ";
}
