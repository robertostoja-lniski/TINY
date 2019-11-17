//
// Created by robert on 15.11.2019.
//

#ifndef TINY_FILE_H
#define TINY_FILE_H

#include <string>

class File {

private:
    std::string name;
    std::string owner;
public:
    File(const std::string name, const std::string owner);

    void setName(const std::string name);

    void setOwner(const std::string owner);

    const std::string getOwner() const;

    const std::string getName() const;

    // bez tego operatora nie mozna budowac seta plikow
    // set musi miec operator porownania
    bool operator<(File const &file) const{
        return name < file.getName();
    }
};


#endif //TINY_FILE_H
