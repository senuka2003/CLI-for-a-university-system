#include "Admin.h"
#include "Course.h"
#include <iostream>

using namespace std;

Admin::Admin(string id, string name, string email, string password)
    : Person(move(id), move(name), move(email), move(password)) {}

void Admin::displayMenu() const {
    cout << "--- Administrator Menu ---\n"
              << "1. Manage users\n2. Manage courses\n3. Generate report\n";
}

string Admin::getRole() const { return "Administrator"; }

void Admin::manageUsers() const {
    cout << "(user management not shown in this excerpt)\n";
}

void Admin::manageCourses() const {
    cout << "(course management not shown in this excerpt)\n";
}

void Admin::generateReport(const vector<Course*>& allCourses) const {
    cout << "=== Enrolment Summary Report ===\n";
    for (Course* course : allCourses) {
        cout << *course << "\n"                       // operator<<
                  << "  Course attendance avg: "
                  << course->getCourseAttendancePercentage() << "%\n";
    }
}