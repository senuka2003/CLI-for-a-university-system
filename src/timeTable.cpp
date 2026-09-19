#include "Timetable.h"

using namespace std;

void Timetable::addSlot(const TimeSlot& slot) {
    slots.push_back(slot);
}

const std::vector<TimeSlot>& Timetable::getSlots() const {
    return slots;
}

bool Timetable::hasClash(const TimeSlot& slot) const {
    for (const auto& existing : slots) {
        if (existing == slot) return true;
    }
    return false;
}

ostream& operator<<(ostream& os, const Timetable& t) {
    os << "Weekly Timetable:\n";
    for (const auto& slot : t.slots) {
        os << "  " << slot << "\n";
    }
    return os;
}