#include "LectureCourse.h"
#include "LabCourse.h"
#include "ProjectCourse.h"
#include "Student.h"
#include "Lecturer.h"
#include "Admin.h"
#include "AttendanceSession.h"
#include "FileReplayCapture.h"
#include "RotatingCodeCapture.h"
#include "Repository.h"
#include "LogEntry.h"
#include "Exceptions.h"
#include <iostream>
#include <vector>
using namespace std;

int main () {
    cout << "=====Deadbolt OOP Project System=====\n\n";

    // ---- Setup: Lecturer + Courses ----
    Lecturer drUdaya("COL001", "Dr. Udaya", "udaya@uni.lk", "pass123");
    Lecturer drKrishanth("COL002", "Dr. Krishanth", "krishanth@uni.lk", "pass456");
    Lecturer drSanka("COL003", "Dr. Sanka", "sanka@uni.lk", "pass789");

    LectureCourse co101("CO101", "Object Oriented Programming", 2, &drUdaya);
    LectureCourse cs102("CS102", "Data Communication", 2, &drKrishanth);
    LectureCourse cs103("CS103", "Computer Orgnization", 2, &drSanka);

    LabCourse col101("COL101", "OOP Lab", 1, &drUdaya);
    LabCourse col102("COL102", "Data Com Lab", 1, &drKrishanth);
    LabCourse col103("COL103", "Organization Lab", 1, &drSanka);

    ProjectCourse cop101("COP101", "OOP Project", 5, &drUdaya);
    ProjectCourse cop102("COP102", "Data Com Project", 5, &drKrishanth);
    ProjectCourse cop103("COP103", "Orgnization Project", 5, &drSanka);

    col101.addPrereqisite(&co101);  
    cop101.addPrereqisite(&co101);
    col102.addPrereqisite(&co102);
    cop102.addPrereqisite(&co102);
    col103.addPrereqisite(&co103);
    col103.addPrereqisite(&co103);

    cs101.addTimeSlot(TimeSlot("Monday", 540, 660, "L501")); // 9:00-11:00
    col101.addTimeSlot(TimeSlot("Tuesday", 600, 720, "Lab 1"));  // 9:00-12:00 - clashes with cs102!
    cop101.addTimeSlot(TimeSlot("Wednesday", 840, 960, "L501")); // 13:00-16:00

    cs102.addTimeSlot(TimeSlot("Tuesday", 740, 860, "L501")); // 9:00-11:00
    col102.addTimeSlot(TimeSlot("Wednesday", 800, 920, "Lab 2"));  // 9:00-12:00 - clashes with cs103!
    cop102.addTimeSlot(TimeSlot("Monday", 1040, 1160, "L501")); // 13:00-16:00

    cs103.addTimeSlot(TimeSlot("Wednesday", 940, 1060, "L501")); // 9:00-11:00
    col103.addTimeSlot(TimeSlot("Monday", 1000, 1120, "Lab 3"));  // 9:00-12:00 - clashes with cs101!
    cop103.addTimeSlot(TimeSlot("Tuesday", 1240, 1360, "L501")); // 13:00-16:00

    cout << "Total courses created (static member): " << Course::getTotalCourseCount() << "\n\n";

    // ---- Students ----
    Student senuka("EN001", "Senuka", "senuka@uni.lk", "pw111");
    Student piyumi("EN002", "Piyumi", "piyumi@uni.lk", "pw222");
    Student dilara("EN003", "Dilara", "dilara@uni.lk", "pw333");

    // ---- prerequisite violation ----
    try {
        senuka.enrol(col101); // col101 needs co101 first - should fail
    } catch (const PrerequisiteNotMetException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
    try {
        senuka.enrol(cop101); // cop101 needs co101 first - should fail
    } catch (const PrerequisiteNotMetException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
    try {
        piyumi.enrol(col102); // col102 needs co102 first - should fail
    } catch (const PrerequisiteNotMetException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }try {
        piyumi.enrol(cop102); // cop102 needs co102 first - should fail
    } catch (const PrerequisiteNotMetException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
     try {
        dilara.enrol(col103); // col103 needs co103 first - should fail
    } catch (const PrerequisiteNotMetException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
      try {
        dilara.enrol(cop103); // cop103 needs co103 first - should fail
    } catch (const PrerequisiteNotMetException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }

    // ---- Successful enrolment ----
    senuka.enrol(co101);
    piyumi.enrol(co102);
    dilara.enrol(co103);
    cout << "Senuka enrolled in co101.\n";
    cout << "Piyumi enrolled in co102.\n";
    cout << "Dilara enrolled in co103.\n";

    // ---- FR3.2/FR3.3: course full ----
    try {
        piyumi.enrol(co101); // capacity is 2, already full
    } catch (const CourseFullException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
     try {
        dilara.enrol(co102); // capacity is 2, already full
    } catch (const CourseFullException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
     try {
        senuka.enrol(co103); // capacity is 2, already full
    } catch (const CourseFullException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }

     // ---- timetable clash ----
    try {
        dilara.enrol(co101); //(co101 overlaps col103 on Monday)
    } catch (const TimetableClashException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
    try {
        senuka.enrol(co102); //(co102 overlaps col101 on Tuesday)
    } catch (const TimetableClashException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
    try {
        piyumi.enrol(co103); //(co103 overlaps col102 on Wednesday)
    } catch (const TimetableClashException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }

    // ---- Prerequisite is met, cs201 should succeed ----
    senuka.enrol(col101);
    cout << "Senuka enrolled in COL101 (prerequisite now satisfied).\n\n";
    senuka.enrol(cop101);
    cout << "Senuka enrolled in COP101 (prerequisite now satisfied).\n\n";
    piyumi.enrol(col1012);
    cout << "Piyumi enrolled in COL102 (prerequisite now satisfied).\n\n";
    piyumi.enrol(cop102);
    cout << "Piyumi enrolled in COP102 (prerequisite now satisfied).\n\n";
    dilara.enrol(col103);
    cout << "Dilara enrolled in COL103 (prerequisite now satisfied).\n\n";
    dilara.enrol(cop103);
    cout << "Dilara enrolled in COP103 (prerequisite now satisfied).\n\n";

     // ---- drop ----
    senuka.drop(co101);
    cout << "Senuka dropped CO101.\n";
    try {
        senuka.drop(co101); // already dropped
    } catch (const NotEnrolledException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }
    senuka.enrol(co101); // re-enrol for the attendance demo below
    cout << "Enrolled for CO101\n";

     // ---- view timetable + printing ----
    cout << senuka.viewTimetable();
    cout << senuka << "\n";
    cout << co101 << "\n";
    cout << col101 << "\n";
    cout << cop101 << "\n\n";

    cout << piyumi.viewTimetable();
    cout << piyumi << "\n";
    cout << co102 << "\n";
    cout << col102 << "\n";
    cout << cop102 << "\n\n";

    cout << dilara.viewTimetable();
    cout << dilara << "\n";
    cout << co103 << "\n";
    cout << col103 << "\n";
    cout << cop103 << "\n\n";

    // ---- lecturer viewing enrolment (own course - allowed) ----
    cout << "-- Dr. Udaya viewing CO101 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&co101);
    cout << "-- Dr. Udaya viewing COL101 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&col101);
    cout << "-- Dr. Udaya viewing COP101 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&cop101);

    cout << "-- Dr. Krishanth viewing CO102 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&co102);
    cout << "-- Dr. Krishanth viewing COL102 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&col102);
    cout << "-- Dr. Krishanth viewing COP102 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&cop102);

    cout << "-- Dr. Sanka viewing CO103 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&co103);
    cout << "-- Dr. Krishanth viewing COL103 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&col103);
    cout << "-- Dr. Krishanth viewing COP103 enrolment (authorized) --\n";
    drUdaya.viewEnrolmentList(&cop103);
    
    // ---- unauthorized access ----
    cout << "\n-- Dr. Krishanth trying to view unauthorized courses --\n";

    Course* unauthorizedCourses[] = { &co101, &col101, &cop101, &co103, &col103, &cop103 };

    for (Course* c : unauthorizedCourses) {
        try {
            drKrishanth.viewEnrolmentList(c);
            cout << "Unexpectedly allowed: " << c->getCode() << "\n";
     } catch (const UnauthorizedAccessException& e) {
        cout << "[Expected] " << e.what() << "\n";
        }
    }
    cout << "\n";

    cout << "\n-- Dr. Udaya trying to view unauthorized courses --\n";

    Course* unauthorizedCourses[] = { &co102, &col102, &cop102, &co103, &col103, &cop103 };

    for (Course* c : unauthorizedCourses) {
        try {
            drUdaya.viewEnrolmentList(c);
            cout << "Unexpectedly allowed: " << c->getCode() << "\n";
        } catch (const UnauthorizedAccessException& e) {
            cout << "[Expected] " << e.what() << "\n";
        }
    }
    cout << "\n";

    cout << "\n-- Dr. Sanka trying to view unauthorized courses --\n";

    Course* unauthorizedCourses[] = { &co101, &col101, &cop101, &co102, &col102, &cop102 };

    for (Course* c : unauthorizedCourses) {
        try {
            drSanka.viewEnrolmentList(c);
            cout << "Unexpectedly allowed: " << c->getCode() << "\n";
        } catch (const UnauthorizedAccessException& e) {
            cout << "[Expected] " << e.what() << "\n";
        }
    }
    cout << "\n";

    // ---- open attendance session (owned course, configurable duration) ----
    AttendanceSession* session = drUdaya.openAttendanceSession(
        &co101, co101.getTimetable().getSlots()[0], 10 /* minutes */);
    cout << "Opened attendance session: " << session->getSessionId() << "\n\n";

    // ---- capture mechanism, selectable at runtime ----
    cout << "-- Using FileReplayCapture (mandatory mechanism) --\n";
    FileReplayCapture fileCapture("data/attendance_events.txt");
    session->setCaptureMechanism(&fileCapture); // base-class pointer
    fileCapture.beginSession();

    // Map student IDs to Student*
    auto findStudent = [&](const string& sid) -> Student* {
        if (sid == "EN001") return &senuka;
        if (sid == "EN002") return &piyumi;
        if (sid == "EN003") return &dilara;
        return nullptr;
    };

    while (true) {
        string sid;
        try {
            sid = fileCapture.captureNext();
        } catch (const DataCorruptionException& e) {
            cout << "[Expected malformed line] " << e.what() << "\n";
            continue; // skip this bad line and keep processing
        }
        if (sid.empty()) break;

        Student* s = findStudent(sid);
        if (!s) continue;

        try {
            session->markPresent(s, &co101, "FileReplay");
            cout << "Marked present: " << sid << "\n";
        } catch (const SessionClosedException& e) {
            cout << "[Expected] " << e.what() << "\n";
        } catch (const NotEnrolledException& e) {
            cout << "[Expected] " << e.what() << "\n";
        } catch (const DuplicateAttendanceException& e) {
            cout << "[Expected] " << e.what() << "\n";
        }
    }
    fileCapture.endSession();
    cout << "\n";

    // ---- swap to the interactive capture mechanism at runtime ----
    cout << "-- Switching to RotatingCodeCapture (interchangeable via base pointer) --\n";
    RotatingCodeCapture codeCapture;
    session->setCaptureMechanism(&codeCapture); // same base pointer, different concrete type
    codeCapture.beginSession();
    cout << "Current session code: " << codeCapture.getCurrentCode() << "\n\n";

    // ---- correction, appended not edited ----
    session->addCorrection(&senuka, "present", &drUdaya, "manual correction after review");
    cout << "Correction appended for Senuka.\n\n";

    cout << "Piyumi attendance % in CO102: " << co102.getAttendancePercentage(&piyumi) << "%\n";
    cout << "CO102 overall attendance %: " << co102.getCourseAttendancePercentage() << "%\n\n";

    // ---- admin report ----
    Admin admin("A001", "Admin User", "admin@uni.lk", "adminpw");
    vector<Course*> allCourses = { &co101, &col101, &cop101, &co102, &col102, &cop102, &co103, &col102, &cop103 };
    admin.generateReport(allCourses);
    cout << "\n";

    // ---- Repository<T> demo ----
    cout << "-- Repository<T> persistence demo --\n";
    Repository<LogEntry> logRepo;
    vector<LogEntry> logs = {
        LogEntry("EN001", "CO101", "enrol"),
        LogEntry("EN001", "COL101", "enrol"),
        LogEntry("EN001", "COP101", "enrol")
        LogEntry("EN002", "CO102", "enrol"),
        LogEntry("EN002", "COL102", "enrol"),
        LogEntry("EN002", "COP102", "enrol")
        LogEntry("EN003", "CO103", "enrol"),
        LogEntry("EN003", "COL103", "enrol"),
        LogEntry("EN003", "COP103", "enrol")
    };
    logRepo.saveToFile("data/logs.txt", logs);
    vector<LogEntry> loaded = logRepo.loadFromFile("data/logs.txt");
    for (const auto& entry : loaded) {
        cout << "Loaded: " << entry.toString() << "\n";
    }

    try {
        logRepo.loadFromFile("data/does_not_exist.txt"); // missing file
    } catch (const DataCorruptionException& e) {
        cout << "[Expected] " << e.what() << "\n";
    }

    cout << "\n===== Completed =====\n";
    return 0;
}