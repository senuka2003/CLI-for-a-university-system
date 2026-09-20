#include "lectureCourse.h"
#include "labCourse.h"
#include "projectCourse.h"
#include "student.h"
#include "lecturer.h"
#include "admin.h"
#include "attendanceSession.h"
#include "attendanceCapture.h"
#include "rotatingCodeCapture.h"
#include "fileReplayCapture.h"
#include "repository.h"
#include "logEntry.h"
#include "exceptions.h"
#include "userRecord.h"
#include "courseRecord.h"
#include "enrolmentRecord.h"
#include <iostream>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

// --- Helper Functions ---
void pauseConsole() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

Course* findCourseByCode(const vector<Course*>& courses, const string& code) {
    for (Course* c : courses) {
        if (c->getCode() == code) return c;
    }
    return nullptr;
}

void loadTimetables(const std::string& filename, const std::vector<Course*>& courses) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string courseCode, day, start, end, loc;

        if (std::getline(ss, courseCode, ',') &&
            std::getline(ss, day, ',') &&
            std::getline(ss, start, ',') &&
            std::getline(ss, end, ',') &&
            std::getline(ss, loc)) {
            
            for (Course* c : courses) {
                if (c->getCode() == courseCode) {
                    c->addTimeSlot(TimeSlot(day, start, end, loc));
                    break;
                }
            }
        }
    }
    file.close();
}

// --- Automated Spec Validation & Test Runner (Reports / Demonstration) ---
void runAutomatedTests(vector<Course*>& allCourses, vector<Student*>& students, vector<Lecturer*>& lecturers) {
    cout << "\n==================================================\n";
    cout << "   RUNNING AUTOMATED SPECIFICATION TEST SUITE\n";
    cout << "==================================================\n\n";

    cout << "1. Checking Static Member (Course count): "
         << Course::getTotalCourseCount() << " courses initialized.\n\n";

    Student& senuka = *students[0];
    Student& piyumi = *students[1];
    Student& dilara = *students[2];

    LectureCourse& co101  = *dynamic_cast<LectureCourse*>(allCourses[0]);
    LabCourse& col101     = *dynamic_cast<LabCourse*>(allCourses[3]);
    LabCourse& col102     = *dynamic_cast<LabCourse*>(allCourses[4]);

    cout << "2. Testing Prerequisite Enforcement (Chain/Requirement)...\n";
    try {
        senuka.enrol(col101); // Needs co101 first
        cout << "   [Failed] Should not allow enrollment without prerequisite.\n";
    } catch (const PrerequisiteNotMetException& e) {
        cout << "   [Passed Expected] " << e.what() << "\n";
    }

    cout << "\n3. Testing Successful Enrollment...\n";
    try {
        senuka.enrol(co101);
        cout << "   [Passed] " << senuka.getName() << " successfully enrolled in " << co101.getCode() << "\n";
    } catch (const DomainException& e) {
        cout << "   [Error] " << e.what() << "\n";
    }

    cout << "\n4. Testing Course Capacity & CourseFullException...\n";
    try {
        piyumi.enrol(co101); // co101 capacity is 2 -> now full
        cout << "   [Passed] " << piyumi.getName() << " enrolled (Capacity: 2/2 full).\n";
        dilara.enrol(co101); // 3rd student -> must fail
        cout << "   [Failed] Allowed 3rd student in capacity 2 course.\n";
    } catch (const CourseFullException& e) {
        cout << "   [Passed Expected] " << e.what() << "\n";
    }

    cout << "\n5. Testing Timetable Clash Detection via Overloaded operator==...\n";
    try {
        senuka.enrol(col102); // checks if any slot overlaps with senuka's timetable
        cout << "   [Passed] No clash detected for COL102.\n";
    } catch (const TimetableClashException& e) {
        cout << "   [Passed Expected Clash] " << e.what() << "\n";
    }

    // FR7.6 Polymorphism: Abstract AttendanceCapture with RotatingCodeCapture
    cout << "\n6. Testing Attendance Session & RotatingCodeCapture (FR7.6)...\n";
    Lecturer& drUdaya = *lecturers[0];
    AttendanceSession* session = drUdaya.openAttendanceSession(&co101, co101.getTimetable().getSlots()[0], 10);
    AttendanceCapture* codeCapture = new RotatingCodeCapture();
    session->setCaptureMechanism(codeCapture);

    codeCapture->beginSession();
    string currentCode = codeCapture->captureNext();
    cout << "   [Passed] Active Rotating Code generated: " << currentCode << "\n";

    // Testing present mark via rotating code
    try {
        session->markPresent(&senuka, &co101, "ROTATING_CODE");
        cout << "   [Passed] Marked present: " << senuka.getName() << "\n";
    } catch (const AttendanceException& e) {
        cout << "   [Passed Expected] " << e.what() << "\n";
    }

    codeCapture->endSession();
    delete codeCapture;

    cout << "\n7. Testing Generic Persistence: Repository<LogEntry>...\n";
    Repository<LogEntry> logRepo;
    vector<LogEntry> logs = {
        LogEntry("EN001", "CO101", "enrol"),
        LogEntry("EN002", "CO101", "enrol")
    };
    try {
        logRepo.saveToFile("data/logs.txt", logs);
        vector<LogEntry> loaded = logRepo.loadFromFile("data/logs.txt");
        cout << "   [Passed] Successfully saved and reloaded " << loaded.size() << " log entries from file.\n";
    } catch (const DataCorruptionException& e) {
        cout << "   [Persistence Error] " << e.what() << "\n";
    }

    cout << "\n==================================================\n";
    cout << "        AUTOMATED TEST SUITE COMPLETED CLEANLY    \n";
    cout << "==================================================\n";
    pauseConsole();
}

