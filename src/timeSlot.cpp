#include "timeSlot.h"

TimeSlot::TimeSlot(const std::string& d, const std::string& start, const std::string& end, const std::string& loc)
    : day(d), startTime(start), endTime(end), location(loc) {}

std::string TimeSlot::getDay() const {
    return day;
}

std::string TimeSlot::getStartTime() const {
    return startTime;
}

std::string TimeSlot::getEndTime() const {
    return endTime;
}

std::string TimeSlot::getLocation() const {
    return location;
}

bool TimeSlot::operator==(const TimeSlot& other) const {
    if (day != other.day) return false;
    return (startTime < other.endTime && other.startTime < endTime);
}

std::ostream& operator<<(std::ostream& os, const TimeSlot& slot) {
    os << slot.day << " " << slot.startTime << "-" << slot.endTime << " @ " << slot.location;
    return os;
}