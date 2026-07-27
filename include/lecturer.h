#ifndef LECTURER_H
#define LECTURER_H

#include "Person.h"
#include "TimeSlot.h"
#include <string>

class Course;
class AttendanceSession;

class Lecturer : public Person {
public:
    Lecturer(std::string id, std::string name, std::string email, std::string password);

    void displayMenu() const override;
    std::string getRole() const override;

    void viewEnrolmentList(Course* course) const;

    AttendanceSession* openAttendanceSession(Course* course, const TimeSlot& slot, int duration);
};

#endif // LECTURER_H