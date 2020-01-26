#include <P2PRecord.h>
#include <vector>
#include <FileBroadcastStruct.h>

AddFileResult P2PRecord::putFile(const File& file) {
    auto it = getFileByName(file.getName());
    // poniewaz w secie nie da sie zmieniac wartosci to usuwamy i dodajemy
    if (it != nullptr) {
        removeFile(file);
    }

    mutex.lock();
    fileSet.insert(file);
    mutex.unlock();

    return ADD_SUCCESS;
}

void P2PRecord::print() {
    mutex.lock_shared();
    for (auto const &item : fileSet) {
        std::cout << item << std::endl;
    }
    mutex.unlock_shared();
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
    mutex.lock_shared();

    std::vector<Communicate> communicates;

    // kopiujemy set na wektor zeby bylo nam latwiej
    std::vector<File> files;
    for (const auto& file:fileSet) {
        files.push_back(file);
    }

    mutex.unlock_shared();

    size_t filesLeftToAdd = files.size();
    size_t fileID = 0;
    while(filesLeftToAdd > 0){
        size_t filesToAdd = std::min(filesLeftToAdd, (size_t)MAX_FILES_IN_COM);
        auto communicate = Communicate(filesToAdd, owner);

        for (int i = 0; i < filesToAdd; i++) {
            communicate.files[fileID].setValues(files[fileID].getName(), files[fileID].getOwner(),
                                                files[fileID].getSize(), files[fileID].getIsRevoked());
        }
        communicates.push_back(communicate);
        filesLeftToAdd -= filesToAdd;
    }

    return communicates;

}

const std::set<File> &P2PRecord::getFileSet() const {
    return fileSet;
}

RecordOperationResult P2PRecord::revokeFile(File file) {
    File newFile(file.getName(), file.getOwner(), file.getSize());
    newFile.setRevoked(true);

    this->putFile(newFile);
    return SUCCESS;
}

const File * P2PRecord::getFileByName(std::string fileName) {
    mutex.lock_shared();
    for (auto& file : fileSet) {
        if (file.getName() == fileName) {
            mutex.unlock_shared();
            return &file;
        }
    }
    mutex.unlock_shared();
    return nullptr;
}

