#ifndef TINY_P2PNODE_H
#define TINY_P2PNODE_H

#include <string>
#include <mutex>
#include "P2PRecord.h"
#include "P2PFiles.h"

#define MAX_USERNAME_LEN 20
#define MAX_FILENAME_LEN 50
#define MAX_TCP_CONNECTIONS 5

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

struct fileRequest {
    // dodac limity na
    char fileName[MAX_FILENAME_LEN];
    unsigned long long offset;
    unsigned long long bytesCount;
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

    /// Port, na ktorym sluchamy na zadania transferu plikow
    int tcpPort;

    /// Ustawia pole name na UNIXową nazwę użytkownika węzła
    static void setName();

    void sendFile(int, int, unsigned long long, unsigned long long);

    void handleDownloadRequests();
public:

    explicit P2PNode(int);

    /// uniewaznia plik
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

    ActionResult startHandlingDownloadRequests();

    ActionResult requestFile(std::string);
};

#endif //TINY_P2PNODE_H
