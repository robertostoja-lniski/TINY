//
// Created by robert on 15.11.2019.
//

#ifndef TINY_FILE_H
#define TINY_FILE_H

#include <string>
#include <ostream>
#include "FileBroadcastStruct.h"

/**
 * @class
 * Reprezentuje plik lokalny lub globalny.
 * Posiada nazwę i właściciela.
 * Posiada operator porównania
 * @author Robert
 */
class File {

private:
    std::string name;
    std::string owner;
    size_t size;
    bool isRevoked {false};

public:
    File() = default;
    File(const std::string& name, const std::string& owner, size_t size);
    explicit File(FileBroadcastStruct &fileBroadcastStruct);

    /// Getter dla właściciela
    [[nodiscard]] std::string getOwner() const;

    /// Getter dla nazwy
    [[nodiscard]] std::string getName() const;

    ///Getter dla rozmiaru
    [[nodiscard]] std::size_t getSize() const;

    bool getIsRevoked() const;

    /**
     * Operator porównania.
     * Niezbędny do istnienia seta. (Set musi mieć operator porównania)
     * @param file referencja do pliku porównywanego
     * @return czy nazwa plik jest alfabetycznie większa od drugiej nazwy
     */
    bool operator<(File const &file) const {
        return name < file.getName();
    }

    friend std::ostream &operator<<(std::ostream &os, const File &file);

    void setRevoked(bool);
};


#endif //TINY_FILE_H
