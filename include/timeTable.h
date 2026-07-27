#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "TimeSlot.h"
#include <vector>
#include <iostream>

class Timetable {
private:
    std::vector<TimeSlot> slots;

public:
    void addSlot(const TimeSlot& slot);
    const std::vector<TimeSlot>& getSlots() const;

    bool hasClash(const TimeSlot& slot) const;

    friend std::ostream& operator<<(std::ostream& os, const Timetable& t);
};

#endif // TIMETABLE_H