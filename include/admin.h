#ifndef ADMIN_H
#define ADMIN_H

#include "Person.h"
#include <vector>
#include <string>

class Course;
class Student;
class Lecturer;

class Admin : public Person {
public:
    Admin(std::string id, std::string name, std::string email, std::string password);
    void displayMenu() const override;
    std::string getRole() const override;

    void manageUsers(std::vector<Student*>& students, 
                     std::vector<Lecturer*>& lecturers, 
                     std::vector<Admin*>& admins) const;
                     
    void manageCourses(std::vector<Course*>& allCourses, 
                       const std::vector<Lecturer*>& lecturers) const;
                       
    void generateReport(const std::vector<Course*>& allCourses) const;
};

#endif // ADMIN_H