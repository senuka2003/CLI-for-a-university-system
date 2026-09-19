#include "AttendanceSession.h"
#include "Course.h"
#include "Student.h"
#include "Exceptions.h"
#include <algorithm>

using namespace std;

AttendanceSession::AttendanceSession(string sessionId, string courseCode, int duration)
    : sessionId(move(sessionId)), courseCode(move(courseCode)),
      open(true), duration(duration), openTime(time(nullptr)), capture(nullptr) {}

void AttendanceSession::close() {
    open = false;
}

bool AttendanceSession::isExpired() const {
    double elapsedMinutes = difftime(time(nullptr), openTime) / 60.0;
    return elapsedMinutes >= duration;
}

bool AttendanceSession::isOpen() const {
    return open && !isExpired();
}

void AttendanceSession::setCaptureMechanism(AttendanceCapture* c) {
    capture = c;
}

void AttendanceSession::markPresent(Student* student, Course* course, const string& capturedVia) {
    // Condition 1: session must be open.
    if (!isOpen()) {
        throw SessionClosedException(sessionId);
    }

    // Condition 2: student must be enrolled in this course.
    const auto& enrolled = course->getEnrolledStudents();
    bool isEnrolled = find(enrolled.begin(), enrolled.end(), student) != enrolled.end();
    if (!isEnrolled) {
        throw NotEnrolledException(student->getId(), course->getCode());
    }

    // Condition 3: student must not already be marked in this session.
    for (const auto& record : records) {
        if (record.getStudent() == student && !record.getIsCorrection()) {
            throw DuplicateAttendanceException(student->getId(), sessionId);
        }
    }

    records.emplace_back(student, this, "present", capturedVia);
}

void AttendanceSession::addCorrection(Student* student, const string& newStatus,
                                      Lecturer* actingLecturer, const string& reason) {
    // append, never edit or delete the original record.
    records.emplace_back(student, this, newStatus, actingLecturer, reason);
}

const vector<AttendanceRecord>& AttendanceSession::getRecords() const {
    return records;
}

string AttendanceSession::getSessionId() const { return sessionId; }
string AttendanceSession::getCourseCode() const { return courseCode; }