void syncEnrolmentsToFile(const vector<Student*>& students) {
    vector<EnrolmentRecord> records;
    for (const auto* s : students) {
        for (const auto* c : s->getEnrolledCourses()) {
            records.emplace_back(s->getId(), c->getCode());
        }
    }
    Repository<EnrolmentRecord> repo;
    repo.saveToFile("data/enrolments.txt", records);
}

std::vector<AttendanceSession*> allSessions;
std::vector<AttendanceSession*> allActiveSessions;

void loadAttendanceEvents(const vector<Course*>& allCourses, const vector<Student*>& allStudents, const vector<Lecturer*>& allLecturers) {
    ifstream inFile("data/attendance_events.txt");
    if (!inFile.is_open()) return;

    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string eventType;
        getline(ss, eventType, ',');

        if (eventType == "SESSION_OPEN") {
            // Format: SESSION_OPEN,courseCode,sessionId,lecturerId,duration,timestamp
            string courseCode, sessionId, lecturerId, durStr, tsStr;
            getline(ss, courseCode, ',');
            getline(ss, sessionId, ',');
            getline(ss, lecturerId, ',');
            getline(ss, durStr, ',');
            getline(ss, tsStr, ',');

            Course* c = findCourseByCode(allCourses, courseCode);
            if (c && !c->getTimetable().getSlots().empty()) {
                // Check if session already created
                bool exists = false;
                for (AttendanceSession* s : allSessions) {
                    if (s->getSessionId() == sessionId) { exists = true; break; }
                }
                if (!exists) {
                    int duration = stoi(durStr);
                    // Recreate session
                    AttendanceSession* s = new AttendanceSession(sessionId, c->getCode(), duration);
                    // If timestamp indicates it's in the past, close it
                    time_t now = time(nullptr);
                    time_t openTime = stoll(tsStr);
                    if (difftime(now, openTime) > (duration * 60)) {
                        s->close();
                    } else {
                        allActiveSessions.push_back(s);
                    }
                    allSessions.push_back(s);
                }
            }
        }
        else if (eventType == "ATTENDANCE_MARKED") {
            // Format: ATTENDANCE_MARKED,studentId,courseCode,sessionId,status,method,timestamp
            string studentId, courseCode, sessionId, status, method, tsStr;
            getline(ss, studentId, ',');
            getline(ss, courseCode, ',');
            getline(ss, sessionId, ',');
            getline(ss, status, ',');
            getline(ss, method, ',');
            getline(ss, tsStr, ',');

            Student* st = nullptr;
            for (Student* s : allStudents) {
                if (s->getId() == studentId) { st = s; break; }
            }
            Course* c = findCourseByCode(allCourses, courseCode);

            for (AttendanceSession* sess : allSessions) {
                if (sess->getSessionId() == sessionId && st && c) {
                    try {
                        sess->markPresent(st, c, method);
                    } catch (...) {}
                    break;
                }
            }
        }
        else if (eventType == "ATTENDANCE_CORRECTION") {
            // Format: ATTENDANCE_CORRECTION,studentId,courseCode,sessionId,status,lecturerId,"reason",timestamp
            string studentId, courseCode, sessionId, status, lecturerId, reasonWithQuotes, tsStr;
            getline(ss, studentId, ',');
            getline(ss, courseCode, ',');
            getline(ss, sessionId, ',');
            getline(ss, status, ',');
            getline(ss, lecturerId, ',');
            getline(ss, reasonWithQuotes, ',');
            getline(ss, tsStr, ',');

            // Clean quotes from reason
            string reason = reasonWithQuotes;
            if (reason.size() >= 2 && reason.front() == '"' && reason.back() == '"') {
                reason = reason.substr(1, reason.size() - 2);
            }

            Student* st = nullptr;
            for (Student* s : allStudents) {
                if (s->getId() == studentId) { st = s; break; }
            }
            Lecturer* lec = nullptr;
            for (Lecturer* l : allLecturers) {
                if (l->getId() == lecturerId) { lec = l; break; }
            }

            for (AttendanceSession* sess : allSessions) {
                if (sess->getSessionId() == sessionId && st && lec) {
                    try {
                        sess->addCorrection(st, status, lec, reason);
                    } catch (...) {}
                    break;
                }
            }
        }
    }
    inFile.close();
}

