//
// Created by robert on 15.11.2019.
//

#ifndef TINY_FILE_H
#define TINY_FILE_H

#include <string>
#include <ostream>
#include <chrono>
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
    std::chrono::milliseconds timeout{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + std::chrono::milliseconds(10000000)};

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

    void setTimeout(std::chrono::milliseconds t);

    std::chrono::milliseconds getTimeout();

    bool isTimedOut() const;
};


#endif //TINY_FILE_H
