//
// Created by robert on 15.11.2019.
//

#ifndef TINY_FILE_H
#define TINY_FILE_H

#include <string>

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
    std::size_t size;
public:
    File(const std::string name, const std::string owner, const size_t size);

    /// Setter dla nazwy
    void setName(const std::string name);

    /// Setter dla właściciela
    void setOwner(const std::string owner);

    /// Getter dla właściciela
    const std::string getOwner() const;

    /// Getter dla nazwy
    const std::string getName() const;

    ///Getter dla rozmiaru
    const std::size_t getSize() const;

    /**
     * Operator porównania.
     * Niezbędny do istnienia seta. (Set musi mieć operator porównania)
     * @param file referencja do pliku porównywanego
     * @return czy nazwa plik jest alfabetycznie większa od drugiej nazwy
     */
    bool operator<(File const &file) const {
        return name < file.getName();
    }
};


#endif //TINY_FILE_H
