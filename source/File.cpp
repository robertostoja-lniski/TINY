#include "../include/File.h"

File::File(const std::string name, const std::string owner) : name(name), owner(owner) {
    // Długość nazwy i właściciela pliku nie może być większa niż 128 bajtów
    if(name.length() > 128){
        this->name = name.substr(0, 128);
    }
    if(owner.length() > 128){
        this->owner = owner.substr(0, 128);
    }
}

const std::string File::getName() const {
    return name;
}

const std::string File::getOwner() const {
    return owner;
}
