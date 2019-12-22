#include "P2PRecord.h"

AddFileResult P2PRecord::addFile(File file) {

    auto it = fileSet.find(file);
    if(it != fileSet.end()) {
        return ADD_ALREADY_EXISTS;
    }

    fileSet.insert(file);
    return ADD_SUCCESS;
}

void P2PRecord::print() {
    if(fileSet.empty()) {
        std::cout << "Nie posiadasz plikow w systemie\n";
    }
    for(auto item : fileSet) {
        std::cout << item.getName() << " ( owner: " << item.getOwner() << " )\n";
    }
}

RecordOperationResult P2PRecord::removeFile(File file) {

    auto pos = fileSet.find(file);

    if( pos == fileSet.end()) {
        return FILE_NOT_FOUND;
    }

    fileSet.erase(file);
    return SUCCESS;
}