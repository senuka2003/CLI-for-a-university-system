#ifndef COURSE_H
#define COURSE_H

#include "Timetable.h"
#include <string>
#include <vector>
#include <iostream>

class Lecturer;
class Student;
class AttendanceSession;

class Course {
protected:
    std::string code;
    std::string title;
    int capacity;
    Lecturer* lecturer;
    Timetable timetable;
    std::vector<Course*> prerequisites;
    std::vector<Student*> enrolledStudents;
    std::vector<AttendanceSession*> sessions;

    static int totalCourseCount;

public:
    Course(std::string code, std::string title, int capacity, Lecturer* lecturer);

    Course(const Course& other);
    Course& operator=(const Course& other);
    virtual ~Course();

    virtual int getCredits() const = 0;
    virtual double calculateGrade(double rawScore) const = 0;

    bool isFull() const;
    void addPrerequisite(Course* c);
    const std::vector<Course*>& getPrerequisites() const;

    void addEnrolledStudent(Student* s);
    void removeEnrolledStudent(Student* s);
    const std::vector<Student*>& getEnrolledStudents() const;

    void addTimeSlot(const TimeSlot& slot);
    const Timetable& getTimetable() const;

    AttendanceSession* openAttendanceSession(const std::string& sessionId, int duration);
    const std::vector<AttendanceSession*>& getSessions() const;

    double getAttendancePercentage(Student* student) const;
    double getCourseAttendancePercentage() const;

    std::string getCode() const;
    std::string getTitle() const;
    Lecturer* getLecturer() const;

    static int getTotalCourseCount();

    friend std::ostream& operator<<(std::ostream& os, const Course& c);
};

#endif // COURSE_H