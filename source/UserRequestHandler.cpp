#include <Exit.h>
#include "UserRequestHandler.h"

RequestResult UserRequestHandler::processRequest(const std::string &request) {

    ActionResult ret = ACTION_NO_EFFECT;

    if (request == "ls-my") {
        ret = node.showLocalFiles();

    } else if (request == "ls") {
        ret = node.showLocalFiles();
        if(ret == ACTION_SUCCESS) {
            ret = node.showGlobalFiles(DO_NOT_PRINT_OWNERS);
        }
    } else if (request == "ls-owners") {
        ret = node.showGlobalFiles(PRINT_OWNERS);
    } else if (request == "help") {
        printHelp();
        return REQUEST_SUCCESS;
    } else if(request == "exit"){
        std::cout << "Exit program" << std::endl;
        throw Exit();
    }

    if (ret != ACTION_SUCCESS) {
        std::cout << "Polecenie " << request << " nie zostalo wykonane\n";
        return REQUEST_FAILURE;
    }

    return REQUEST_SUCCESS;
}

RequestResult UserRequestHandler::processRequest(const std::string &requestPrefix, const std::string &requestSufix) {

    ActionResult ret = ACTION_NO_EFFECT;

    if (requestPrefix == "get") {
        ret = node.downloadFile(requestSufix);

    } else if (requestPrefix == "put") {
        ret = node.uploadFile(requestSufix);

    } else if (requestPrefix == "rm") {
        ret = node.removeFile(requestSufix);
    }

    if (ret != ACTION_SUCCESS) {
        std::cout << "Nie udalo sie wykonac polecenia " << requestPrefix << "\n";
        return REQUEST_FAILURE;
    }

    return REQUEST_SUCCESS;
}

void UserRequestHandler::printHelp() {

    std::cout << "Lista komend:\n";
    std::cout << "ls                                            \twyswietlanie zasobow globalnych\n";
    std::cout
            << "ls-owners                                     \twyswietlanie zasobow globalnych razem z wlascicielem\n";
    std::cout
            << "ls-my                                         \twyswietlanie plikow w systemie, ktorych jestes wlascicielem\n";
    std::cout << "get [nazwa_pliku]                             \tpobranie pilku\n";
    std::cout << "put [sciezka_do_pliku]/[nazwa_pliku]          \tdodanie pliku do systemu\n";
    std::cout << "rm [nazwa_pliku]                              \tusuwa plik z systemu\n";
    std::cout << "exit                                          \twyjscie z programu\n";
}

void UserRequestHandler::waitForRequest() {

    printHelp();
    printPrompt();
    // dzieki std::ws nie mamy bialych znakow na poczatku wiersza
    for (std::string userInput; getline(std::cin >> std::ws, userInput, '\n');) {

        std::string requestPrefix;
        std::string requestSufix;

        RequestType requestType = preprocessRequest(userInput, requestPrefix, requestSufix);

        if (requestType == NOT_A_REQUEST) {
            printUnknownCommandMsg();
            printPrompt();
            std::cin.ignore(INT64_MAX);
            continue;
        }

        RequestResult requestRet = REQUEST_FAILURE;

        if (requestType == SINGLE_REQUEST) {
            requestRet = processRequest(requestPrefix);

        } else if (requestType == DOUBLE_REQUEST) {
            requestRet = processRequest(requestPrefix, requestSufix);
        }

        if (requestRet != REQUEST_SUCCESS) {
            printHelp();
        }
        printPrompt();
    }
}

RequestType
UserRequestHandler::preprocessRequest(std::string userInput, std::string &requestPrefix, std::string &requestSufix) {

    //usuwa spacje z konca strumienia
    const char whitespace[]{" \t\n"};
    const size_t last(userInput.find_last_not_of(whitespace));
    userInput = userInput.substr(0, last + 1);

    // sprawdzam ile jest slow
    // jesli sa dwa slowa to istnieje tylko jedna przerwa miedzy nimi

    // szukam poczatku przerwy
    const size_t firstWordEnd(userInput.find_first_of(whitespace));
    // jesli poczatek przerwy to npos, oznacza to ze jest to tylko jedno slowo ( nie ma bialych znakow w srodku )
    if (std::string::npos == firstWordEnd) {
        // sprawdzam czy uzytkownik wpisal pojedyncza komende czy jakies smieci
        if (std::find(singleCommands.begin(), singleCommands.end(), userInput) != singleCommands.end()) {
            requestPrefix = userInput;
            return SINGLE_REQUEST;
        }
    }

    // jesli poczatek przerwy jest w srodku slowa, szukam konca przerwy
    const size_t breakEnd(userInput.find_last_of(whitespace));

    // sprawdzam czy przerwa zawiera tylko biale znaki
    for (size_t i = firstWordEnd; i <= breakEnd; i++) {
        if (userInput[i] != ' ') {
            return NOT_A_REQUEST;
        }
    }
    // sprawdzam czy pierwsze slowo jest poleceniem ( jest jednym z prefiksow dwuczlonowego polecnia )
    std::string firstWord = userInput.substr(0, firstWordEnd);

    // zostanie zwrocone double request, jesli pierwsze slowo jest prefixem, lub false, jesli nie jest
    if (std::find(doubleCommandsPrefix.begin(), doubleCommandsPrefix.end(), firstWord)
        != doubleCommandsPrefix.end()) {
        requestPrefix = firstWord;
        requestSufix = userInput.substr(breakEnd + 1, userInput.size());
        return DOUBLE_REQUEST;
    }

    return NOT_A_REQUEST;
}

void UserRequestHandler::printUnknownCommandMsg() {
    std::cout << "Nieznana komenda - wybierz jedna z ponizszych\n";
}

UserRequestHandler::UserRequestHandler(P2PNode &node) : node(node) {

}

void UserRequestHandler::printPrompt() {
    std::cout << ">>> help - lista komend" << std::endl;
    std::cout << "TINBash> ";
}
