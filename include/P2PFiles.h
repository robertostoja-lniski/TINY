//
// Created by robert on 15.11.2019.
//

#ifndef TINY_P2PFILES_H
#define TINY_P2PFILES_H


#include <map>
#include "P2PRecord.h"

class P2PFiles {

private:
    /// Mapa rekordów setów plików z kluczem nazw
    std::map< std::string, P2PRecord > files;

    /// Wykorzystywane do wypisywania
    /// @synchronized
    std::map< std::string, P2PRecord > getFiles() const;

public:
    /// Aktualizuj konkretny rekord
    /// @synchronized
    void updateFiles(std::string name, P2PRecord newFiles);

    /// Pokaż wszystkie pliki
    /// @synchronized
    void showFiles();
};



#endif //TINY_P2PFILES_H