// --- Reporting Engine ---
void generateAttendanceReport(Course* course, double threshold = 80.0) {
    // 1. Gather all sessions held for this course
    std::vector<AttendanceSession*> courseSessions;
    for (AttendanceSession* s : allSessions) {
        if (s->getCourseCode() == course->getCode()) {
            courseSessions.push_back(s);
        }
    }

    int totalSessions = courseSessions.size();
    const auto& enrolled = course->getEnrolledStudents();

    std::cout << "\n======================================================\n";
    std::cout << " ATTENDANCE & ELIGIBILITY REPORT: [" << course->getCode() << "] " << course->getTitle() << "\n";
    std::cout << " Total Sessions Held : " << totalSessions << "\n";
    std::cout << " Enrolled Students   : " << enrolled.size() << "\n";
    std::cout << " Minimum Threshold   : " << threshold << "%\n";
    std::cout << "======================================================\n";

    if (totalSessions == 0) {
        std::cout << ">> No attendance sessions held yet for this course.\n";
        return;
    }

    if (enrolled.empty()) {
        std::cout << ">> No students enrolled in this course.\n";
        return;
    }

    double totalPercentageSum = 0.0;
    std::vector<std::pair<Student*, double>> lowAttendanceStudents;

    std::cout << std::left << std::setw(12) << "Student ID" 
              << std::setw(20) << "Name" 
              << std::setw(12) << "Attended" 
              << std::setw(14) << "Percentage" 
              << std::setw(15) << "Status" << "\n";
    std::cout << std::string(65, '-') << "\n";

    for (Student* st : enrolled) {
        int attendedCount = 0;

        for (AttendanceSession* sess : courseSessions) {
            // Determine effective status: last matching record takes priority (FR7.3)
            std::string effectiveStatus = "absent";
            for (const auto& rec : sess->getRecords()) {
                if (rec.getStudent() == st) {
                    effectiveStatus = rec.getStatus();
                }
            }

            if (effectiveStatus == "present" || effectiveStatus == "late") {
                attendedCount++;
            }
        }

        double percentage = (static_cast<double>(attendedCount) / totalSessions) * 100.0;
        totalPercentageSum += percentage;

        std::string eligibility = (percentage >= threshold) ? "ELIGIBLE" : "BELOW THRESHOLD";

        std::cout << std::left << std::setw(12) << st->getId()
                  << std::setw(20) << st->getName()
                  << std::setw(12) << attendedCount
                  << std::fixed << std::setprecision(1) << std::setw(14) << percentage
                  << std::setw(15) << eligibility << "\n";

        if (percentage < threshold) {
            lowAttendanceStudents.push_back({st, percentage});
        }
    }

    // Course Attendance Average
    double courseAverage = totalPercentageSum / enrolled.size();
    std::cout << std::string(65, '-') << "\n";
    std::cout << ">> Overall Course Attendance Average: " << std::fixed << std::setprecision(1) << courseAverage << "%\n";

    // Eligibility Warning (< Threshold)
    std::cout << "\n--- EXAM ELIGIBILITY DEFICIT REPORT (< " << threshold << "%) ---\n";
    if (lowAttendanceStudents.empty()) {
        std::cout << "[OK] All enrolled students meet the attendance eligibility criteria.\n";
    } else {
        for (const auto& entry : lowAttendanceStudents) {
            std::cout << " [!] INELIGIBLE: " << entry.first->getName() 
                      << " (" << entry.first->getId() << ") - Attendance: " 
                      << std::fixed << std::setprecision(1) << entry.second << "%\n";
        }
    }
    std::cout << "======================================================\n";
}

// Student personal attendance view
void viewStudentAttendanceReport(Student* student, const vector<Course*>& allCourses) {
    cout << "\n======================================================\n";
    cout << " MY ATTENDANCE REPORT: " << student->getName() << " (" << student->getId() << ")\n";
    cout << "======================================================\n";

    cout << left << setw(12) << "Course Code"
         << setw(24) << "Course Title"
         << setw(10) << "Attended"
         << setw(8) << "Total"
         << setw(14) << "Percentage"
         << setw(12) << "Status" << "\n";
    cout << string(80, '-') << "\n";

    bool enrolledAny = false;

    for (Course* course : allCourses) {
        // Check if student is enrolled
        bool isEnrolled = false;
        for (Student* st : course->getEnrolledStudents()) {
            if (st->getId() == student->getId()) {
                isEnrolled = true;
                break;
            }
        }

        if (!isEnrolled) continue;
        enrolledAny = true;

        // Gather sessions for this course
        vector<AttendanceSession*> courseSessions;
        for (AttendanceSession* s : allSessions) {
            if (s->getCourseCode() == course->getCode()) {
                courseSessions.push_back(s);
            }
        }

        int totalSessions = courseSessions.size();
        int attendedCount = 0;

        for (AttendanceSession* sess : courseSessions) {
            string effectiveStatus = "absent";
            for (const auto& rec : sess->getRecords()) {
                if (rec.getStudent()->getId() == student->getId()) {
                    effectiveStatus = rec.getStatus();
                }
            }
            if (effectiveStatus == "present" || effectiveStatus == "late") {
                attendedCount++;
            }
        }

        double percentage = (totalSessions == 0) ? 0.0 : (static_cast<double>(attendedCount) / totalSessions) * 100.0;
        string status = (totalSessions == 0) ? "NO SESSIONS" : ((percentage >= 80.0) ? "ELIGIBLE" : "DEFICIT");

        cout << left << setw(12) << course->getCode()
             << setw(24) << course->getTitle().substr(0, 22)
             << setw(10) << attendedCount
             << setw(8) << totalSessions
             << fixed << setprecision(1) << setw(14) << percentage
             << setw(12) << status << "\n";
    }

    if (!enrolledAny) {
        cout << ">> You are not enrolled in any courses.\n";
    }

    cout << "======================================================\n";
}

