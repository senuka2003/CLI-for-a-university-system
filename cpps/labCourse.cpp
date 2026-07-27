#include "LabCourse.h"

using namespace std;

int LabCourse::getCredits() const { return 2; }

double LabCourse::calculateGrade(double rawScore) const {
    return rawScore > 90.0 ? 90.0 : rawScore;
}