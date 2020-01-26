
#include <FileBroadcastStruct.h>
#include <cstring>
#include <utility>

FileBroadcastStruct::FileBroadcastStruct() {

}

void FileBroadcastStruct::setValues(std::string name_, std::string owner_, size_t size, bool isRevoked) {
    const char *n = name_.c_str();
    const char *o = owner_.c_str();
    strncpy(name, n, 63);
    strncpy(owner, o, 63);
    this->size = size;
    this->isRevoked = isRevoked;
}
