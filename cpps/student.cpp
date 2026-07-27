#include "Student.h"
#include "Course.h"
#include "Exceptions.h"
#include <algorithm>

using namespace std;

Student::Student(string id, string name, string email, string password)
    : Person(move(id), move(name), move(email), move(password)) {}

void Student::displayMenu() const {
    cout << "--- Student Menu ---\n"
              << "1. Enrol in course\n2. Drop course\n3. View timetable\n";
}

string Student::getRole() const { return "Student"; }

bool Student::hasCompleted(Course* target) const {
    return find(enrolledCourses.begin(), enrolledCourses.end(), target) != enrolledCourses.end();
}

bool Student::prerequisitesSatisfied(Course* course) const {
    for (Course* prereq : course->getPrerequisites()) {
        if (!hasCompleted(prereq)) return false;
        if (!prerequisitesSatisfied(prereq)) return false; // walk the whole chain
    }
    return true;
}

void Student::enrol(Course& course) {
    if (course.isFull()) {
        throw CourseFullException(course.getCode());
    }
    if (!prerequisitesSatisfied(&course)) {
        throw PrerequisiteNotMetException(course.getCode());
    }
    // reject if any of the course's slots clash with our existing timetable.
    Timetable myTimetable = viewTimetable();
    for (const TimeSlot& slot : course.getTimetable().getSlots()) {
        if (myTimetable.hasClash(slot)) {
            throw TimetableClashException(course.getCode());
        }
    }
    enrolledCourses.push_back(&course);
    course.addEnrolledStudent(this);
}

void Student::drop(Course& course) {
    auto it = find(enrolledCourses.begin(), enrolledCourses.end(), &course);
    if (it == enrolledCourses.end()) {
        throw NotEnrolledException(id, course.getCode());
    }
    enrolledCourses.erase(it);
    course.removeEnrolledStudent(this);
}

const vector<Course*>& Student::getEnrolledCourses() const {
    return enrolledCourses;
}

Timetable Student::viewTimetable() const {
    Timetable combined;
    for (Course* c : enrolledCourses) {
        for (const TimeSlot& slot : c->getTimetable().getSlots()) {
            combined.addSlot(slot);
        }
    }
    return combined;
}

ostream& operator<<(std::ostream& os, const Student& s) {
    os << "Student[" << s.id << "] " << s.name
       << " - enrolled in " << s.enrolledCourses.size() << " course(s)";
    return os;
}


