# Team Deadbolt
    24/ENG/062
    24/ENG/064
    24/ENG/100
    
# University Course & Attendance Management System

A modular C++ Object-Oriented Course and Attendance Management System designed to handle academic operations, course enrolments, schedule timetables, automated attendance tracking strategies, and administrative user management.

---

## Architecture & Design Overview

The system strictly adheres to Object-Oriented Programming (OOP) principles:
- **Inheritance & Polymorphism:** 
  - Base `Person` class extended by `Student`, `Lecturer`, and `Admin`.
  - Abstract base `Course` extended by `LectureCourse`, `LabCourse`, and `ProjectCourse`.
  - Abstract `AttendanceCapture` interface implemented by `RotatingCodeCapture` and `FileReplayCapture`.
- **Encapsulation:** Safe accessors, mutators, and credential verification (`checkPassword`).
- **Generic File Persistence:** Generic `Repository<T>` template engine powering text file persistence with CSV records.
- **Robust Exception Handling:** Domain-specific exceptions (`exceptions.h`) for capacity breaches, prerequisite validation failures, and duplicate enrolments.

---

## Key Features

### 1. Multi-Role Authentication & Access Control
- Independent login authentication for **Student**, **Lecturer**, and **Administrator**.
- Credential validation backed by file persistence and runtime user object updates.

### 2. Administrator Capabilities
- **User Management:** Create, view, delete, and modify users (ID, Name, Email, Password) with bidirectional synchronization to `data/users.txt`.
- **Course Administration:** Add and manage Course configurations (Capacities, assigned Lecturers, Prerequisites, and Timetable slots).
- **Academic Reporting:** Generate enrolment lists and attendance performance summaries.

### 3. Course Enrolment & Prerequisites
- Dynamic prerequisites verification preventing circular or unsatisfied dependencies.
- Course capacity enforcement and duplicate enrolment guards.

### 4. Attendance Tracking Strategies
- **Rotating Code Capture:** Dynamic passcode verification for physical/virtual sessions.
- **File Replay Capture:** Automated batch attendance simulation from external session data.

---

## Project Structure

```text
OOP Project/
├── data/
│   ├── attendance_events.txt
│   ├── attendance_replay.txt
│   ├── courses.txt
│   ├── enrolments.txt
│   ├── timeTable.txt
│   └── users.txt
├── include/
│   ├── admin.h
│   ├── attendanceCapture.h
│   ├── attendanceRecord.h
│   ├── attendanceSession.h
│   ├── course.h
│   ├── courseRecord.h
│   ├── enrolmentRecord.h
│   ├── exceptions.h
│   ├── fileReplayCapture.h
│   ├── labCourse.h
│   ├── lectureCourse.h
│   ├── lecturer.h
│   ├── logEntry.h
│   ├── person.h
│   ├── projectCourse.h
│   ├── repository.h
│   ├── rotatingCodeCapture.h
│   ├── student.h
│   ├── timeSlot.h
│   ├── timeTable.h
│   └── userRecord.h
├── src/
│   ├── Admin.cpp
│   ├── attendanceRecord.cpp
│   ├── attendanceSession.cpp
│   ├── course.cpp
│   ├── fileReplayCapture.cpp
│   ├── labCourse.cpp
│   ├── lectureCourse.cpp
│   ├── Lecturer.cpp
│   ├── main.cpp
│   ├── person.cpp
│   ├── projectCourse.cpp
│   ├── rotatingCodeCapture.cpp
│   ├── student.cpp
│   ├── timeSlot.cpp
│   └── timeTable.cpp
├── Makefile
└── README.md