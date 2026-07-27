#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <string>
#include <sstream>
#include <stdexcept>

class LogEntry {
private:
    std::string studentId;
    std::string courseCode;
    std::string action; 

public:
    LogEntry() = default;
    LogEntry(std::string studentId, std::string courseCode, std::string action)
        : studentId(std::move(studentId)), courseCode(std::move(courseCode)), action(std::move(action)) {}

    std::string serialize() const {
        return studentId + "," + courseCode + "," + action;
    }

    static LogEntry deserialize(const std::string& line) {
        std::stringstream ss(line);
        std::string sid, code, act;
        if (!std::getline(ss, sid, ',') || !std::getline(ss, code, ',') || !std::getline(ss, act, ',')) {
            throw std::runtime_error("malformed log entry: " + line);
        }
        return LogEntry(sid, code, act);
    }

    std::string toString() const {
        return studentId + " " + action + " " + courseCode;
    }
};

#endif // LOG_ENTRY_H