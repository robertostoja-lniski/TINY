#include <P2PRecord.h>
#include <P2PNode.h>
#include <vector>

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

    for (auto item : fileSet) {
        std::cout << item.getName() << " ( owner: " << item.getOwner() << " )\n";
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

std::vector<File> P2PRecord::getFiles() {
    std::vector<File> ret;
    
    mutex.lock_shared();
    
    for(const auto file : fileSet){
        ret.push_back(file);
    }
    
    mutex.unlock_shared();
    return ret;   

}

