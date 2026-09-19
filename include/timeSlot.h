#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <string>
#include <iostream>

class TimeSlot {
private:
    std::string day;
    int startTime; // minutes since midnight, e.g. 9:00 -> 540
    int endTime;
    std::string location;

public:
    TimeSlot(std::string day, int startTime, int endTime, std::string location);

    std::string getDay() const;
    int getStartTime() const;
    int getEndTime() const;
    std::string getLocation() const;

bool operator==(const TimeSlot& other) const;

friend std::ostream& operator<<(std::ostream& os, const TimeSlot& slot);
};

#endif // TIMESLOT_H