//
// Created by robert on 15.11.2019.
//

#ifndef TINY_GLOBALFILES_H
#define TINY_GLOBALFILES_H


#include <map>
#include <utility>
#include "P2PRecord.h"

enum AddGlobalFileResult {
    ADD_GLOBAL_SUCCESS = 0,
    ADD_GLOBAL_REVOKED = 1,
};

// dodalem w pliku .h bo to prosta funkcja potem mozna rozbic na .h i .cpp
class P2PRecordPossessor{

private:
    std::string name;
    std::string ip;

public:

    P2PRecordPossessor() = default;
    P2PRecordPossessor(std::string name, std::string ip) : name(std::move(name)), ip(std::move(ip)){}

    std::string getIp() const { return ip; }
    std::string getName() const { return name; }

    friend bool operator < (P2PRecordPossessor const &p1, P2PRecordPossessor const &p2)
    {
        return p1.name < p2.name;
    }

};

class GlobalFiles {

private:
    /// Mapa rekordów setów plików z kluczem nazw
    std::map<P2PRecordPossessor, P2PRecord> files;

    /// mutex synchronizujący mapę files
    std::shared_mutex mutex;



public:
    /// Dodaje plik do listy plików unieważnionych przez dany węzeł
    void addToFilesRevokedByMe(File file);

    /// Dodaje plik do listy plików danego węzła. Jeśli był unieważniony, zwraca wartość informującą o tym.
    AddGlobalFileResult put(P2PRecordPossessor possessor, File file);

    /// Aktualizuj konkretny rekord
    /// @synchronized
    void updateFiles(std::string name, P2PRecord newFiles);

    /// Pokaż wszystkie pliki
    /// @synchronized
    void showFiles();

    File getFileByName(const std::string &fileName);

    std::vector<P2PRecordPossessor> getFilePossessors(const std::string &fileName);
};


#endif //TINY_GLOBALFILES_H
