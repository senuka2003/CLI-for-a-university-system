#include "Timetable.h"

void Timetable::addSlot(const TimeSlot& slot) {
    slots.push_back(slot);
}



bool Timetable::hasClash(const TimeSlot& slot) const {
    for (const auto& s : slots) {
        if (s == slot) {
            return true;
        }
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const Timetable& t) {
    if (t.slots.empty()) {
        os << "No time slots scheduled.";
        return os;
    }
    os << "Weekly Timetable:\n";
    for (const auto& slot : t.slots) {
        os << "  " << slot << "\n";
    }
    return os;
}