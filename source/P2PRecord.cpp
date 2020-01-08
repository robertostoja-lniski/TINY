#include <P2PRecord.h>
#include <P2PNode.h>
#include <vector>

AddFileResult P2PRecord::addFile(File file) {

    mutex.lock_shared();

    auto it = fileSet.find(file);
    if (it != fileSet.end()) {
        return ADD_ALREADY_EXISTS;
    }

    mutex.unlock_shared();


    mutex.lock();
    fileSet.insert(file);
    mutex.unlock();

    return ADD_SUCCESS;
}

void P2PRecord::print() {
    mutex.lock_shared();
    if (fileSet.empty()) {
        std::cout << "Nie posiadasz plikow w systemie\n";
    }
    for (auto item : fileSet) {

        std::cout << item.getName() << " ( owner: " << item.getOwner() << " )\n";
    }
    mutex.unlock_shared();
}

RecordOperationResult P2PRecord::removeFile(File file) {

    mutex.lock_shared();

    auto pos = fileSet.find(file);

    if (pos == fileSet.end()) {
        return FILE_NOT_FOUND;
    }

    mutex.unlock_shared();

    mutex.lock();
    fileSet.erase(file);
    mutex.unlock();

    return SUCCESS;
}

std::vector<std::pair<u_short, std::string>> P2PRecord::getBroadcastCommunicates() {
    // maksymalna ilość plików w jednym komunikacie = 253
    std::vector<std::pair<u_short, std::string>> vector;

    u_short currentCommunicateSize = 0;
    std::string currentCommunicateString = "";

    mutex.lock_shared();
    for (auto file: fileSet) {
        currentCommunicateString += file.getName() + '\t' + file.getOwner() + '\n';
        // jeśli przekroczono limit liczby plików
        if (++currentCommunicateSize == 253) {
            vector.push_back(std::make_pair(currentCommunicateSize, currentCommunicateString));
            currentCommunicateSize = 0;
            currentCommunicateString = "";
        }
    }
    mutex.unlock_shared();
    if (currentCommunicateSize != 0) {
        vector.push_back(std::make_pair(currentCommunicateSize, currentCommunicateString));
    }
    return vector;
}