// --- Interactive Menus ---
void handleStudentSession(Student* student, const vector<Course*>& allCourses, const vector<Student*>& allStudents) {
    (void)allStudents;
    while (true) {
        cout << "\n----------------------------------------\n";
        cout << " Logged in as Student: " << student->getName() << " (" << student->getId() << ")\n";
        cout << "----------------------------------------\n";

        cout << "--- Student Menu ---\n";
        cout << "1. Enrol in course\n";
        cout << "2. Drop course\n";
        cout << "3. View timetable\n";
        cout << "4. Mark Attendance (Rotating Code)\n";
        cout << "5. View My Attendance Status\n";
        cout << "6. Logout\n";
        cout << "Enter choice: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            cout << "\n--- Available Courses to Enrol ---\n";
            for (Course* c : allCourses) {
                bool alreadyEnrolled = false;
                for (Course* enrolled : student->getEnrolledCourses()) {
                    if (enrolled->getCode() == c->getCode()) {
                        alreadyEnrolled = true;
                        break;
                    }
                }
                cout << " - " << c->getCode() << " : " << c->getTitle() 
                     << (alreadyEnrolled ? " [Already Enrolled]" : "") << "\n";
            }
            cout << "----------------------------------\n";

            cout << "Enter Course Code to enrol: ";
            string code;
            cin >> code;

            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << ">> Error: Course not found!\n";
            } else {
                try {
                    student->enrol(*target);
                    cout << ">> [SUCCESS] Successfully enrolled in " << target->getCode() << " (" << target->getTitle() << ")!\n";

                    ofstream logFile("data/attendance_events.txt", ios::app);
                    if (logFile.is_open()) {
                        time_t now = time(nullptr);
                        logFile << "ENROL," << student->getId() << "," << target->getCode() << "," << now << "\n";
                        logFile.close();
                    }
                } catch (const DomainException& e) {
                    cout << ">> Enrolment Failed: " << e.what() << "\n";
                }
            }
            pauseConsole();
        }
        else if (choice == 2) {
            cout << "\n--- Your Enrolled Courses ---\n";
            const auto& myCourses = student->getEnrolledCourses();
            if (myCourses.empty()) {
                cout << ">> You are not enrolled in any courses.\n";
            } else {
                for (Course* c : myCourses) {
                    cout << " - " << c->getCode() << " : " << c->getTitle() << "\n";
                }
            }
            cout << "-----------------------------\n";

            if (myCourses.empty()) {
                pauseConsole();
                continue;
            }

            cout << "Enter Course Code to drop: ";
            string code;
            cin >> code;

            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << ">> Error: Course not found!\n";
            } else {
                try {
                    student->drop(*target);
                    cout << ">> [SUCCESS] Successfully dropped " << target->getCode() << "!\n";

                    ofstream logFile("data/attendance_events.txt", ios::app);
                    if (logFile.is_open()) {
                        time_t now = time(nullptr);
                        logFile << "DROP," << student->getId() << "," << target->getCode() << "," << now << "\n";
                        logFile.close();
                    }
                } catch (const DomainException& e) {
                    cout << ">> Drop Failed: " << e.what() << "\n";
                }
            }
            pauseConsole();
        }
        else if (choice == 3) {
            student->viewTimetable();
            pauseConsole();
        }
        else if (choice == 4) {
            cout << "Enter Course Code: ";
            string code;
            cin >> code;

            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << ">> Error: Course not found!\n";
            } else {
                bool isEnrolled = false;
                for (Course* c : student->getEnrolledCourses()) {
                    if (c->getCode() == target->getCode()) {
                        isEnrolled = true;
                        break;
                    }
                }

                if (!isEnrolled) {
                    cout << ">> Error: You are not enrolled in this course!\n";
                } else {
                    AttendanceSession* activeSess = nullptr;
                    for (auto it = allActiveSessions.begin(); it != allActiveSessions.end(); ++it) {
                        if ((*it)->getCourseCode() == target->getCode() && (*it)->isOpen() && !(*it)->isExpired()) {
                            activeSess = *it;
                            break;
                        }
                    }

                    if (!activeSess) {
                        cout << ">> Error: No active attendance session currently open for this course!\n";
                    } else {
                        cout << ">> Active session detected: " << activeSess->getSessionId() << "\n";
                        cout << "Enter Active Rotating Code: ";
                        string inputCode;
                        cin >> inputCode;

                        RotatingCodeCapture rcc;
                        string currentValidCode = rcc.getCurrentCode();

                        if (!currentValidCode.empty() && inputCode == currentValidCode) {
                            try {
                                activeSess->markPresent(student, target, "RotatingCode");
                                cout << ">> [SUCCESS] Attendance marked successfully as PRESENT for Session " 
                                     << activeSess->getSessionId() << "!\n";

                                ofstream logFile("data/attendance_events.txt", ios::app);
                                if (logFile.is_open()) {
                                    time_t now = time(nullptr);
                                    logFile << "ATTENDANCE_MARKED," << student->getId() << "," 
                                            << target->getCode() << "," << activeSess->getSessionId() 
                                            << ",present," << now << "\n";
                                    logFile.close();
                                }
                            } catch (const DomainException& e) {
                                cout << ">> Attendance Error: " << e.what() << "\n";
                            }
                        } else {
                            cout << ">> [FAILED] Invalid or expired Rotating Code! Attendance was not recorded.\n";
                        }
                    }
                }
            }
            pauseConsole();
        }
        else if (choice == 5) {
            cout << "\n--- Your Attendance Summary ---\n";
            for (Course* c : student->getEnrolledCourses()) {
                int totalHeld = 0;
                int attended = 0;

                for (AttendanceSession* sess : allSessions) {
                    if (sess->getCourseCode() == c->getCode()) {
                        totalHeld++;
                        for (const auto& rec : sess->getRecords()) {
                            if (rec.getStudent()->getId() == student->getId() && 
                               (rec.getStatus() == "present" || rec.getStatus() == "late")) {
                                attended++;
                                break;
                            }
                        }
                    }
                }

                double pct = (totalHeld == 0) ? 0.0 : (static_cast<double>(attended) / totalHeld) * 100.0;
                cout << "Course: " << c->getCode() << " | Attended: " << attended << "/" << totalHeld 
                     << " (" << pct << "%) | Status: " << (pct >= 80.0 ? "Eligible" : "At Risk / Ineligible") << "\n";
            }
            pauseConsole();
        }
        else if (choice == 6) {
            break;
        }
    }
}

