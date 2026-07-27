#ifndef LAB_COURSE_H
#define LAB_COURSE_H

#include "Course.h"

class LabCourse : public Course {
public:
    using Course::Course;
    int getCredits() const override;
    double calculateGrade(double rawScore) const override;
};

#endif // LAB_COURSE_H