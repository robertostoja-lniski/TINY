//
// Created by Wojtek on 17/12/2019.
//

#include "GlobalFiles.h"

P2PRecord &GlobalFiles::operator[](P2PRecordPossessor possessor) {
    std::shared_lock<std::shared_mutex> lk(mutex);
    return files[possessor];
}

AddGlobalFileResult GlobalFiles::put(P2PRecordPossessor possessor, File file) {
    mutex.lock_shared();

    files[possessor].putFile(std::move(file));
    mutex.unlock_shared();

    return ADD_GLOBAL_SUCCESS;
}

void GlobalFiles::showFiles() {
    mutex.lock_shared();

    for(auto &record : files){
        std::cout << record.first.getName() << " (" + record.first.getIp() + ")" << ": " << std::endl;
        record.second.print();
    }
    mutex.unlock_shared();
}

File GlobalFiles::getFileByName(const std::string &fileName) {
    for (auto & [possessor, possessorFiles] : files){
        for (const auto& file : possessorFiles.getFileSet()) {
            if(file.getName() == fileName){
                return file;
            }
        }
    }
    throw std::runtime_error("nie ma takiego pliku");
}

std::vector<P2PRecordPossessor> GlobalFiles::getFilePossessors(const std::string& fileName) {
    std::vector <P2PRecordPossessor> possessorsIPs;
    for (auto & [possessor, possessorsFiles] : files){
        for (const auto& file : possessorsFiles.getFileSet()){
            if (fileName == file.getName()){
                possessorsIPs.push_back(possessor);
            }
        }
    }
    return possessorsIPs;
}