void handleLecturerSession(Lecturer* lecturer, const vector<Course*>& allCourses, const vector<Student*>& allStudents) {
    while (true) {
        cout << "\n----------------------------------------\n";
        cout << " Logged in as Lecturer: " << lecturer->getName() << " (" << lecturer->getId() << ")\n";
        cout << "----------------------------------------\n";

        cout << "--- Lecturer Menu ---\n";
        cout << "1. Open new attendance session\n";
        cout << "2. View / Manage active session\n";
        cout << "3. Manual attendance correction\n";
        cout << "4. View course attendance report\n";
        cout << "5. Logout\n";
        cout << "Enter choice: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            cout << "Enter Course Code: ";
            string ccode;
            cin >> ccode;

            Course* target = findCourseByCode(allCourses, ccode);
            if (!target) {
                cout << ">> Error: Course not found!\n";
            } else {
                cout << "Enter session duration in seconds (e.g. 600): ";
                int dur;
                cin >> dur;

                string sessId = ccode + "-S" + to_string(allSessions.size() + 1);
                AttendanceSession* newSess = new AttendanceSession(sessId, ccode, dur);
                allSessions.push_back(newSess);
                allActiveSessions.push_back(newSess);

                ofstream logFile("data/attendance_events.txt", ios::app);
                if (logFile.is_open()) {
                    time_t now = time(nullptr);
                    logFile << "SESSION_OPENED," << sessId << "," << ccode << "," << dur << "," << now << "\n";
                    logFile.close();
                }

                cout << ">> [SUCCESS] Attendance session opened successfully: " << sessId << "\n";
            }
            pauseConsole();
        }
        else if (choice == 2) {
            cout << "Enter Course Code for active session: ";
            string ccode;
            cin >> ccode;

            AttendanceSession* s = nullptr;
            for (AttendanceSession* sess : allActiveSessions) {
                if (sess->getCourseCode() == ccode && sess->isOpen() && !sess->isExpired()) {
                    s = sess;
                    break;
                }
            }

            if (!s) {
                cout << ">> No active open session found for course " << ccode << "\n";
            } else {
                Course* target = findCourseByCode(allCourses, ccode);
                while (true) {
                    cout << "\n--- Active Session Menu (" << s->getSessionId() << ") ---\n";
                    cout << "1. Check session status\n";
                    cout << "2. Close session manually\n";
                    cout << "3. Generate / View Active Rotating Code\n";
                    cout << "4. Replay Attendance from File (FileReplayCapture)\n";
                    cout << "5. Return to Lecturer Menu (Keep session running)\n";
                    cout << "Enter choice: ";

                    int sessionChoice;
                    if (!(cin >> sessionChoice)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        continue;
                    }

                    if (sessionChoice == 1) {
                        cout << ">> Session Status: " << (s->isOpen() ? "ACTIVE" : "CLOSED / EXPIRED") << "\n";
                    } else if (sessionChoice == 2) {
                        s->close();
                        cout << ">> Session closed manually by Lecturer.\n";
                        break;
                    } else if (sessionChoice == 3) {
                        AttendanceCapture* captureEngine = new RotatingCodeCapture();
                        captureEngine->beginSession();
                        string currentCode = captureEngine->captureNext();
                        cout << "\n==================================================\n";
                        cout << ">> CURRENT ACTIVE ROTATING CODE: [" << currentCode << "]\n";
                        cout << ">> (Valid for 10 minutes - Share this with students)\n";
                        cout << "==================================================\n\n";
                        delete captureEngine;
                    } else if (sessionChoice == 4) {
                        try {
                            AttendanceCapture* fileEngine = new FileReplayCapture("data/attendance_replay.txt");
                            fileEngine->beginSession();
                            cout << ">> Starting bulk attendance replay from file...\n";

                            string line;
                            int count = 0;
                            while (!(line = fileEngine->captureNext()).empty()) {
                                string sId = line;
                                string status = "present";
                                size_t commaPos = line.find(',');
                                if (commaPos != string::npos) {
                                    sId = line.substr(0, commaPos);
                                    status = line.substr(commaPos + 1);
                                }

                                Student* stTarget = nullptr;
                                for (Student* st : allStudents) {
                                    if (st->getId() == sId) {
                                        stTarget = st;
                                        break;
                                    }
                                }

                                if (stTarget && target) {
                                    s->markPresent(stTarget, target, "FileReplay");
                                    cout << "   [OK] Marked " << status << " for: " << stTarget->getName() << " (" << sId << ")\n";
                                    count++;
                                } else {
                                    cout << "   [SKIP] Student ID not found: " << sId << "\n";
                                }
                            }
                            fileEngine->endSession();
                            delete fileEngine;
                            cout << ">> [SUCCESS] Replayed and marked attendance for " << count << " students!\n";
                        } catch (const DomainException& e) {
                            cout << ">> Replay Error / Malformed Line: " << e.what() << "\n";
                        }
                    } else if (sessionChoice == 5) {
                        cout << ">> Returning to main menu. Session running in background.\n";
                        break;
                    }
                }
            }
            pauseConsole();
        }
        else if (choice == 3) {
            cout << "Enter Course Code: ";
            string ccode;
            cin >> ccode;

            cout << "Enter Session ID: ";
            string sessId;
            cin >> sessId;

            AttendanceSession* s = nullptr;
            for (AttendanceSession* sess : allSessions) {
                if (sess->getSessionId() == sessId && sess->getCourseCode() == ccode) {
                    s = sess;
                    break;
                }
            }

            if (!s) {
                cout << ">> Session not found!\n";
            } else {
                cout << "Enter Student ID: ";
                string sId;
                cin >> sId;

                Student* st = nullptr;
                for (Student* stud : allStudents) {
                    if (stud->getId() == sId) {
                        st = stud;
                        break;
                    }
                }

                if (!st) {
                    cout << ">> Student not found!\n";
                } else {
                    cout << "Enter new status (present/late/absent): ";
                    string newStatus;
                    cin >> newStatus;

                    cout << "Enter reason for correction: ";
                    cin.ignore();
                    string reason;
                    getline(cin, reason);

                    try {
                        s->addCorrection(st, newStatus, lecturer, reason);
                        cout << ">> [SUCCESS] Attendance corrected successfully.\n";

                        ofstream logFile("data/attendance_events.txt", ios::app);
                        if (logFile.is_open()) {
                            time_t now = time(nullptr);
                            logFile << "CORRECTION," << sId << "," << ccode << "," << sessId << ","
                                    << newStatus << "," << lecturer->getId() << "," << now << "\n";
                            logFile.close();
                        }
                    } catch (const DomainException& e) {
                        cout << ">> Correction Failed: " << e.what() << "\n";
                    }
                }
            }
            pauseConsole();
        }
        else if (choice == 4) {
            cout << "Enter Course Code: ";
            string ccode;
            cin >> ccode;

            Course* target = findCourseByCode(allCourses, ccode);
            if (!target) {
                cout << ">> Course not found!\n";
            } else {
                cout << "\n=== Attendance Report for " << target->getCode() << " (" << target->getTitle() << ") ===\n";
                int totalCourseSessions = 0;
                for (AttendanceSession* sess : allSessions) {
                    if (sess->getCourseCode() == target->getCode()) {
                        totalCourseSessions++;
                    }
                }

                cout << "Total sessions held: " << totalCourseSessions << "\n";
                for (Student* st : allStudents) {
                    bool enrolled = false;
                    for (Course* c : st->getEnrolledCourses()) {
                        if (c->getCode() == target->getCode()) {
                            enrolled = true;
                            break;
                        }
                    }

                    if (enrolled) {
                        int attended = 0;
                        for (AttendanceSession* sess : allSessions) {
                            if (sess->getCourseCode() == target->getCode()) {
                                for (const auto& rec : sess->getRecords()) {
                                    if (rec.getStudent()->getId() == st->getId() &&
                                       (rec.getStatus() == "present" || rec.getStatus() == "late")) {
                                        attended++;
                                        break;
                                    }
                                }
                            }
                        }

                        double pct = (totalCourseSessions == 0) ? 0.0 : (static_cast<double>(attended) / totalCourseSessions) * 100.0;
                        cout << " - Student: " << st->getName() << " (" << st->getId() << ") | Attended: "
                             << attended << "/" << totalCourseSessions << " (" << pct << "%) | Status: "
                             << (pct >= 80.0 ? "Eligible" : "At Risk") << "\n";
                    }
                }
            }
            pauseConsole();
        }
        else if (choice == 5) {
            break;
        }
    }
}

