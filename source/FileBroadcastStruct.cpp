
#include <FileBroadcastStruct.h>
#include <cstring>
#include <utility>

FileBroadcastStruct::FileBroadcastStruct(std::string name_, std::string owner_, size_t size_) {
    setValues(std::move(name_), owner_, size_);
}

FileBroadcastStruct::FileBroadcastStruct() {

}

void FileBroadcastStruct::setValues(std::string name_, std::string owner_, size_t size_) {
    const char *n = name_.c_str();
    const char *o = owner_.c_str();
    strncpy(name, n, 63);
    strncpy(owner, o, 63);
    size = size_;
}
