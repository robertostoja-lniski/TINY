//
// Created by robert on 15.11.2019.
//

#ifndef TINY_P2PFILES_H
#define TINY_P2PFILES_H


#include <map>
#include "P2PRecord.h"

enum AddGlobalFileResult {
    ADD_GLOBAL_SUCCESS = 0,
    ADD_GLOBAL_REVOKED = 1,
};

class P2PFiles {

private:
    /// Mapa rekordów setów plików z kluczem nazw
    std::map<std::string, P2PRecord> files;

    /// mutex synchronizujący mapę files
    std::shared_mutex mutex;

    /// Wykorzystywane do wypisywania
    /// @synchronized
    std::map<std::string, P2PRecord> getFiles() const;

    /// Set przechowujący informacje o unieważnionych przez lokalny węzeł plikach.
    /// W przypadku gdy ktoś dalej rozsyła unieważniony plik, komunikat jest ponawiany.
    std::set<File> filesRevokedByMe;

    /// Getter dla rekordu połączonego z danym węzłem
    /// @param nodeName nazwa węzła
    /// na razie niepotrzebny
    P2PRecord &operator[](std::string);

public:
    /// Dodaje plik do listy plików unieważnionych przez dany węzeł
    void addToFilesRevokedByMe(File file);

    /// Dodaje plik do listy plików danego węzła. Jeśli był unieważniony, zwraca wartość informującą o tym.
    AddGlobalFileResult add(std::string node, File file);

    /// Usuwa plik ze wszytskich list
    void revoke(File file);

    /// Aktualizuj konkretny rekord
    /// @synchronized
    void updateFiles(std::string name, P2PRecord newFiles);

    /// Pokaż wszystkie pliki
    /// @synchronized
    void showFiles();
};


#endif //TINY_P2PFILES_H
