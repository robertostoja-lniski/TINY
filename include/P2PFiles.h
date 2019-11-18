//
// Created by robert on 15.11.2019.
//

#ifndef TINY_P2PFILES_H
#define TINY_P2PFILES_H


#include <map>
#include "P2PRecord.h"

class P2PFiles {

private:
    std::map< std::string, P2PRecord > files;
    // wykorzystywane do wypisywania
    std::map< std::string, P2PRecord > getFiles() const;

public:
    // dostep powinien byc synchronizowany
    void updateFiles(std::string name, P2PRecord newFiles);
    // odczyt powinien byc synchronizowany
    void showFiles();
};



#endif //TINY_P2PFILES_H
