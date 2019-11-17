#include "../include/P2PRecord.h"

void P2PRecord::addFile(File file) {
    fileSet.insert(file);
}

void P2PRecord::print() {
    if(fileSet.empty()) {
        std::cout << "There are no uploaded files yet\n";
    }
    for(auto item : fileSet) {
        std::cout << item.getName() << " ( owner: " << item.getOwner() << " )\n";
    }
}

RecordOperationResult P2PRecord::removeFile(File file) {

    auto pos = fileSet.find(file);

    if( pos == fileSet.end()) {
        std::cout << "File requested to remove does not exists\n";
        return FILE_NOT_FOUND;
    }

    fileSet.erase(file);
    return SUCCESS;
}