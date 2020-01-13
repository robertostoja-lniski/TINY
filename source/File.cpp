
#include <File.h>

File::File(const std::string& name, const std::string& owner, const size_t size) : name(name), owner(owner), size(size){
    // Długość nazwy i właściciela pliku nie może być większa niż 128 bajtów
    if (name.length() > 128) {
        this->name = name.substr(0, 128);
    }
    if (owner.length() > 128) {
        this->owner = owner.substr(0, 128);
    }

}

std::string File::getName() const {
    return name;
}

std::string File::getOwner() const {
    return owner;
}

std::size_t File::getSize() const {
    return size;
}

File::File(FileBroadcastStruct &fileBroadcastStruct) {
    name = fileBroadcastStruct.name;
    owner = fileBroadcastStruct.owner;
    size = fileBroadcastStruct.size;
}


std::ostream &operator<<(std::ostream &os, const File &file) {
    os << "name: " << file.name << " ---|--- owner: " << file.owner << " ---|--- size: " << file.size;
    return os;
}
