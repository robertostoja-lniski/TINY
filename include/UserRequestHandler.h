//
// Created by robert on 17.11.2019.
//

#ifndef TINY_USERREQUESTHANDLER_H
#define TINY_USERREQUESTHANDLER_H

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include "LocalSystemHandler.h"

/// @enum typ zapytania
enum RequestType{
    NOT_A_REQUEST = -1,
    SINGLE_REQUEST = 0,
    DOUBLE_REQUEST = 1,
};

/// @enum rezultat zapytania
enum RequestResult{
    REQUEST_SUCCESS = 0,
    REQUEST_FAILURE = 1,
};

/// @enum typ wypisywania
enum LsType{
    WO_OWNER = 0, /// nie wyświetla właścicieli plików
    W_OWNER = 1,  ///     wyświetla właścicieli plików
};

/** @class
 * Obsługuje zapytania użytkownika.
 * @author Robert
 */
class UserRequestHandler{

private:

    /// Pojedyncze komendy - vector użyty w celu uproszczenia deklaracji
    const std::vector<std::string> singleCommands = {"ls-my", "ls", "ls-owners"};

    /// Podwójne komendy
    const std::vector<std::string> doubleCommandsPrefix = {"get", "add", "put", "rm"};

    /// Lokalny kontroler systemu
    LocalSystemHandler localSystemHandler;
    /**
     *  Sprawdza czy pierwszy parametr jest poleceniem
     *  jesli tak, to zwaraca rodzaj polecania ( jedno, lub dwuargumentowe )
     *  i w zaleznosci od rodzaju modyfikuje argumenty wejsciowe,
     *  w taki sposob ze przypisane im sa oddzielne slowa polecenia
     *  np po otrzymaniu:      get          /home/robert/abcd
     *  zwroci DOUBLE_REQUEST, drugi argument bedzie mial wartosc "get", a trzeci "/home/robert/abcd"
     *  po otrzymaniu        unixy   tiny
     *  zwroci NOT_A_REQUEST i nie zmieni argumentu drugiego i trzeciego
     */
    RequestType preprocessRequest(std::string, std::string&, std::string&);
    // dla pojedynczych polecen
    RequestResult processRequest(const std::string&);
    // dla podwojnych polecen
    RequestResult processRequest(const std::string&, const std::string&);
    static void printHelp();
    static void printErrorMessage();

public:
    /// Konstruktor
    explicit UserRequestHandler(const LocalSystemHandler &systemHandler);

    /// Odbiera komendy i je przetwarza
    void waitForRequest();
};
#endif //TINY_USERREQUESTHANDLER_H
