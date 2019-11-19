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

enum RequestType{
    NOT_A_REQUEST = -1,
    SINGLE_REQUEST = 0,
    DOUBLE_REQUEST = 1,
};

enum RequestResult{
    REQUEST_SUCCESS = 0,
    REQUEST_FAILURE = 1,
};

// dla wo_owner nie wyswietla wlascicieli plikow
enum LsType{
    WO_OWNER = 0,
    W_OWNER = 1,
};
class UserRequestHandler{

private:
    // skorzystalem z wektora, poniewaz tablica musi miec podana dlugosc
    // przy deklaracji, a chcialem ograniczyc dodawanie kolejnych oznaczen
    const std::vector<std::string> singleCommands = {"ls-my", "ls", "ls-owners"};
    const std::vector<std::string> doubleCommandsPrefix = {"get", "add", "put", "rm"};
    LocalSystemHandler systemHandler;
    /*
     *  sprawdza czy pierwszy parametr jest poleceniem
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
    explicit UserRequestHandler(const LocalSystemHandler &systemHandler);
    // jedyne publiczne polecenie, odbiera komendy i je przetwarza
    void waitForRequest();
};
#endif //TINY_USERREQUESTHANDLER_H
