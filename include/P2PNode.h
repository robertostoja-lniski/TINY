#ifndef TINY_P2PNODE_H
#define TINY_P2PNODE_H

#include <string>
#include <thread>
#include <mutex>
#include "P2PRecord.h"
#include "P2PFiles.h"
#include <future>

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

/// @enum Typ komunikatu UDP. Pierwszy bajt komunikatu.
enum UDPCommunicateType{
    UDP_BROADCAST = 0,
    UDP_REVOKE = 1,
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

    struct Broadcast{
        /// Deskryptor gniazda UDP broadcast
        int socketFd = -1;
        std::mutex preparationMutex, exitMutex;
        /// @synchronized
        bool exit = false;
        std::thread sendThread, recvThread;
        std::chrono::seconds interval = std::chrono::seconds(5);
        std::chrono::seconds restartConnectionInterval = std::chrono::seconds(5);

        static const int UDP_BROADCAST_PORT = 7654;
    } broadcast;

    /// Inicjalizuje gniazdo do rozgłaszania
    /// @param restart czy restartujemy połączenie
    /// @synchronized tylko jeden wątek może przygotowywać się na broadcast
    ActionResult prepareForBroadcast(bool restart = false);

    /// PRYWATNE METODY

    /// Ustawia pole name na UNIXową nazwę użytkownika węzła
    static void setName();
public:
    explicit P2PNode();
    // uniewaznia plik
    ActionResult revoke(std::string);
    // pokazuje pliki w sytemie
    ActionResult showGlobalFiles(void);
    // zmienia tablice lokalnych plikow jesli pojawil sie nowy
    ActionResult updateLocalFiles(void);
    // rozglasza pliki po zmianie
    ActionResult startBroadcastingFiles();
    /// Rozpoczyna otrzymywanie deskryptorów plików od innych węzłów
    ActionResult startReceivingBroadcastingFiles();
    /// Pobiera plik o zadanej nazwie
    ActionResult downloadFile(const std::string&);

    /// Wrzuca plik do lokalnego systemu
    ActionResult uploadFile(std::string);

    /// Pokazuje pliki lokalne
    ActionResult showLocalFiles();
    /// Wysyła komunikat unieważnienia pliku
    ActionResult sendRevokeCommunicate(const File);

    /// Destruktor
    virtual ~P2PNode();
};

#endif //TINY_P2PNODE_H
