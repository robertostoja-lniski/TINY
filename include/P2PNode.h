#ifndef TINY_P2PNODE_H
#define TINY_P2PNODE_H

#include <string>
#include <thread>
#include <mutex>
#include "P2PRecord.h"
#include "GlobalFiles.h"
#include "LocalSystemHandler.h"
#include <future>
#include <netinet/in.h>
#include "Communicate.h"
#include "Defines.h"
#include "PackageLosingApi.h"

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
    unsigned long long bytes;
};

enum SHOW_GLOBAL_FILE_TYPE {
    DO_NOT_PRINT_OWNERS = 0,
    PRINT_OWNERS = 1,
};
/**
 * @class
 * Reprezentuje węzeł sieci P2P.
 * @implements wzorzec projektowy singleton
 */
class P2PNode {

private:
    /// Pliki globalne całego systemu, których nazwy są pobierane przez UDP.
    GlobalFiles globalFiles;

    /// Pliki lokalne.
    P2PRecord localFiles;
    /// Lokalna lista plkow uniewaznionych
    P2PRecord filesRevokedByMe;

    /// Set przechowujący informacje o unieważnionych przez lokalny węzeł plikach.
    /// W przypadku gdy ktoś dalej rozsyła unieważniony plik, komunikat jest ponawiany.

    LocalSystemHandler& handler;
    /// Port, na ktorym sluchamy na zadania transferu plikow
    int tcpPort;

    void sendFile(int, int, unsigned long long, unsigned long long);

    void handleDownloadRequests();

    /// @enum Rezultat operacji pobrania użytkownika
    enum GetUser {
        GET_USER_SUCCESS = 0,
        GET_USER_FAIL = 1,
    };

    /// uniewaznia plik
    struct Broadcast {
        /// Deskryptor gniazda UDP broadcast
        int sendSocketFd = -1;
        int recvSocketFd = -1;
        std::mutex preparationMutex, exitMutex;
        bool exit = false;
        std::thread sendThread, recvThread;
        std::chrono::seconds interval = std::chrono::seconds(5);

        const int UDP_BROADCAST_PORT = 7654;
        const char *UDP_BROADCAST_IP = BROADCAST_ADDR;
        struct sockaddr_in sendAddress, recvAddress;

    } broadcast;

    /// Inicjalizuje gniazdo do rozgłaszania
    void prepareForSendingBroadcast();

    void prepareForReceivingBroadcast();

    bool isRemoteIp(const std::string&);
public:

    explicit P2PNode(int, LocalSystemHandler&);

    // uniewaznia plik

    // pokazuje pliki w sytemie
    void showGlobalFiles();

    // zmienia tablice lokalnych plikow jesli pojawil sie nowy
    ActionResult updateLocalFiles();

    // rozglasza pliki po zmianie
    ActionResult startBroadcastingFiles();

    /// Rozpoczyna otrzymywanie deskryptorów plików od innych węzłów
    ActionResult startReceivingBroadcastingFiles();

    /// Pobiera plik o zadanej nazwie
    ActionResult downloadFile(const std::string &);

    /// Wrzuca plik do lokalnego systemu
    ActionResult uploadFile(const std::string&);

    /// Usuwa plik z lokalnego systemu
    ActionResult removeFile(std::string);

    /// Pokazuje pliki lokalne
    void showLocalFiles();

    /// Wysyła komunikat unieważnienia pliku
    ActionResult sendRevokeCommunicate(File);

    ActionResult startHandlingDownloadRequests();

    /// Destruktor
    virtual ~P2PNode();

    void requestAndDownloadFileFragment(fileRequest request, std::string ip_addr);

    ActionResult performDownloadingFileFragment(fileRequest request, std::string ip_addr);
};

#endif //TINY_P2PNODE_H
