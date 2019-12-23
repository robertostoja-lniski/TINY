//
// Created by Wojtek on 17/12/2019.
//

#include "P2PFiles.h"

P2PRecord &P2PFiles::operator[](std::string nodeName) {
    std::shared_lock<std::shared_mutex> lk(mutex);
    return files[nodeName];
}

AddGlobalFileResult P2PFiles::add(std::string node, File file) {
    std::unique_lock<std::shared_mutex> lk(mutex);

    auto it = filesRevokedByMe.find(file);
    if(it == filesRevokedByMe.end()) {
        // jeśli nie ma pliku na liście unieważnionych, to dodaj plik
        files[node].addFile(std::move(file));
        return ADD_GLOBAL_SUCCESS;
    }
    // jeśli plik jest na liście unieważnionych, to ponów komunikat o unieważnieniu
    return ADD_GLOBAL_REVOKED;
}

void P2PFiles::revoke(File file) {
    std::unique_lock<std::shared_mutex> lk(mutex);

    // usuń plik z każdego węzła
    for(auto &record : files){
        record.second.removeFile(file);
    }

}

void P2PFiles::addToFilesRevokedByMe(File file) {
    std::unique_lock<std::shared_mutex> lk(mutex);

    filesRevokedByMe.insert(file);
}

