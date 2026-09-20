#ifndef TIMESLOT_H
#define TIMESLOT_H

#include <string>
#include <iostream>

class TimeSlot {
private:
    std::string day;
    std::string startTime;
    std::string endTime;
    std::string location;

public:
    TimeSlot() = default;
    TimeSlot(const std::string& d, const std::string& start, const std::string& end, const std::string& loc);

    std::string getDay() const;
    std::string getStartTime() const;
    std::string getEndTime() const;
    std::string getLocation() const;

    bool operator==(const TimeSlot& other) const;

    friend std::ostream& operator<<(std::ostream& os, const TimeSlot& slot);
};

#endif