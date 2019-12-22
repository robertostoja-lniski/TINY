
#include <File.h>

File::File(const std::string name, const std::string owner) : name(name), owner(owner) {}

const std::string File::getName() const {
    return name;
}

const std::string File::getOwner() const {
    return owner;
}

void File::setOwner(const std::string owner) {
    this->owner = owner;
}

void File::setName(const std::string name) {
    this->name = name;
}
