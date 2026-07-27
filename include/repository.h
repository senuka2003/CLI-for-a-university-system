#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "Exceptions.h"
#include <string>
#include <vector>
#include <fstream>

template <typename T>
class Repository {
public:
    void saveToFile(const std::string& filename, const std::vector<T>& data) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw DataCorruptionException(filename, "cannot open for writing");
        }
        for (const auto& item : data) {
            file << item.serialize() << "\n";
        }
    }

        std::vector<T> loadFromFile(const std::string& filename) const {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw DataCorruptionException(filename, "file not found");
        }

        std::vector<T> result;
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            if (line.empty()) continue;
            try {
                result.push_back(T::deserialize(line));
            } catch (const std::exception&) {
                throw DataCorruptionException(filename,
                    "malformed record at line " + std::to_string(lineNumber));
            }
        }
        return result;
    }
};

#endif // REPOSITORY_H