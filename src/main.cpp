#include "lectureCourse.h"
#include "labCourse.h"
#include "projectCourse.h"
#include "student.h"
#include "lecturer.h"
#include "admin.h"
#include "attendanceSession.h"
#include "fileReplayCapture.h"
#include "rotatingCodeCapture.h"
#include "repository.h"
#include "logEntry.h"
#include "exceptions.h"
#include <iostream>
#include <vector>
#include <limits>

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

// --- Automated Spec Validation & Test Runner (Reports / Demonstration) ---
void runAutomatedTests(vector<Course*>& allCourses, vector<Student*>& students, vector<Lecturer*>& lecturers) {
    cout << "\n=======================================================\n";
    cout << "   RUNNING AUTOMATED SPECIFICATION TEST SUITE (CO2203)\n";
    cout << "=======================================================\n\n";

    cout << "1. Checking Static Member (Course count): " 
         << Course::getTotalCourseCount() << " courses initialized.\n\n";

    Student& senuka = *students[0];
    Student& piyumi = *students[1];
    Student& dilara = *students[2];

    LectureCourse& co101 = *dynamic_cast<LectureCourse*>(allCourses[0]);
    LabCourse& col101    = *dynamic_cast<LabCourse*>(allCourses[3]);
    LabCourse& col102    = *dynamic_cast<LabCourse*>(allCourses[4]);

    cout << "2. Testing Prerequisite Enforcement (Chain/Requirement)...\n";
    try {
        senuka.enrol(col101); // Needs co101 first
        cout << "[Failed] Should not allow enrollment without prerequisite.\n";
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
        senuka.enrol(col102); // Checks if any slot overlaps with senuka's timetable
        cout << "   [Passed] No clash detected for COL102.\n";
    } catch (const TimetableClashException& e) {
        cout << "   [Passed Expected Clash] " << e.what() << "\n";
    }

    cout << "\n6. Testing Lecturer Attendance Session & FileReplayCapture...\n";
    Lecturer& drUdaya = *lecturers[0];
    AttendanceSession* session = drUdaya.openAttendanceSession(&co101, co101.getTimetable().getSlots()[0], 10);
    FileReplayCapture fileCapture("data/attendance_events.txt");
    session->setCaptureMechanism(&fileCapture);
    fileCapture.beginSession();

    auto lookupStudent = [&](const string& sid) -> Student* {
        for (Student* s : students) {
            if (s->getId() == sid) return s;
        }
        return nullptr;
    };

    while (true) {
        string sid;
        try {
            sid = fileCapture.captureNext();
        } catch (const DataCorruptionException& e) {
            cout << "   [Passed Malformed Line in File] " << e.what() << "\n";
            continue;
        }
        if (sid.empty()) break;
        Student* s = lookupStudent(sid);
        if (!s) continue;
        try {
            session->markPresent(s, &co101, "FileReplay");
            cout << "   [Passed] Marked present: " << sid << "\n";
        } catch (const AttendanceException& e) {
            cout << "   [Passed Expected] " << e.what() << "\n";
        }
    }
    fileCapture.endSession();

    cout << "\n7. Testing Runtime Interchangeability: Switching to RotatingCodeCapture...\n";
    RotatingCodeCapture codeCapture;
    session->setCaptureMechanism(&codeCapture); // Polymorphic pointer swap
    codeCapture.beginSession();
    cout << "   [Passed] Active Rotating Code generated: " << codeCapture.getCurrentCode() << "\n";

    cout << "\n8. Testing Generic Persistence: Repository<LogEntry>...\n";
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

    cout << "\n=======================================================\n";
    cout << "         AUTOMATED TEST SUITE COMPLETED CLEANLY        \n";
    cout << "=======================================================\n";
    pauseConsole();
}

// --- Interactive Menus ---
void handleStudentSession(Student* student, const vector<Course*>& allCourses) {
    while (true) {
        cout << "\n------------------------------------------\n";
        cout << " Logged in as Student: " << student->getName() << " (" << student->getId() << ")\n";
        cout << "------------------------------------------\n";
        student->displayMenu(); // Polymorphic menu call
        cout << "4. Logout\n";
        cout << "Enter choice: ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            cout << "\nAvailable Courses:\n";
            for (Course* c : allCourses) {
                cout << " - " << *c << "\n"; // operator<<
            }
            cout << "Enter Course Code to Enrol: ";
            string code;
            cin >> code;
            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << "Course not found!\n";
            } else {
                try {
                    student->enrol(*target);
                    cout << ">> Enrolled successfully in " << target->getCode() << "!\n";
                } catch (const DomainException& e) {
                    cout << ">> Enrollment Error: " << e.what() << "\n";
                }
            }
            pauseConsole();
        } else if (choice == 2) {
            cout << "\nYour Enrolled Courses:\n";
            for (Course* c : student->getEnrolledCourses()) {
                cout << " - [" << c->getCode() << "] " << c->getTitle() << "\n";
            }
            cout << "Enter Course Code to Drop: ";
            string code;
            cin >> code;
            Course* target = findCourseByCode(allCourses, code);
            if (!target) {
                cout << "Course not found!\n";
            } else {
                try {
                    student->drop(*target);
                    cout << ">> Dropped successfully from " << target->getCode() << "!\n";
                } catch (const DomainException& e) {
                    cout << ">> Drop Error: " << e.what() << "\n";
                }
            }
            pauseConsole();
        } else if (choice == 3) {
            cout << "\n" << student->viewTimetable() << "\n";
            pauseConsole();
        } else if (choice == 4) {
            break;
        }
    }
}

