//
// Created by Wojtek on 17/12/2019.
//

#include "P2PFiles.h"

P2PRecord &P2PFiles::operator[](P2PRecordPossessor possessor) {
    std::shared_lock<std::shared_mutex> lk(mutex);
    return files[possessor];
}

AddGlobalFileResult P2PFiles::add(P2PRecordPossessor possessor, File file) {
    std::unique_lock<std::shared_mutex> lk(mutex);

    files[possessor].addFile(std::move(file));
    return ADD_GLOBAL_SUCCESS;
}

void P2PFiles::revoke(File file) {
    std::unique_lock<std::shared_mutex> lk(mutex);

    // usuń plik z każdego węzła
    for (auto &record : files) {
        record.second.removeFile(file);
    }

}

void P2PFiles::showFiles() {
    std::shared_lock<std::shared_mutex> lk(mutex);

    for(auto &record : files){
        std::cout << record.first.getName() << " (" + record.first.getIp() + ")" << ": " << std::endl;
        record.second.print();
    }
}

