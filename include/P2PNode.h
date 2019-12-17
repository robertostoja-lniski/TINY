#ifndef TINY_P2PNODE_H
#define TINY_P2PNODE_H

#include <string>
#include <thread>
#include "P2PRecord.h"
#include "P2PFiles.h"
#include <future>

/*
 * mysle ze lepiej dac jednakowy zwracany enum
 * dla kilku metod zamiast zwracac inta,
 * przy sprawdzaniu poprawnosci wykonania zadania
 * bedzie wieksza czytelnosc jesli uzyjemy enum
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

class P2PNode {

private:
    std::string name;
    P2PFiles globalFiles;
    P2PRecord localFiles;

    struct Broadcast{
        /// Deskryptor gniazda UDP broadcast
        int socketFd = -1;
        std::promise<bool> exit;
        std::thread thread;
        std::chrono::seconds interval = std::chrono::seconds(5);
        std::chrono::seconds restartConnectionInterval = std::chrono::seconds(5);

        static const int UDP_BROADCAST_PORT = 7654;
    } broadcast;

    /// Inicjalizuje gniazdo do rozgłaszania
    ActionResult prepareForBroadcast();
public:
    explicit P2PNode(const std::string&);
    // uniewaznia plik
    ActionResult revoke(std::string);
    // pokazuje pliki w sytemie
    ActionResult showGlobalFiles(void);
    // zmienia tablice lokalnych plikow jesli pojawil sie nowy
    ActionResult updateLocalFiles(void);
    // rozglasza pliki po zmianie
    ActionResult startBroadcastingFiles();
    // sciaga plik o zadanej nazwie
    ActionResult downloadFile(const std::string&);
    // uploaduje plik
    ActionResult uploadFile(std::string);
    // pokazuje pliki lokalne
    ActionResult showLocalFiles();

    /// Destruktor
    virtual ~P2PNode();
};
#endif //TINY_P2PNODE_H
