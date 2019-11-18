#include "../include/File.h"

File::File(const std::string name, const std::string owner) : name(name), owner(owner) {}

const std::string File::getName() const {
    return name;
}

const std::string File::getOwner() const {
    return owner;
}
