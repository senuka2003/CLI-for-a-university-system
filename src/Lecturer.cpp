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
    if (!course) return;

    if (!course->getLecturer() || course->getLecturer()->getId() != this->getId()) {
        throw DomainException("Access denied: not your course: " + course->getCode());
    }

    std::cout << "\nEnrolled students in " << course->getCode() << ":\n";
    const auto& enrolled = course->getEnrolledStudents();
    if (enrolled.empty()) {
        std::cout << "  (No students currently enrolled in this course)\n";
        return;
    }

    for (const auto* s : enrolled) {
        std::cout << "  - [" << s->getId() << "] " << s->getName() << " (" << s->getEmail() << ")\n";
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
