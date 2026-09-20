#include "lectureCourse.h"
#include "labCourse.h"
#include "projectCourse.h"
#include "student.h"
#include "lecturer.h"
#include "admin.h"
#include "attendanceSession.h"
#include "attendanceCapture.h"
#include "rotatingCodeCapture.h"
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
void handleStudentSession(Student* student, const vector<Course*>& allCourses, const vector<Student*>& students) {
    while (true) {
        cout << "\n----------------------------------------\n";
        cout << " Logged in as Student: " << student->getName() << " (" << student->getId() << ")\n";
        cout << "----------------------------------------\n";

        student->displayMenu();
        cout << "5. View My Attendance Status \n";
        cout << "6. Logout\n";
        cout << "Enter choice: ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            cout << "Enter Course Code: ";
            string code;
            cin >> code;
            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << ">> Error: Course not found!\n";
            } else {
                try {
                    // target pointer eka dereference kara (*target)
                    student->enrol(*target);
                    cout << ">> Successfully enrolled in " << target->getTitle() << "!\n";
                    // students vector eka pass kara
                    syncEnrolmentsToFile(students);
                } catch (const DomainException& e) {
                    cout << ">> Enrolment Error: " << e.what() << "\n";
                }
            }
            pauseConsole();
        } 
        else if (choice == 2) {
            cout << "Enter Course Code: ";
            string code;
            cin >> code;
            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << ">> Error: Course not found!\n";
            } else {
                try {
                    // target pointer eka dereference kara (*target)
                    student->drop(*target);
                    cout << ">> Successfully dropped from " << target->getTitle() << "!\n";
                    // students vector eka pass kara
                    syncEnrolmentsToFile(students);
                } catch (const DomainException& e) {
                    cout << ">> Drop Error: " << e.what() << "\n";
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
                cout << "Enter Active Session ID: ";
                string sessionId;
                cin >> sessionId;

                AttendanceSession* targetSession = nullptr;
                for (AttendanceSession* s : allActiveSessions) {
                    if (s->getSessionId() == sessionId && s->getCourseCode() == target->getCode()) {
                        targetSession = s;
                        break;
                    }
                }

                if (!targetSession) {
                    cout << ">> Error: Active session with ID " << sessionId << " not found!\n";
                } else {
                    try {
                        targetSession->markPresent(student, target, "CONSOLE");
                        cout << ">> [SUCCESS] Attendance marked successfully for " << student->getName() << "!\n";

                        // Log event
                        ofstream logFile("data/attendance_events.txt", ios::app);
                        if (logFile.is_open()) {
                            time_t now = time(nullptr);
                            logFile << "ATTENDANCE_MARKED," 
                                    << student->getId() << "," 
                                    << target->getCode() << "," 
                                    << sessionId << ",present,CONSOLE," 
                                    << now << "\n";
                            logFile.close();
                        }
                    } catch (const DomainException& e) {
                        cout << ">> [FAILED] Attendance Error: " << e.what() << "\n";
                    }
                }
            }
            pauseConsole();
        } 
        else if (choice == 5) {
            viewStudentAttendanceReport(student, allCourses);
            pauseConsole();
        } 
        else if (choice == 6) {
            break;
        }
    }
}

