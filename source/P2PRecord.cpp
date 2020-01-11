#include <P2PRecord.h>
#include <P2PNode.h>
#include <vector>
#include <FileBroadcastStruct.h>
#include <memory>

AddFileResult P2PRecord::addFile(File file) {

    mutex.lock_shared();

    auto it = fileSet.find(file);
    if (it != fileSet.end()) {
        mutex.unlock_shared();
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
        mutex.lock_shared();
        return;
    }

    for (auto const &item : fileSet) {
        std::cout << item << std::endl;
    }
    mutex.unlock_shared();
}

RecordOperationResult P2PRecord::removeFile(File file) {

    mutex.lock_shared();

    auto pos = fileSet.find(file);

    if (pos == fileSet.end()) {
        mutex.lock_shared();
        return FILE_NOT_FOUND;
    }

    mutex.unlock_shared();

    mutex.lock();
    fileSet.erase(file);
    mutex.unlock();

    return SUCCESS;
}

std::unique_ptr<std::vector<Communicate>> P2PRecord::getBroadcastCommunicates() {
    // maksymalna ilość plików w jednym komunikacie = 253
    auto communicates = std::make_unique<std::vector<Communicate>>();

    mutex.lock_shared();
    communicates->push_back(Communicate());
    auto &currentCommunicatesVector = communicates->back().files;
    for (auto file: fileSet) {
        FileBroadcastStruct fileBroadcastStruct(file.getName(), file.getOwner(), file.getSize());
        currentCommunicatesVector.push_back(fileBroadcastStruct);
        if (communicates->back().files.size() >= 256) {
            communicates->push_back(Communicate());
            currentCommunicatesVector = communicates->back().files;

        }
    }
    mutex.unlock_shared();
    return communicates;
}

