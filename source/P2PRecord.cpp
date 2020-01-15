#include <P2PRecord.h>
#include <vector>
#include <FileBroadcastStruct.h>

AddFileResult P2PRecord::addFile(const File& file) {

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

RecordOperationResult P2PRecord::removeFile(const File& file) {

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

std::vector<Communicate> P2PRecord::getBroadcastCommunicates(const std::string& owner) {
    // maksymalnie MAX_FILES_IN_COM plikow w komunikacie, wiec musimy wyslac kilka komunikatow
    std::shared_lock<std::shared_mutex> lk(this->mutex);

    std::vector<Communicate> communicates;

    // kopiujemy set na wektor zeby bylo nam latwiej
    std::vector<File> files;
    for (const auto& file:fileSet) {
        files.push_back(file);
    }

    size_t filesLeftToAdd = files.size();
    size_t fileID = 0;
    while(filesLeftToAdd > 0){
        size_t filesToAdd = std::min(filesLeftToAdd, (size_t)MAX_FILES_IN_COM);
        auto communicate = Communicate(filesToAdd, owner);

        for (int i = 0; i < filesToAdd; i++) {
            communicate.files[fileID].setValues(files[fileID].getName(), files[fileID].getOwner(),
                                                files[fileID].getSize());
        }
        communicates.push_back(communicate);
        filesLeftToAdd -= filesToAdd;
    }

    return communicates;

}

const std::set<File> &P2PRecord::getFileSet() const {
    return fileSet;
}

