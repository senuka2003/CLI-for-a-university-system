#ifndef COURSE_RECORD_H
#define COURSE_RECORD_H

#include <string>
#include <sstream>
#include <stdexcept>

class CourseRecord {
public:
    std::string type;       // "Lecture", "Lab", "Project"
    std::string code;
    std::string title;
    int capacity;
    std::string lecturerId;
    std::string prereqCode;

    CourseRecord() : capacity(0) {}
    CourseRecord(std::string type, std::string code, std::string title, int capacity, std::string lecturerId, std::string prereqCode = "NONE")
        : type(std::move(type)), code(std::move(code)), title(std::move(title)),
          capacity(capacity), lecturerId(std::move(lecturerId)), prereqCode(std::move(prereqCode)) {}

    std::string serialize() const {
        return type + "," + code + "," + title + "," + std::to_string(capacity) + "," + lecturerId + "," + prereqCode;
    }

    static CourseRecord deserialize(const std::string& line) {
        std::stringstream ss(line);
        std::string type, code, title, capStr, lecId, prereq;
        if (!std::getline(ss, type, ',') || !std::getline(ss, code, ',') ||
            !std::getline(ss, title, ',') || !std::getline(ss, capStr, ',') ||
            !std::getline(ss, lecId, ',') || !std::getline(ss, prereq, ',')) {
            throw std::runtime_error("Malformed course record: " + line);
        }
        return CourseRecord(type, code, title, std::stoi(capStr), lecId, prereq);
    }
};

#endif // COURSE_RECORD_H