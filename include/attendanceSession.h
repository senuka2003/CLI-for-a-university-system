#ifndef ATTENDANCE_SESSION_H
#define ATTENDANCE_SESSION_H

#include "AttendanceRecord.h"
#include "AttendanceCapture.h"
#include <string>
#include <vector>
#include <ctime>

class Student;
class Course;
class Lecturer;

class AttendanceSession {
private:
    std::string sessionId;
    std::string courseCode;
    bool open;
    int duration;
    time_t openTime;
    AttendanceCapture* capture;
    std::vector<AttendanceRecord> records;

public:
    AttendanceSession(std::string sessionId, std::string courseCode, int duration);

    void close();
    bool isExpired() const;
    bool isOpen() const;

    void setCaptureMechanism(AttendanceCapture* c);

    void markPresent(Student* student, Course* course, const std::string& capturedVia);

    void addCorrection(Student* student, const std::string& newStatus,
                        Lecturer* actingLecturer, const std::string& reason);

    const std::vector<AttendanceRecord>& getRecords() const;
    std::string getSessionId() const;
    std::string getCourseCode() const;
};

#endif // ATTENDANCE_SESSION_H



