#include "Lecturer.h"
#include "Course.h"
#include "Student.h"
#include "Exceptions.h"
#include <iostream>

using namespace std;

Lecturer::Lecturer(string id, string name, string email, string password)
    : Person(std::move(id), std::move(name), std::move(email), std::move(password)) {}

void Lecturer::displayMenu() const {
    cout << "--- Lecturer Menu ---\n"
              << "1. View enrolment list\n2. Open attendance session\n";
}

string Lecturer::getRole() const { return "Lecturer"; }

void Lecturer::viewEnrolmentList(Course* course) const {
    // ownership check - "their own courses only".
    if (course->getLecturer() != this) {
        throw UnauthorizedAccessException("not your course: " + course->getCode());
    }
    for (Student* s : course->getEnrolledStudents()) {
        cout << *s << "\n"; // reuses Student::operator<<
    }
}

AttendanceSession* Lecturer::openAttendanceSession(Course* course, const TimeSlot& slot, int duration) {
    //"their own" course time slot only.
    if (course->getLecturer() != this) {
        throw UnauthorizedAccessException("not your course: " + course->getCode());
    }
    static int sessionCounter = 0;
    string sessionId = course->getCode() + "-S" + to_string(++sessionCounter);
    return course->openAttendanceSession(sessionId, duration);
}
