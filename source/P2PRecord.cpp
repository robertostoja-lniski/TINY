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
    std::shared_lock<std::shared_mutex> lk(mutex);
    if (fileSet.empty()) {
        std::cout << "System globalny nie posiada plikow." << std::endl;
        return;
    }

    for (auto const &item : fileSet) {
        std::cout << item << std::endl;
    }
}

RecordOperationResult P2PRecord::removeFile(File file) {

    mutex.lock_shared();

    auto pos = fileSet.find(file);

    if (pos == fileSet.end()) {
        mutex.unlock_shared();
        return FILE_NOT_FOUND;
    }

    mutex.unlock_shared();

    mutex.lock();
    fileSet.erase(file);
    mutex.unlock();

    return SUCCESS;
}

std::unique_ptr<std::vector<Communicate>> P2PRecord::getBroadcastCommunicates() {
    auto N = Communicate::MAX_FILES_IN_COM;
    std::shared_lock<std::shared_mutex> lk(this->mutex);

    int fullCommunicatesCount = (int) fileSet.size() / N;
    const int lastCommunicateSize = (int) fileSet.size() % N;
    auto communicates = std::make_unique<std::vector<Communicate>>(fullCommunicatesCount, Communicate());
    communicates->push_back(Communicate(lastCommunicateSize));

    auto record_it = fileSet.begin();
    for(auto communicate : *communicates) {
        FileBroadcastStruct *file = communicate.files;
        auto filesCount = communicate.filesCount;

        for (auto i = 0; i < filesCount; ++i, ++file, ++record_it) {
            auto const& f = *record_it;
            file->setValues(f.getName(), f.getOwner(), f.getSize());
        }
    }
    return communicates;

}

