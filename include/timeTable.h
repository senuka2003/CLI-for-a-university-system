#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "timeSlot.h"
#include <vector>
#include <iostream>

class Timetable {
private:
    std::vector<TimeSlot> slots;

public:
    Timetable() = default;

    void addSlot(const TimeSlot& slot);
    
    std::vector<TimeSlot>& getSlots() { return slots; }
    
    const std::vector<TimeSlot>& getSlots() const { return slots; }

    void removeSlot(size_t index) {
        if (index < slots.size()) {
            slots.erase(slots.begin() + index);
        }
    }

    void updateSlot(size_t index, const TimeSlot& newSlot) {
        if (index < slots.size()) {
            slots[index] = newSlot;
        }
    }

    bool hasClash(const TimeSlot& newSlot) const;

    friend std::ostream& operator<<(std::ostream& os, const Timetable& t);
};

#endif