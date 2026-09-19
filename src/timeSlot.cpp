#include "TimeSlot.h"

using namespace std;

TimeSlot::TimeSlot(string day, int startTime, int endTime, string location)
    : day(move(day)), startTime(startTime), endTime(endTime), location(move(location)) {}

string TimeSlot::getDay() const { return day; }
int TimeSlot::getStartTime() const { return startTime; }
int TimeSlot::getEndTime() const { return endTime; }
string TimeSlot::getLocation() const { return location; }

bool TimeSlot::operator==(const TimeSlot& other) const {
    if (day != other.day) return false;
    return (startTime < other.endTime) && (other.startTime < endTime);
}

ostream& operator<<(ostream& os, const TimeSlot& slot) {
    os << slot.day << " " << slot.startTime << "-" << slot.endTime
       << " @ " << slot.location;
    return os;
}