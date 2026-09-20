#ifndef STUDENT_H
#define STUDENT_H

#include "repository.h"
#include "exceptions.h"
#include "Person.h"
#include "Timetable.h"
#include <vector>
#include <iostream>
#include <set>
#include <string>

class Course;

class Student : public Person {
private:
    std::vector<Course*> enrolledCourses;

    bool checkChain(Course* c, std::set<Course*>& visited) const;
    bool hasCompleted(Course* target) const;
    bool prerequisitesSatisfied(Course* course) const;

public:
    Student(std::string id, std::string name, std::string email, std::string password);

    void displayMenu() const override;
    std::string getRole() const override;

    void enrol(Course& course);
    void drop(Course& course);

    const std::vector<Course*>& getEnrolledCourses() const;
    
    Timetable viewTimetable() const;

    friend std::ostream& operator<<(std::ostream& os, const Student& s);
};

#endif // STUDENT_H