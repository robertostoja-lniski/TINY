#ifndef TINY_P2PNODE_H
#define TINY_P2PNODE_H

#include <string>
#include "P2PRecord.h"
#include "P2PFiles.h"

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

class P2PNode {

private:
    std::string name;
    P2PFiles globalFiles;
    P2PRecord localFiles;

    /// Mutex synchronizujący dostęp
    static std::mutex singletonMutex;

    /// jedyny obiekt
    static P2PNode *node;

    /// PRYWATNE METODY


    /// Prywatny kontruktor.
    /// Jest wywoływany przez metodę P2PNode#getInstance
    explicit P2PNode();

    /// Ustawia pole name na UNIXową nazwę użytkownika węzła
    static void setName();
public:
    /**
     * @if obiekt nie istnieje @then tworzy obiekt.
     * @return obiekt (singleton)
     */
    static P2PNode &getInstance();
    // uniewaznia plik
    ActionResult revoke(std::string);
    // pokazuje pliki w sytemie
    ActionResult showGlobalFiles(void);
    // zmienia tablice lokalnych plikow jesli pojawil sie nowy
    ActionResult updateLocalFiles(void);
    // rozglasza informacje o plikach lokalnych co dana liczbe sekund
    ActionResult BroadcastFilesFrequently(double);
    // rozglasza pliki po zmianie
    ActionResult broadcastFiles();
    // sciaga plik o zadanej nazwie
    ActionResult downloadFile(const std::string&);
    // uploaduje plik
    ActionResult uploadFile(std::string);
    // pokazuje pliki lokalne
    ActionResult showLocalFiles();

    virtual ~P2PNode(){
        delete node;
    }
};
#endif //TINY_P2PNODE_H