void handleLecturerSession(Lecturer* lecturer, const vector<Course*>& allCourses) {
    while (true) {
        cout << "\n----------------------------------------\n";
        cout << " Logged in as Lecturer: " << lecturer->getName() << " (" << lecturer->getId() << ")\n";
        cout << "----------------------------------------\n";

        // Check active and auto-expired sessions
        if (!allActiveSessions.empty()) {
            cout << "\n--- Active Attendance Sessions ---\n";
            for (auto it = allActiveSessions.begin(); it != allActiveSessions.end(); ) {
                if ((*it)->isExpired()) {
                    cout << "[!] Notice: Session " << (*it)->getSessionId() 
                         << " (" << (*it)->getCourseCode() << ") has automatically EXPIRED!\n";

                    ofstream logFile("data/attendance_events.txt", ios::app);
                    if (logFile.is_open()) {
                        time_t now = time(nullptr);
                        logFile << "SESSION_EXPIRED," << (*it)->getCourseCode() << "," 
                                << (*it)->getSessionId() << "," << lecturer->getId() << "," 
                                << now << "\n";
                        logFile.close();
                    }

                    it = allActiveSessions.erase(it);
                } else {
                    cout << "[i] Active: Session " << (*it)->getSessionId() 
                         << " (" << (*it)->getCourseCode() << ") is currently RUNNING.\n";
                    ++it;
                }
            }
            cout << "----------------------------------\n\n";
        }

        lecturer->displayMenu();
        cout << "3. Correct Attendance Record\n";
        cout << "4. View Attendance & Eligibility Report\n";
        cout << "5. Logout\n";
        cout << "Enter choice: ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            cout << "Enter Course Code to view enrolled students: ";
            string code;
            cin >> code;
            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << "Course not found!\n";
            } else {
                try {
                    lecturer->viewEnrolmentList(target);
                } catch (const DomainException& e) {
                    cout << ">> Access Error: " << e.what() << "\n";
                }
            }
            pauseConsole();
        }
        else if (choice == 2) {
            cout << "Enter Course Code to open attendance session: ";
            string code;
            cin >> code;

            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << "Course not found!\n";
            } else {
                try {
                    if (target->getTimetable().getSlots().empty()) {
                        cout << "This course has no assigned time slots!\n";
                    } else {
                        int duration = 10;
                        cout << "Enter session duration in minutes (e.g., 10): ";
                        if (!(cin >> duration) || duration <= 0) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            duration = 10;
                        }

                        // Open session and track in both lists
                        AttendanceSession* s = lecturer->openAttendanceSession(target, target->getTimetable().getSlots()[0], duration);
                        allActiveSessions.push_back(s);
                        allSessions.push_back(s);

                        cout << ">> Session opened successfully!\n";
                        cout << ">> Session ID: " << s->getSessionId() << " (Duration: " << duration << " mins)\n";

                        ofstream logFile("data/attendance_events.txt", ios::app);
                        if (logFile.is_open()) {
                            time_t now = time(nullptr);
                            logFile << "SESSION_OPEN," << target->getCode() << "," 
                                    << s->getSessionId() << "," << lecturer->getId() << "," 
                                    << duration << "," << now << "\n";
                            logFile.close();
                        }

                        int sessionChoice = 0;
                        while (true) {
                            if (s->isExpired()) {
                                cout << "\n[!] Notice: Session duration elapsed. Session has automatically EXPIRED!\n";
                                break;
                            }

                            cout << "\n--- Active Session Menu (" << s->getSessionId() << ") ---\n";
                            cout << "1. Check session status\n";
                            cout << "2. Close session manually\n";
                            cout << "3. Replay Attendance from File \n";
                            cout << "4. Return to Lecturer Menu (Keep session running in background)\n";
                            cout << "Enter choice: ";
                            if (!(cin >> sessionChoice)) {
                                cin.clear();
                                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                continue;
                            }

                            if (sessionChoice == 1) {
                                if (s->isOpen()) {
                                    cout << ">> Session " << s->getSessionId() << " is currently ACTIVE.\n";
                                } else {
                                    cout << ">> Session " << s->getSessionId() << " is CLOSED / EXPIRED.\n";
                                    break;
                                }
                            } else if (sessionChoice == 2) {
                                s->close();
                                cout << ">> Session closed manually by Lecturer.\n";

                                ofstream closeLog("data/attendance_events.txt", ios::app);
                                if (closeLog.is_open()) {
                                    time_t now = time(nullptr);
                                    closeLog << "SESSION_MANUAL_CLOSED," << target->getCode() << "," 
                                             << s->getSessionId() << "," << lecturer->getId() << "," 
                                             << now << "\n";
                                    closeLog.close();
                                }
                                break;
                            } else if (sessionChoice == 3) {
                                // Pass Concrete object via Abstract Pointer
                                cout << "Enter file path (e.g., data/replay_attendance.txt): ";
                                string replayPath;
                                cin >> replayPath;

                                AttendanceCapture* captureEngine = new RotatingCodeCapture();
                                captureEngine->beginSession();
                                cout << ">> [ROTATING CODE] Active Code: " << captureEngine->captureNext() << "\n";
                                captureEngine->endSession();
                                delete captureEngine;
                            } else if (sessionChoice == 4) {
                                cout << ">> Returning to main menu. Session will keep running in background.\n";
                                break;
                            }
                        }
                    }
                } catch (const DomainException& e) {
                    cout << ">> Error: " << e.what() << "\n";
                }
            }
            pauseConsole();
        }
        else if (choice == 3) { // Attendance Correction
            cout << "Enter Course Code: ";
            string code;
            cin >> code;

            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << ">> Error: Course not found!\n";
            } else {
                cout << "Enter Session ID to correct: ";
                string sessionId;
                cin >> sessionId;

                AttendanceSession* targetSession = nullptr;
                for (AttendanceSession* s : allSessions) {
                    if (s->getSessionId() == sessionId && s->getCourseCode() == target->getCode()) {
                        targetSession = s;
                        break;
                    }
                }

                if (!targetSession) {
                    cout << ">> Error: Session not found in sessions list!\n";
                } else {
                    cout << "Enter Student ID: ";
                    string studentId;
                    cin >> studentId;

                    Student* targetStudent = nullptr;
                    for (Student* st : target->getEnrolledStudents()) {
                        if (st->getId() == studentId) {
                            targetStudent = st;
                            break;
                        }
                    }

                    if (!targetStudent) {
                        cout << ">> Error: Student not enrolled in this course!\n";
                    } else {
                        cout << "Enter Corrected Status (present/late/absent): ";
                        string newStatus;
                        cin >> newStatus;

                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Enter Reason for correction: ";
                        string reason;
                        getline(cin, reason);

                        try {
                            targetSession->addCorrection(targetStudent, newStatus, lecturer, reason);
                            cout << ">> [SUCCESS] Correction record appended successfully!\n";

                            ofstream logFile("data/attendance_events.txt", ios::app);
                            if (logFile.is_open()) {
                                time_t now = time(nullptr);
                                logFile << "ATTENDANCE_CORRECTION,"
                                        << targetStudent->getId() << ","
                                        << target->getCode() << ","
                                        << sessionId << ","
                                        << newStatus << ","
                                        << lecturer->getId() << ","
                                        << "\"" << reason << "\","
                                        << now << "\n";
                                logFile.close();
                            }
                        } catch (const DomainException& e) {
                            cout << ">> [FAILED] Correction Error: " << e.what() << "\n";
                        }
                    }
                }
            }
            pauseConsole();
        }
        else if (choice == 4) { // Attendance & Eligibility Reporting
            cout << "Enter Course Code: ";
            string code;
            cin >> code;

            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << ">> Error: Course not found!\n";
            } else {
                cout << "Enter Eligibility Threshold % (Default 80): ";
                double th = 80.0;
                cin >> th;
                if (cin.fail() || th <= 0.0) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    th = 80.0;
                }

                generateAttendanceReport(target, th);
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
        cout<< "Warning : could not load user records from data/users.txt. startig with empty user lists. \n";
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
                handleStudentSession(students[idx - 1], allCourses, students);
            }
        } else if (mainChoice == 2) {
            cout << "\nSelect Lecturer Login:\n";
            for (size_t i = 0; i < lecturers.size(); ++i) {
                cout << " " << (i + 1) << ". " << lecturers[i]->getName() << " (" << lecturers[i]->getId() << ")\n";
            }
            cout << "Enter selection (1-" << lecturers.size() << "): ";
            size_t idx;
            if (cin >> idx && idx >= 1 && idx <= lecturers.size()) {
                handleLecturerSession(lecturers[idx - 1], allCourses);
            }
        } else if (mainChoice == 3) {
            handleAdminSession(admins[0], allCourses,students,lecturers,admins);
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
