#ifndef LECTURE_COURSE_H
#define LECTURE_COURSE_H

#include "Course.h"

class LectureCourse : public Course {
public:
    using Course::Course;
    int getCredits() const override;
    double calculateGrade(double rawScore) const override;
};

#endif // LECTURE_COURSE_H