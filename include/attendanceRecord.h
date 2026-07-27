#ifndef ATTENDANCE_RECORD_H
#define ATTENDANCE_RECORD_H

#include <string>
#include <ctime>

class Student;
class AttendanceSession;
class Lecturer;

class AttendanceRecord {
private:
    Student* student;
    AttendanceSession* session;
    std::string timestamp;
    std::string status;
    std::string capturedVia;
    bool isCorrection;
    Lecturer* actingLecturer;
    std::string reason;

public:
    AttendanceRecord(Student* student, AttendanceSession* session,
                      std::string status, std::string capturedVia);

    AttendanceRecord(Student* student, AttendanceSession* session,
                      std::string status, Lecturer* actingLecturer,
                      std::string reason);

    Student* getStudent() const;
    AttendanceSession* getSession() const;
    std::string getTimestamp() const;
    std::string getStatus() const;
    std::string getCapturedVia() const;
    bool getIsCorrection() const;
    Lecturer* getActingLecturer() const;
    std::string getReason() const;
};

#endif // ATTENDANCE_RECORD_H