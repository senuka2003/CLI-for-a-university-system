#include "AttendanceRecord.h"

using namespace std;

namespace {
string currentTimestamp() {
    time_t now = time(nullptr);
    string s = ctime(&now);
    if (!s.empty() && s.back() == '\n') s.pop_back();
    return s;
}
}

AttendanceRecord::AttendanceRecord(Student* student, AttendanceSession* session,
                                    string status, string capturedVia)
    : student(student), session(session), timestamp(currentTimestamp()),
      status(move(status)), capturedVia(move(capturedVia)),
      isCorrection(false), actingLecturer(nullptr), reason("") {}

AttendanceRecord::AttendanceRecord(Student* student, AttendanceSession* session,
                                    string status, Lecturer* actingLecturer,
                                    string reason)
    : student(student), session(session), timestamp(currentTimestamp()),
      status(move(status)), capturedVia("correction"),
      isCorrection(true), actingLecturer(actingLecturer), reason(move(reason)) {}

Student* AttendanceRecord::getStudent() const { return student; }
AttendanceSession* AttendanceRecord::getSession() const { return session; }
string AttendanceRecord::getTimestamp() const { return timestamp; }
string AttendanceRecord::getStatus() const { return status; }
string AttendanceRecord::getCapturedVia() const { return capturedVia; }
bool AttendanceRecord::getIsCorrection() const { return isCorrection; }
Lecturer* AttendanceRecord::getActingLecturer() const { return actingLecturer; }
string AttendanceRecord::getReason() const { return reason; }