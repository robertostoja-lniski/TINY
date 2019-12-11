
#include <P2PRecord.h>
#include <P2PNode.h>
#include <vector>

#include "../include/P2PRecord.h"

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

std::vector<std::pair<size_t, std::string>> P2PRecord::getBroadcastCommunicates() {
    // maksymalna ilość plików w jednym komunikacie = 253
    std::vector<std::pair<size_t, std::string>> vector;

    size_t currentCommunicateSize = 0;
    std::string currentCommunicateString = "";
    for(auto file: fileSet){
        currentCommunicateString += file.getName() + '\t' + file.getOwner() + '\n';
        // jeśli przekroczono limit liczby plików
        if(++currentCommunicateSize == 253){
            vector.push_back(std::make_pair(currentCommunicateSize, currentCommunicateString));
            currentCommunicateSize = 0;
            currentCommunicateString = "";
        }
    }
    if(currentCommunicateSize != 0){
        vector.push_back(std::make_pair(currentCommunicateSize, currentCommunicateString));
    }
    return vector;
}

