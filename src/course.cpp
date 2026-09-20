#include "Course.h"
#include "Student.h"
#include "AttendanceSession.h"
#include <algorithm>

using namespace std;

int Course::totalCourseCount = 0;

Course::Course(string code, string title, int capacity, Lecturer* lecturer)
    : code(move(code)), title(std::move(title)), capacity(capacity), lecturer(lecturer) {
    ++totalCourseCount;
}

Course::Course(const Course& other)
    : code(other.code), title(other.title), capacity(other.capacity),
      lecturer(other.lecturer), timetable(other.timetable),
      prerequisites(other.prerequisites), enrolledStudents(other.enrolledStudents) {
    for (AttendanceSession* s : other.sessions) {
        sessions.push_back(new AttendanceSession(*s));
    }
    ++totalCourseCount;
}

Course& Course::operator=(const Course& other) {
    if (this == &other) return *this;

    for (AttendanceSession* s : sessions) delete s; // free existing before overwrite
    sessions.clear();

    code = other.code;
    title = other.title;
    capacity = other.capacity;
    lecturer = other.lecturer;
    timetable = other.timetable;
    prerequisites = other.prerequisites;
    enrolledStudents = other.enrolledStudents;

    for (AttendanceSession* s : other.sessions) {
        sessions.push_back(new AttendanceSession(*s));
    }
    return *this;
}

Course::Course(Course&& other) noexcept
    : code(std::move(other.code)),
      title(std::move(other.title)),
      capacity(other.capacity),
      lecturer(other.lecturer),
      timetable(std::move(other.timetable)),
      prerequisites(std::move(other.prerequisites)),
      enrolledStudents(std::move(other.enrolledStudents)),
      sessions(std::move(other.sessions)) {
    
    other.capacity = 0;
    other.lecturer = nullptr;
    other.sessions.clear();
    ++totalCourseCount;
}

Course& Course::operator=(Course&& other) noexcept {
    if (this == &other) return *this;

    for (AttendanceSession* s : sessions) {
        delete s;
    }
    sessions.clear();

    code = std::move(other.code);
    title = std::move(other.title);
    capacity = other.capacity;
    lecturer = other.lecturer;
    timetable = std::move(other.timetable);
    prerequisites = std::move(other.prerequisites);
    enrolledStudents = std::move(other.enrolledStudents);
    sessions = std::move(other.sessions);

    other.capacity = 0;
    other.lecturer = nullptr;
    other.sessions.clear();

    return *this;

}

Course::~Course() {
    for (AttendanceSession* s : sessions) delete s;
    --totalCourseCount;
}

bool Course::isFull() const {
    return static_cast<int>(enrolledStudents.size()) >= capacity;
}

void Course::addPrerequisite(Course* c) { prerequisites.push_back(c); }
const vector<Course*>& Course::getPrerequisites() const { return prerequisites; }

void Course::addEnrolledStudent(Student* s) { enrolledStudents.push_back(s); }

void Course::removeEnrolledStudent(Student* s) {
    enrolledStudents.erase(remove(enrolledStudents.begin(), enrolledStudents.end(), s),
                            enrolledStudents.end());
}

const vector<Student*>& Course::getEnrolledStudents() const { return enrolledStudents; }

void Course::addTimeSlot(const TimeSlot& slot) { timetable.addSlot(slot); }

AttendanceSession* Course::openAttendanceSession(const string& sessionId, int duration) {
    AttendanceSession* session = new AttendanceSession(sessionId, code, duration);
    sessions.push_back(session);
    return session;
}

const vector<AttendanceSession*>& Course::getSessions() const { return sessions; }

double Course::getAttendancePercentage(Student* student) const {
    int total = static_cast<int>(sessions.size());
    if (total == 0) return 0.0;

    int presentCount = 0;
    for (AttendanceSession* session : sessions) {
        bool present = false;
        bool found = false;
        for (const auto& record : session->getRecords()) {
            if (record.getStudent() == student) {
                present = (record.getStatus() == "present");
                found = true;
            }
        }
        if (found && present) presentCount++;
    }
    return (static_cast<double>(presentCount) / total) * 100.0;
}

double Course::getCourseAttendancePercentage() const {
    if (enrolledStudents.empty()) return 0.0;
    double sum = 0.0;
    for (Student* s : enrolledStudents) {
        sum += getAttendancePercentage(s);
    }
    return sum / enrolledStudents.size();
}

string Course::getCode() const { return code; }
string Course::getTitle() const { return title; }
Lecturer* Course::getLecturer() const { return lecturer; }
int Course::getTotalCourseCount() { return totalCourseCount; }

ostream& operator<<(std::ostream& os, const Course& c) {
    os << "[" << c.code << "] " << c.title
       << " (credits=" << c.getCredits()
       << ", capacity=" << c.capacity
       << ", enrolled=" << c.enrolledStudents.size() << ")";
    return os;
}