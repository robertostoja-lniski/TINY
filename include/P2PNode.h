#ifndef TINY_P2PNODE_H
#define TINY_P2PNODE_H

#include <string>
#include "P2Precord.h"
#include "P2PFiles.h"

/*
 * mysle ze lepiej dac jednakowy zwracany enum
 * dla kilku metod zamiast zwracac inta,
 * przy sprawdzaniu poprawnosci wykonania zadania
 * bedzie wieksza czytelnosc jesli uzyjemy enum
 */
enum ActionResult {
    SUCCESS = 0,
    FAILURE = 1,
};

class P2PNode {

private:

    std::string name;
    P2PFiles globalFiles;
    P2Precord localFiles;

public:
    P2PNode(const std::string &name);
    // uniewaznia plik
    ActionResult revoke(File);
    // pokazuje pliki w sytemie
    ActionResult showGlobalFiles(void);
    // zmienia tablice lokalnych plikow jesli pojawil sie nowy
    ActionResult updateLocalFiles(void);
    // rozglasza informacje o plikach lokalnych co dana liczbe sekund
    ActionResult BroadcastFiles(double);
    // sciaga plik
    ActionResult downloadFile(File);

};
#endif //TINY_P2PNODE_H
