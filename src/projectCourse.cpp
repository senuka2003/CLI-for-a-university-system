#include "ProjectCourse.h"

using namespace std;

int ProjectCourse::getCredits() const { return 4; }

double ProjectCourse::calculateGrade(double rawScore) const {
    double result = rawScore + (rawScore * 0.05);
    return result > 100.0 ? 100.0 : result;
}