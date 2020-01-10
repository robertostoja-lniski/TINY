
#include <FileBroadcastStruct.h>
#include <cstring>

FileBroadcastStruct::FileBroadcastStruct(std::string name_, std::string owner_, size_t size_) {
    strncpy(name, name_.c_str(), 63);
    strncpy(owner, owner_.c_str(), 63);
    size = size_;
}

FileBroadcastStruct::FileBroadcastStruct() {

}