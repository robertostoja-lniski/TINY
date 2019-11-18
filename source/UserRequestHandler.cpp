
#include "../include/UserRequestHandler.h"

void UserRequestHandler::processRequest(std::string request) {

    // logike podepne potem
    std::cout << "wybrano " + request + "\n";
    return;
}

void UserRequestHandler::processRequest(std::string requestPrefix, std::string requestSufix) {

    // logike podepne potem
    std::cout << "wybrano " + requestPrefix + " " + requestSufix + "\n";
    return;
}

void UserRequestHandler::printHelp() {
    
    std::cout << "Lista komend:\n";
    std::cout << "lls\twyswietlanie zasobow lokalnych\n";
    std::cout << "ls\twyswietlanie zasobow globalnych\n";
    std::cout << "ls-owners\twyswietlanie zasobow globalnych razem z wlascicielem\n";
    std::cout << "get [nazwa_pliku]\tpobranie pilku";
    std::cout << "set-workspace [sciezka_do_folderu_roboczego\ttworzy folder roboczy\n";
    std::cout << "add [sciezka_do_pliku]/[nazwa_pliku]]\tdodanie pliku do folderu roboczego\n";
    std::cout << "put [nazwa_pliku]\tdodanie pliku do systemu\n";
    std::cout << "sys-rm [nazwa_pliku]\tusuwa plik z systemu\n";
    std::cout << "full-rm [nazwa_pliku\tusuwa plik z systemu i folderu roboczego\n";
}

void UserRequestHandler::receiveRequest() {

    printHelp();

    // dzieki std::ws nie mamy bialych znakow na poczatku wiersza
    for(std::string userInput; getline(std::cin>>std::ws, userInput, '\n');) {

        std::string requestPrefix = "";
        std::string requestSufix = "";

        RequestType requestType = preprocessRequest(userInput, requestPrefix, requestSufix);

        if(requestType == NOT_A_REQUEST) {
            printErrorMessage();
            printHelp();
            continue;
        }

        if(requestType == SINGLE_REQUEST) {
            processRequest(requestPrefix);

        } else if(requestType == DOUBLE_REQUEST){
            processRequest(requestPrefix, requestSufix);
        }
    }
}

RequestType UserRequestHandler::preprocessRequest(std::string userInput, std::string& requestPrefix, std::string& requestSufix) {

    //usuwa spacje z konca strumienia
    const char whitespace[] {" \t\n"};
    const size_t last(userInput.find_last_not_of(whitespace));
    userInput = userInput.substr(0, last + 1);

    // sprawdzam ile jest slow
    // jesli sa dwa slowa to istnieje tylko jedna przerwa miedzy nimi

    // szukam poczatku przerwy
    const size_t firstWordEnd(userInput.find_first_of(whitespace));
    // jesli poczatek przerwy to npos, oznacza to ze jest to tylko jedno slowo ( nie ma bialych znakow w srodku )
    if(std::string::npos == firstWordEnd) {
        // sprawdzam czy uzytkownik wpisal pojedyncza komende czy jakies smieci
        if(std::find(singleCommands.begin(), singleCommands.end(), userInput) != singleCommands.end()){
            requestPrefix = userInput;
            return SINGLE_REQUEST;
        }
    }

    // jesli poczatek przerwy jest w srodku slowa, szukam konca przerwy
    const size_t secondWordBegin(userInput.find_last_of(whitespace));

    // sprawdzam czy przerwa zawiera tylko biale znaki
    for(size_t i = firstWordEnd; i <= secondWordBegin; i++) {
        if(userInput[i] != ' ') {
            return NOT_A_REQUEST;
        }
    }
    // sprawdzam czy pierwsze slowo jest poleceniem ( jest jednym z prefiksow dwuczlonowego polecnia )
    std::string firstWord = userInput.substr(0, firstWordEnd);

    // zostanie zwrocone double request, jesli pierwsze slowo jest prefixem, lub false, jesli nie jest
    if(std::find(doubleCommandsPrefix.begin(), doubleCommandsPrefix.end(), firstWord)
        != doubleCommandsPrefix.end()){
        requestPrefix = firstWord;
        requestSufix = userInput.substr(secondWordBegin, userInput.size());
        return DOUBLE_REQUEST;
    }

    return NOT_A_REQUEST;
}

void UserRequestHandler::printErrorMessage() {
    std::cout << "Nieznana komenda - wybierz jedna z ponizszych\n";
}
