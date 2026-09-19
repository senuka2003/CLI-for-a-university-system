#include "LectureCourse.h"

using namespace std;

int LectureCourse::getCredits() const { return 3; }

double LectureCourse::calculateGrade(double rawScore) const {
    return rawScore;
}