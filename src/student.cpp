#include "Student.h"
#include "Course.h"
#include "Exceptions.h"
#include "repository.h"
#include <algorithm>
#include <set>

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


bool Student::checkChain(Course* c, std::set<Course*>& visited) const {
    if (!c) return true;

    if (visited.find(c) != visited.end()) {
        return true;
    }
    visited.insert(c);

    for (Course* prereq : c->getPrerequisites()) {
        if (!hasCompleted(prereq)) {
            return false;
        }
        if (!checkChain(prereq, visited)) {
            return false;
        }
    }
    return true;
}

bool Student::prerequisitesSatisfied(Course* course) const {
    std::set<Course*> visited;
    return checkChain(course, visited);
}

void Student::enrol(Course& course) {
    for (Course* c : enrolledCourses) {
        if (c->getCode() == course.getCode()) {
            throw DomainException("Student (" + id + ") is already enrolled in course " + course.getCode());
        }
    }
    
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

    if (enrolledCourses.empty()) {
        return combined;
    }
    
    std::cout << "\n========================================\n";
    std::cout << " TIMETABLE FOR: " << name << " (" << id << ")\n";
    std::cout << "========================================\n";

    if (enrolledCourses.empty()) {
        std::cout << ">> You are not enrolled in any courses.\n";
        return combined;
    }

    bool hasSlots = false;
    for (Course* c : enrolledCourses) {
        for (const TimeSlot& slot : c->getTimetable().getSlots()) {
            combined.addSlot(slot);
            std::cout << "Course: " << c->getCode() 
                      << " | Day: " << slot.getDay() 
                      << " | Time: " << slot.getStartTime() << " - " << slot.getEndTime() << "\n";
            hasSlots = true;
        }
    }

    if (!hasSlots) {
        std::cout << ">> No scheduled timetable slots found for your enrolled courses.\n";
    }
    std::cout << "========================================\n";

    return combined;
}

ostream& operator<<(std::ostream& os, const Student& s) {
    os << "Student[" << s.id << "] " << s.name
       << " - enrolled in " << s.enrolledCourses.size() << " course(s)";
    return os;
}