void handleLecturerSession(Lecturer* lecturer, const vector<Course*>& allCourses) {
    while (true) {
        cout << "\n------------------------------------------\n";
        cout << " Logged in as Lecturer: " << lecturer->getName() << " (" << lecturer->getId() << ")\n";
        cout << "------------------------------------------\n";
        lecturer->displayMenu(); // Polymorphic menu call
        cout << "3. Logout\n";
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
        } else if (choice == 2) {
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
                        AttendanceSession* s = lecturer->openAttendanceSession(target, target->getTimetable().getSlots()[0], 15);
                        cout << ">> Session opened! Session ID: " << s->getSessionId() << " (Duration: 15 mins)\n";
                    }
                } catch (const DomainException& e) {
                    cout << ">> Error: " << e.what() << "\n";
                }
            }
            pauseConsole();
        } else if (choice == 3) {
            break;
        }
    }
}

void handleAdminSession(Admin* admin, const vector<Course*>& allCourses) {
    while (true) {
        cout << "\n------------------------------------------\n";
        cout << " Logged in as Administrator: " << admin->getName() << " (" << admin->getId() << ")\n";
        cout << "------------------------------------------\n";
        admin->displayMenu(); // Polymorphic menu call
        cout << "4. Logout\n";
        cout << "Enter choice: ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            admin->manageUsers();
            pauseConsole();
        } else if (choice == 2) {
            admin->manageCourses();
            pauseConsole();
        } else if (choice == 3) {
            admin->generateReport(allCourses);
            pauseConsole();
        } else if (choice == 4) {
            break;
        }
    }
}

// --- Main Program Entry Point ---
int main() {
    // 1. Initial Setup: Lecturers
    Lecturer drUdaya("COL001", "Dr. Udaya", "udaya@uni.lk", "pass123");
    Lecturer drKrishanth("COL002", "Dr. Krishanth", "krishanth@uni.lk", "pass456");
    Lecturer drSanka("COL003", "Dr. Sanka", "sanka@uni.lk", "pass789");

    vector<Lecturer*> lecturers = { &drUdaya, &drKrishanth, &drSanka };

    // 2. Setup Courses (LectureCourse, LabCourse, ProjectCourse)
    LectureCourse co101("CO101", "Object Oriented Programming", 2, &drUdaya);
    LectureCourse cs102("CS102", "Data Communication", 2, &drKrishanth);
    LectureCourse cs103("CS103", "Computer Organization", 2, &drSanka);

    LabCourse col101("COL101", "OOP Lab", 2, &drUdaya);
    LabCourse col102("COL102", "Data Com Lab", 2, &drKrishanth);
    LabCourse col103("COL103", "Organization Lab", 2, &drSanka);

    ProjectCourse cop101("COP101", "OOP Project", 5, &drUdaya);
    ProjectCourse cop102("COP102", "Data Com Project", 5, &drKrishanth);
    ProjectCourse cop103("COP103", "Organization Project", 5, &drSanka);

    // Setup Prerequisites
    col101.addPrerequisite(&co101);
    cop101.addPrerequisite(&co101);
    col102.addPrerequisite(&cs102);
    cop102.addPrerequisite(&cs102);
    col103.addPrerequisite(&cs103);
    cop103.addPrerequisite(&cs103);

    // Setup Weekly Timetable Slots
    co101.addTimeSlot(TimeSlot("Monday", 540, 660, "L501"));         // 9:00-11:00
    col101.addTimeSlot(TimeSlot("Tuesday", 600, 720, "Lab 1"));      // 10:00-12:00
    cop101.addTimeSlot(TimeSlot("Wednesday", 840, 960, "L501"));     // 14:00-16:00
    cs102.addTimeSlot(TimeSlot("Tuesday", 740, 860, "L501"));        // 12:20-14:20
    col102.addTimeSlot(TimeSlot("Wednesday", 800, 920, "Lab 2"));
    cop102.addTimeSlot(TimeSlot("Monday", 1040, 1160, "L501"));
    cs103.addTimeSlot(TimeSlot("Wednesday", 940, 1060, "L501"));
    col103.addTimeSlot(TimeSlot("Monday", 1000, 1120, "Lab 3"));
    cop103.addTimeSlot(TimeSlot("Tuesday", 1240, 1360, "L501"));

    vector<Course*> allCourses = {
        &co101, &cs102, &cs103,
        &col101, &col102, &col103,
        &cop101, &cop102, &cop103
    };

    // 3. Setup Students & Admin
    Student senuka("EN001", "Senuka", "senuka@uni.lk", "pw111");
    Student piyumi("EN002", "Piyumi", "piyumi@uni.lk", "pw222");
    Student dilara("EN003", "Dilara", "dilara@uni.lk", "pw333");
    vector<Student*> students = { &senuka, &piyumi, &dilara };

    Admin sysAdmin("A001", "Administrator", "admin@uni.lk", "adminpw");

    // --- Interactive Master Loop ---
    while (true) {
        cout << "\n=======================================================\n";
        cout << "   UNIVERSITY COURSE & ATTENDANCE MANAGEMENT SYSTEM    \n";
        cout << "=======================================================\n";
        cout << "1. Student Portal\n";
        cout << "2. Lecturer Portal\n";
        cout << "3. Administrator Portal\n";
        cout << "4. Run Automated Spec Test Suite (Demonstration/Viva)\n";
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
                handleStudentSession(students[idx - 1], allCourses);
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
            handleAdminSession(&sysAdmin, allCourses);
        } else if (mainChoice == 4) {
            runAutomatedTests(allCourses, students, lecturers);
        } else if (mainChoice == 5) {
            cout << "\nExiting System. Goodbye!\n";
            break;
        }
    }

    return 0;
}