//
// Created by Wojtek on 17/12/2019.
//

#include "GlobalFiles.h"

P2PRecord &GlobalFiles::operator[](P2PRecordPossessor possessor) {
    std::shared_lock<std::shared_mutex> lk(mutex);
    return files[possessor];
}

AddGlobalFileResult GlobalFiles::put(P2PRecordPossessor possessor, File file) {
    std::unique_lock<std::shared_mutex> lk(mutex);

    files[possessor].putFile(std::move(file));
    return ADD_GLOBAL_SUCCESS;
}

void GlobalFiles::revoke(File file) {
    std::unique_lock<std::shared_mutex> lk(mutex);

    // usuń plik z każdego węzła
    for (auto &record : files) {
        record.second.removeFile(file);
    }

}

void GlobalFiles::showFiles() {
    std::shared_lock<std::shared_mutex> lk(mutex);

    for(auto &record : files){
        std::cout << record.first.getName() << " (" + record.first.getIp() + ")" << ": " << std::endl;
        record.second.print();
    }
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