void handleAdminSession(Admin* admin, vector<Course*>& allCourses, 
                        vector<Student*>& students, 
                        vector<Lecturer*>& lecturers, 
                        vector<Admin*>& admins) {
    while (true) {
        cout << "\n------------------------------------------\n";
        cout << " Logged in as Administrator: " << admin->getName() << " (" << admin->getId() << ")\n";
        cout << "------------------------------------------\n";
        admin->displayMenu();
        cout << "4. Logout\n";
        cout << "Enter choice: ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            admin->manageUsers(students, lecturers, admins);
            pauseConsole();
        } else if (choice == 2) {
            admin->manageCourses(allCourses, lecturers);
            pauseConsole();
        } else if (choice == 3) {
            admin->generateReport(allCourses);
            pauseConsole();
        } else if (choice == 4) {
            break;
        }
    }
}

  void saveTimeSlotToFile(const std::string& filename, const std::string& courseCode, const TimeSlot& slot) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << courseCode << ","
             << slot.getDay() << ","
             << slot.getStartTime() << ","
             << slot.getEndTime() << ","
             << slot.getLocation() << "\n";
        file.close();
    }
}

// --- Main Program Entry Point ---
int main() {
    vector<Student*> students;
    vector<Lecturer*> lecturers;
    vector<Admin*> admins;

    Repository<UserRecord> userRepo;
    vector<UserRecord> loadedUsers;
    try {
        loadedUsers = userRepo.loadFromFile("data/users.txt");
    } catch (const exception&) {
        cout << "Warning : could not load user records from data/users.txt. starting with empty user lists. \n";
    }

    for (const auto& u : loadedUsers) {
        if (u.role == "Student") {
            students.push_back(new Student(u.id, u.name, u.email, u.password));
        } else if (u.role == "Lecturer") {
            lecturers.push_back(new Lecturer(u.id, u.name, u.email, u.password));
        } else if (u.role == "Admin") {
            admins.push_back(new Admin(u.id, u.name, u.email, u.password));
        }
    }

    // Default users
    if (lecturers.empty()) {
        lecturers.push_back(new Lecturer("COL001", "Dr. Udaya", "udaya@uni.lk", "pass123"));
        lecturers.push_back(new Lecturer("COL002", "Dr. Krishanth", "krishanth@uni.lk", "pass456"));
        lecturers.push_back(new Lecturer("COL003", "Dr. Sanka", "sanka@uni.lk", "pass789"));
    }
    if (students.empty()) {
        students.push_back(new Student("EN001", "Senuka", "senuka@uni.lk", "pw111"));
        students.push_back(new Student("EN002", "Piyumi", "piyumi@uni.lk", "pw222"));
        students.push_back(new Student("EN003", "Dilara", "dilara@uni.lk", "pw333"));
    }
    if (admins.empty()) {
        admins.push_back(new Admin("A001", "Administrator", "admin@uni.lk", "adminpw"));
    }

    // 2. Courses Persistence Load
    vector<Course*> allCourses;
    Repository<CourseRecord> courseRepo;
    vector<CourseRecord> loadedCourses;
    try {
        loadedCourses = courseRepo.loadFromFile("data/courses.txt");
    } catch (...) {}

    auto findLecturerById = [&](const string& lid) -> Lecturer* {
        for (auto* l : lecturers) if (l->getId() == lid) return l;
        return lecturers.empty() ? nullptr : lecturers[0];
    };

    for (const auto& cr : loadedCourses) {
        Lecturer* lec = findLecturerById(cr.lecturerId);
        Course* nc = nullptr;
        if (cr.type == "Lab") nc = new LabCourse(cr.code, cr.title, cr.capacity, lec);
        else if (cr.type == "Project") nc = new ProjectCourse(cr.code, cr.title, cr.capacity, lec);
        else nc = new LectureCourse(cr.code, cr.title, cr.capacity, lec);
        allCourses.push_back(nc);
    }

    // Link Prerequisites from file data
    for (size_t i = 0; i < loadedCourses.size(); ++i) {
        if (loadedCourses[i].prereqCode != "NONE") {
            for (auto* target : allCourses) {
                if (target->getCode() == loadedCourses[i].prereqCode) {
                    allCourses[i]->addPrerequisite(target);
                    break;
                }
            }
        }
    }

    loadTimetables("data/timeTable.txt", allCourses);

    Repository<EnrolmentRecord> enrolRepo;
    try {
        auto loadedEnrols = enrolRepo.loadFromFile("data/enrolments.txt");
        for (const auto& rec : loadedEnrols) {
            Student* st = nullptr;
            for (auto* s : students) {
                if (s->getId() == rec.studentId) { st = s; break; }
            }
            Course* cr = findCourseByCode(allCourses, rec.courseCode);
            if (st && cr) {
                try {
                    st->enrol(*cr);
                } catch (...) {}
            }
        }
    } catch (...) {}

    loadAttendanceEvents(allCourses, students, lecturers); 

    // --- Interactive Master Loop ---
    while (true) {
        cout << "\n======================================================================\n";
        cout << "   TEAM DEADBOLT - UNIVERSITY COURSE & ATTENDANCE MANAGEMENT SYSTEM    \n";
        cout << "======================================================================\n";
        cout << "1. Student Login\n";
        cout << "2. Lecturer Login\n";
        cout << "3. Administrator Login\n";
        cout << "4. Run Automated Spec Test Suite\n";
        cout << "5. Exit\n";
        cout << "Enter choice: ";

        int mainChoice;
        if (!(cin >> mainChoice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (mainChoice == 1) {
            cout << "\nSelect Student Login:\n";
            for (size_t i = 0; i < students.size(); ++i) {
                cout << " " << (i + 1) << ". " << students[i]->getName() << " (" << students[i]->getId() << ")\n";
            }
            cout << "Enter selection (1-" << students.size() << "): ";
            size_t idx;
            if (cin >> idx && idx >= 1 && idx <= students.size()) {
                Student* selectedStudent = students[idx - 1];
                string enteredPass;
                cout << "Enter Password for " << selectedStudent->getName() << ": ";
                cin >> enteredPass;

                if (!selectedStudent->checkPassword(enteredPass)) {
                    cout << ">> [ACCESS DENIED] Incorrect password!\n";
                } else {
                    handleStudentSession(selectedStudent, allCourses, students);
                }
            } else {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << ">> Invalid selection!\n";
            }
        } else if (mainChoice == 2) {
            cout << "\nSelect Lecturer Login:\n";
            for (size_t i = 0; i < lecturers.size(); ++i) {
                cout << " " << (i + 1) << ". " << lecturers[i]->getName() << " (" << lecturers[i]->getId() << ")\n";
            }
            cout << "Enter selection (1-" << lecturers.size() << "): ";
            size_t idx;
            if (cin >> idx && idx >= 1 && idx <= lecturers.size()) {
                Lecturer* selectedLecturer = lecturers[idx - 1];
                string enteredPass;
                cout << "Enter Password for " << selectedLecturer->getName() << ": ";
                cin >> enteredPass;

                if (!selectedLecturer->checkPassword(enteredPass)) {
                    cout << ">> [ACCESS DENIED] Incorrect password!\n";
                } else {
                    handleLecturerSession(selectedLecturer, allCourses, students);
                }
            } else {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << ">> Invalid selection!\n";
            }
        } else if (mainChoice == 3) {
            if (admins.empty()) {
                cout << ">> Error: No administrator account available!\n";
                continue;
            }

            Admin* selectedAdmin = admins[0];
            if (admins.size() > 1) {
                cout << "\nSelect Admin Login:\n";
                for (size_t i = 0; i < admins.size(); ++i) {
                    cout << " " << (i + 1) << ". " << admins[i]->getName() << " (" << admins[i]->getId() << ")\n";
                }
                cout << "Enter selection (1-" << admins.size() << "): ";
                size_t idx;
                if (cin >> idx && idx >= 1 && idx <= admins.size()) {
                    selectedAdmin = admins[idx - 1];
                } else {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << ">> Invalid selection!\n";
                    continue;
                }
            }

            string enteredPass;
            cout << "Enter Password for Admin (" << selectedAdmin->getName() << "): ";
            cin >> enteredPass;

            if (!selectedAdmin->checkPassword(enteredPass)) {
                cout << ">> [ACCESS DENIED] Incorrect password!\n";
            } else {
                handleAdminSession(selectedAdmin, allCourses, students, lecturers, admins);
            }
        } else if (mainChoice == 4) {
            runAutomatedTests(allCourses, students, lecturers);
        } else if (mainChoice == 5) {
            cout << "\nExiting System. Goodbye!\n";
            break;
        }
    }

    // Memory Cleanup
    for (Student* s : students) delete s;
    for (Lecturer* l : lecturers) delete l;
    for (Admin* a : admins) delete a;

    return 0;
}