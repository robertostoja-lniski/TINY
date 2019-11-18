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
    ACTION_SUCCESS = 0,
    ACTION_FAILURE = 1,
};

class P2PNode {

private:

    std::string name;
    P2PFiles globalFiles;
    P2PRecord localFiles;

public:
    P2PNode(const std::string name);
    // uniewaznia plik
    ActionResult revoke(std::string);
    // pokazuje pliki w sytemie
    ActionResult showGlobalFiles(void);
    // zmienia tablice lokalnych plikow jesli pojawil sie nowy
    ActionResult updateLocalFiles(void);
    // rozglasza informacje o plikach lokalnych co dana liczbe sekund
    ActionResult BroadcastFilesFrequently(double);
    // rozglasza pliki po zmianie
    ActionResult BroadcastFiles();
    // sciaga plik
    ActionResult downloadFile(std::string);
    // upload file
    ActionResult uploadFile(std::string);
    void showLocalFiles();
};
#endif //TINY_P2PNODE_H
