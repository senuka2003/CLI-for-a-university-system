#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#pragma once
#include <exception>
#include <string>

class DomainException : public std::exception {
protected:
    std::string message;
public:
    explicit DomainException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
    virtual ~DomainException() = default;
};

class EnrolmentException : public DomainException {
public:
    explicit EnrolmentException(const std::string& msg) : DomainException(msg) {}
};

class PrerequisiteNotMetException : public EnrolmentException {
public:
    explicit PrerequisiteNotMetException(const std::string& courseCode)
        : EnrolmentException("Prerequisite not met for course: " + courseCode) {}
};

class CourseFullException : public EnrolmentException {
public:
    explicit CourseFullException(const std::string& courseCode)
        : EnrolmentException("Course is full: " + courseCode) {}
};

class TimetableClashException : public DomainException {
public:
    explicit TimetableClashException(const std::string& courseCode)
        : DomainException("Timetable clash detected for course: " + courseCode) {}
};

class AttendanceException : public DomainException {
public:
    explicit AttendanceException(const std::string& msg) : DomainException(msg) {}
};

class SessionClosedException : public AttendanceException {
public:
    explicit SessionClosedException(const std::string& sessionId)
        : AttendanceException("Attendance session is closed: " + sessionId) {}
};

class NotEnrolledException : public AttendanceException {
public:
    NotEnrolledException(const std::string& studentId, const std::string& courseCode)
        : AttendanceException("Student " + studentId + " is not enrolled in course " + courseCode) {}
};

class DuplicateAttendanceException : public AttendanceException {
public:
    DuplicateAttendanceException(const std::string& studentId, const std::string& sessionId)
        : AttendanceException("Student " + studentId + " already marked in session " + sessionId) {}
};

class DataCorruptionException : public DomainException {
public:
    DataCorruptionException(const std::string& filename, const std::string& reason)
        : DomainException("Data error in " + filename + ": " + reason) {}
};

class UnauthorizedAccessException : public DomainException {
public:
    explicit UnauthorizedAccessException(const std::string& msg)
        : DomainException("Access denied: " + msg) {}
};

#endif // EXCEPTIONS_H