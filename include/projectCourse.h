#ifndef PROJECT_COURSE_H
#define PROJECT_COURSE_H

#include "Course.h"

class ProjectCourse : public Course {
public:
    using Course::Course;
    int getCredits() const override;
    double calculateGrade(double rawScore) const override;
};

#endif // PROJECT_COURSE_H