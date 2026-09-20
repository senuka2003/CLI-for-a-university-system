#ifndef ENROLMENT_RECORD_H
#define ENROLMENT_RECORD_H

#include <string>
#include <sstream>
#include <stdexcept>

class EnrolmentRecord {
public:
    std::string studentId;
    std::string courseCode;

    EnrolmentRecord() = default;
    EnrolmentRecord(std::string sid, std::string ccode)
        : studentId(std::move(sid)), courseCode(std::move(ccode)) {}

    std::string serialize() const {
        return studentId + "," + courseCode;
    }

    static EnrolmentRecord deserialize(const std::string& line) {
        std::stringstream ss(line);
        std::string sid, ccode;
        if (!std::getline(ss, sid, ',') || !std::getline(ss, ccode, ',')) {
            throw std::runtime_error("Malformed enrolment record: " + line);
        }
        return EnrolmentRecord(sid, ccode);
    }
};

#endif // ENROLMENT_RECORD_H