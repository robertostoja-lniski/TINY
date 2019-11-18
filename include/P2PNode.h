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
    ACTION_FAILURE = 1,
};

class P2PNode {

private:
    std::string name;
    P2PFiles globalFiles;
    P2PRecord localFiles;

public:
    explicit P2PNode(const std::string&);
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
};
#endif //TINY_P2PNODE_H
