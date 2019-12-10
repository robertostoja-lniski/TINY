#ifndef TINY_P2PNODE_H
#define TINY_P2PNODE_H

#include <string>
#include "P2PRecord.h"
#include "P2PFiles.h"

/**
 * @enum
 * Rezultat akcji wykonywanej przez P2PNode
 * @see P2PNode
 */
enum ActionResult {
    ACTION_NOT_HANDLED = -1,
    ACTION_SUCCESS = 0,
    ACTION_NO_EFFECT = 1,
    ACTION_FAILURE = 2,
};

/**
 * @class
 * Reprezentuje węzeł sieci P2P.
 * @implements wzorzec projektowy singleton
 */
class P2PNode {

private:
    /// Nazwa węzła. Jest to nazwa użytkownika systemu UNIX.
    std::string name;

    /// Pliki globalne całego systemu, których nazwy są pobierane przez UDP.
    P2PFiles globalFiles;

    /// Pliki lokalne.
    P2PRecord localFiles;

public:
    explicit P2PNode(const std::string&);

    /// Unieważnia plik, którego jest właścicielem. Rozsyła komunikat o unieważnieniu do wszystkich.
    ActionResult revoke(std::string);

    /// Pokazuje listę globalnych plików w systemie.
    ActionResult showGlobalFiles();

    /// Zmienia tablicę lokalnych plików. Pobiera informacje z ukrytego katalogu i pliku konfiguracyjnego.
    ActionResult updateLocalFiles();

    /// Rozgłasza informacje o plikach lokalnych co daną liczbę sekund w osobnym wątku.
    ActionResult BroadcastFilesFrequently(double);

    /// Rozgłasza pliki do wszystkich węzłów
    ActionResult broadcastFiles();

    /// Pobiera plik o zadanej nazwie
    ActionResult downloadFile(const std::string&);

    /// Wrzuca plik do lokalnego systemu
    ActionResult uploadFile(std::string);

    /// Pokazuje pliki lokalne
    ActionResult showLocalFiles();
};
#endif //TINY_P2PNODE_H
