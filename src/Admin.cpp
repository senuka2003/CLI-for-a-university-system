#include "Admin.h"
#include "Course.h"
#include "lectureCourse.h"
#include "labCourse.h"
#include "projectCourse.h"
#include "Student.h"
#include "Lecturer.h"
#include "repository.h"
#include "userRecord.h"
#include "courseRecord.h"
#include "timeSlot.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <fstream>

using namespace std;

namespace {
void syncUsersToFile(const vector<Student*>& students, 
                     const vector<Lecturer*>& lecturers, 
                     const vector<Admin*>& admins) {
    vector<UserRecord> records;
    for (const auto* s : students) {
        records.emplace_back(s->getId(), s->getName(), s->getEmail(), "pw111", "Student");
    }
    for (const auto* l : lecturers) {
        records.emplace_back(l->getId(), l->getName(), l->getEmail(), "pass123", "Lecturer");
    }
    for (const auto* a : admins) {
        records.emplace_back(a->getId(), a->getName(), a->getEmail(), "adminpw", "Admin");
    }
    Repository<UserRecord> repo;
    repo.saveToFile("data/users.txt", records);
}

void syncCoursesToFile(const vector<Course*>& allCourses) {
    vector<CourseRecord> records;
    for (const auto* c : allCourses) {
        string ctype = "Lecture";
        if (dynamic_cast<const LabCourse*>(c)) ctype = "Lab";
        else if (dynamic_cast<const ProjectCourse*>(c)) ctype = "Project";

        string lecId = c->getLecturer() ? c->getLecturer()->getId() : "NONE";
        string prereq = "NONE";
        if (!c->getPrerequisites().empty() && c->getPrerequisites()[0]) {
            prereq = c->getPrerequisites()[0]->getCode();
        }
        records.emplace_back(ctype, c->getCode(), c->getTitle(), c->getCapacity(), lecId, prereq);
    }
    Repository<CourseRecord> repo;
    repo.saveToFile("data/courses.txt", records);
}
}

Admin::Admin(string id, string name, string email, string password)
    : Person(move(id), move(name), move(email), move(password)) {}

void Admin::displayMenu() const {
    cout << "--- Administrator Menu ---\n"
         << "1. Manage users\n"
         << "2. Manage courses\n"
         << "3. Generate report\n";
}

string Admin::getRole() const { 
    return "Administrator"; 
}

static void saveTimeSlotToFile(const std::string& filename, const std::string& courseCode, const TimeSlot& slot) {
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

void Admin::manageUsers(vector<Student*>& students, 
                         vector<Lecturer*>& lecturers, 
                         vector<Admin*>& admins) const {
    while (true) {
        cout << "\n=== User Management ===\n";
        cout << "1. List All Users\n";
        cout << "2. Create New User (Student / Lecturer / Admin)\n";
        cout << "3. Remove User\n";
        cout << "4. Back to Main Admin Menu\n";
        cout << "Enter choice: ";

        int ch;
        if (!(cin >> ch)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (ch == 1) {
            cout << "\n--- Current Users ---\n";
            for (const auto* s : students) cout << " [Student] " << s->getId() << " - " << s->getName() << "\n";
            for (const auto* l : lecturers) cout << " [Lecturer] " << l->getId() << " - " << l->getName() << "\n";
            for (const auto* a : admins) cout << " [Admin] " << a->getId() << " - " << a->getName() << "\n";
        } else if (ch == 2) {
            cout << "\nSelect Type (1: Student, 2: Lecturer, 3: Admin): ";
            int r; cin >> r;
            string uid, uname, uemail, upass;
            cout << "ID: "; cin >> uid;
            cout << "Name: "; cin >> uname;
            cout << "Email: "; cin >> uemail;
            cout << "Password: "; cin >> upass;

            if (r == 1) students.push_back(new Student(uid, uname, uemail, upass));
            else if (r == 2) lecturers.push_back(new Lecturer(uid, uname, uemail, upass));
            else if (r == 3) admins.push_back(new Admin(uid, uname, uemail, upass));

            syncUsersToFile(students, lecturers, admins);
            cout << ">> User created and saved to data/users.txt!\n";
        } else if (ch == 3) {
            string uid;
            cout << "Enter ID to remove: "; cin >> uid;
            auto sIt = find_if(students.begin(), students.end(), [&](Student* s){ return s->getId() == uid; });
            if (sIt != students.end()) { delete *sIt; students.erase(sIt); syncUsersToFile(students, lecturers, admins); cout << ">> Removed.\n"; continue; }

            auto lIt = find_if(lecturers.begin(), lecturers.end(), [&](Lecturer* l){ return l->getId() == uid; });
            if (lIt != lecturers.end()) { delete *lIt; lecturers.erase(lIt); syncUsersToFile(students, lecturers, admins); cout << ">> Removed.\n"; continue; }

            auto aIt = find_if(admins.begin(), admins.end(), [&](Admin* a){ return a->getId() == uid; });
            if (aIt != admins.end()) {
                if (admins.size() <= 1) cout << ">> Cannot remove the sole administrator account!\n";
                else { delete *aIt; admins.erase(aIt); syncUsersToFile(students, lecturers, admins); cout << ">> Removed.\n"; }
                continue;
            }
            cout << ">> User not found!\n";
        } else if (ch == 4) break;
    }
}

static void syncAllTimeSlotsToFile(const std::string& filename, const std::vector<Course*>& allCourses) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    for (Course* c : allCourses) {
        for (const auto& slot : c->getTimetable().getSlots()) {
            file << c->getCode() << ","
                 << slot.getDay() << ","
                 << slot.getStartTime() << ","
                 << slot.getEndTime() << ","
                 << slot.getLocation() << "\n";
        }
    }
    file.close();
}

// FR2.1 Course Management Implementation
void Admin::manageCourses(vector<Course*>& allCourses, const vector<Lecturer*>& lecturers) const {
    while (true) {
        cout << "\n=== Course Management ===\n";
        cout << "1. List All Courses\n";
        cout << "2. Create New Course Offering\n";
        cout << "3. Edit Course \n";
        cout << "4. Remove Course Offering\n";
        cout << "5. Back to Main Admin Menu\n";
        cout << "Enter choice: ";

        int ch;
        if (!(cin >> ch)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (ch == 1) {
            cout << "\n--- Current Course Offerings ---\n";
            for (const auto* c : allCourses) {
                cout << " * [" << c->getCode() << "] " << c->getTitle() 
                     << " | Capacity: " << c->getCapacity()
                     << " | Lecturer: " << (c->getLecturer() ? c->getLecturer()->getName() : "None") << "\n";
            }
        } else if (ch == 2) {
            cout << "\nSelect Course Type:\n1. LectureCourse\n2. LabCourse\n3. ProjectCourse\nEnter (1-3): ";
            int typeChoice; cin >> typeChoice;
            string code, title, lecId, prereqCode;
            int cap;

            cout << "Enter Course Code (e.g. CO201): "; cin >> code;
            cout << "Enter Course Title: "; cin.ignore(); getline(cin, title);
            cout << "Enter Capacity: "; cin >> cap;

            cout << "Available Lecturers:\n";
            for (const auto* l : lecturers) cout << "  " << l->getId() << " : " << l->getName() << "\n";
            cout << "Enter Lecturer ID to assign: "; cin >> lecId;

            Lecturer* assignedLec = nullptr;
            for (auto* l : lecturers) {
                if (l->getId() == lecId) { assignedLec = l; break; }
            }
            if (!assignedLec && !lecturers.empty()) assignedLec = lecturers[0];

            Course* newCourse = nullptr;
            if (typeChoice == 1) newCourse = new LectureCourse(code, title, cap, assignedLec);
            else if (typeChoice == 2) newCourse = new LabCourse(code, title, cap, assignedLec);
            else if (typeChoice == 3) newCourse = new ProjectCourse(code, title, cap, assignedLec);

            if (newCourse) {
                cout << "Enter Prerequisite Course Code (or 'NONE'): "; cin >> prereqCode;
                if (prereqCode != "NONE") {
                    for (auto* c : allCourses) {
                        if (c->getCode() == prereqCode) {
                            newCourse->addPrerequisite(c);
                            break;
                        }
                    }
                }
                allCourses.push_back(newCourse);
                syncCoursesToFile(allCourses);
                cout << ">> Course " << code << " created and persisted to data/courses.txt!\n";

                char addSlotChoice;
        cout << "Do you want to add a Time Slot for this new course? (y/n): ";
        cin >> addSlotChoice;

        if (addSlotChoice == 'y' || addSlotChoice == 'Y') {
            string day, start, end, loc;
            cout << "Enter Day (e.g., Monday): ";
            cin >> day;
            cout << "Enter Start Time (HH:MM): ";
            cin >> start;
            cout << "Enter End Time (HH:MM): ";
            cin >> end;
            cout << "Enter Location: ";
            cin.ignore();
            getline(cin, loc);

            TimeSlot newSlot(day, start, end, loc);
            newCourse->addTimeSlot(newSlot);
            saveTimeSlotToFile("data/timeslot.txt", newCourse->getCode(), newSlot);

            cout << ">> Time slot added successfully!\n";
        }
            } else {
                cout << ">> Invalid course type selected!\n";
            }

         } else if (ch == 3) {
        string code;
        cout << "Enter Course Code to Edit: ";
        cin >> code;

        auto it = find_if(allCourses.begin(), allCourses.end(), [&](Course* c){ return c->getCode() == code; });
        if (it == allCourses.end()) {
            cout << ">> Course not found!\n";
            continue;
        }

        Course* targetCourse = *it;
        cout << "\n============================================\n";
        cout << "Editing Course: [" << targetCourse->getCode() << "] " << targetCourse->getTitle() << "\n";
        cout << "Current Lecturer: " << (targetCourse->getLecturer() ? targetCourse->getLecturer()->getName() : "None") << "\n";
        cout << "============================================\n";
        cout << "1. Edit Title\n";
        cout << "2. Edit Capacity\n";
        cout << "3. Change Assigned Lecturer\n";
        cout << "4. Add New Time Slot\n";
        cout << "5. Edit Existing Time Slot\n";
        cout << "6. Delete a Time Slot\n";
        cout << "Enter your choice (1-6): ";
        int editChoice;
        cin >> editChoice;

        if (editChoice == 1) {
            cout << "Current Title: " << targetCourse->getTitle() << "\n";
            cout << "Enter New Title: ";
            cin.ignore();
            string newTitle;
            getline(cin, newTitle);
            targetCourse->setTitle(newTitle);
            syncCoursesToFile(allCourses);
            cout << ">> Course title updated!\n";

        } else if (editChoice == 2) {
            cout << "Current Capacity: " << targetCourse->getCapacity() << "\n";
            cout << "Enter New Capacity: ";
            int newCap;
            cin >> newCap;
            targetCourse->setCapacity(newCap);
            syncCoursesToFile(allCourses);
            cout << ">> Course capacity updated!\n";

        } else if (editChoice == 3) {
            cout << "\nAvailable Lecturers:\n";
            for (const auto* l : lecturers) {
                cout << "  [" << l->getId() << "] " << l->getName() << "\n";
            }
            cout << "Enter Lecturer ID to assign: ";
            string lecId;
            cin >> lecId;

            Lecturer* assignedLec = nullptr;
            for (auto* l : lecturers) {
                if (l->getId() == lecId) {
                    assignedLec = l;
                    break;
                }
            }

            if (assignedLec) {
                targetCourse->setLecturer(assignedLec);
                syncCoursesToFile(allCourses);
                cout << ">> Assigned lecturer updated successfully!\n";
            } else {
                cout << ">> Lecturer ID not found!\n";
            }

        } else if (editChoice == 4) {
            string day, start, end, loc;
            cout << "Enter Day (e.g., Monday): ";
            cin >> day;
            cout << "Enter Start Time (HH:MM): ";
            cin >> start;
            cout << "Enter End Time (HH:MM): ";
            cin >> end;
            cout << "Enter Location (e.g., Lab 1): ";
            cin.ignore();
            getline(cin, loc);

            TimeSlot newSlot(day, start, end, loc);
            targetCourse->addTimeSlot(newSlot);
            syncAllTimeSlotsToFile("data/timeTable.txt", allCourses);
            cout << ">> Time slot added successfully!\n";

        } else if (editChoice == 5) {
            auto& slots = targetCourse->getTimetable().getSlots();
            if (slots.empty()) {
                cout << ">> No time slots found for this course!\n";
            } else {
                cout << "\nCurrent Time Slots:\n";
                for (size_t i = 0; i < slots.size(); ++i) {
                    cout << "  " << (i + 1) << ". " << slots[i].getDay() 
                         << " [" << slots[i].getStartTime() << " - " << slots[i].getEndTime() 
                         << "] @ " << slots[i].getLocation() << "\n";
                }
                cout << "Select slot number to edit (1-" << slots.size() << "): ";
                size_t slotIdx;
                cin >> slotIdx;

                if (slotIdx >= 1 && slotIdx <= slots.size()) {
                    string day, start, end, loc;
                    cout << "Enter New Day: ";
                    cin >> day;
                    cout << "Enter New Start Time (HH:MM): ";
                    cin >> start;
                    cout << "Enter New End Time (HH:MM): ";
                    cin >> end;
                    cout << "Enter New Location: ";
                    cin.ignore();
                    getline(cin, loc);

                   targetCourse->updateTimeSlot(slotIdx - 1, TimeSlot(day, start, end, loc));
                    syncAllTimeSlotsToFile("data/timeTable.txt", allCourses);
                    cout << ">> Time slot updated successfully!\n";
                } else {
                    cout << ">> Invalid slot number!\n";
                }
            }

        } else if (editChoice == 6) {
            auto& slots = targetCourse->getTimetable().getSlots();
            if (slots.empty()) {
                cout << ">> No time slots found to delete!\n";
            } else {
                cout << "\nCurrent Time Slots:\n";
                for (size_t i = 0; i < slots.size(); ++i) {
                    cout << "  " << (i + 1) << ". " << slots[i].getDay() 
                         << " [" << slots[i].getStartTime() << " - " << slots[i].getEndTime() 
                         << "] @ " << slots[i].getLocation() << "\n";
                }
                cout << "Select slot number to delete (1-" << slots.size() << "): ";
                size_t slotIdx;
                cin >> slotIdx;

                if (slotIdx >= 1 && slotIdx <= slots.size()) {
                    targetCourse->removeTimeSlot(slotIdx - 1);
                    syncAllTimeSlotsToFile("data/timeTable.txt", allCourses);
                    cout << ">> Time slot deleted successfully!\n";
                } else {
                    cout << ">> Invalid slot number!\n";
                }
            }

        } else {
            cout << ">> Invalid choice!\n";
        }
        } else if (ch == 4) {
            string code;
            cout << "Enter Course Code to Remove: "; cin >> code;
            auto it = find_if(allCourses.begin(), allCourses.end(), [&](Course* c){ return c->getCode() == code; });
            if (it != allCourses.end()) {
                delete *it;
                allCourses.erase(it);
                syncCoursesToFile(allCourses);
                syncAllTimeSlotsToFile("data/timeTable.txt", allCourses);
                cout << ">> Course removed and updated in data/courses.txt!\n";
            } else {
                cout << ">> Course not found!\n";
            }
        } else if (ch == 5) {
            break;
        }
    }
}

void Admin::generateReport(const vector<Course*>& allCourses) const {
    cout << "=== Enrolment Summary Report ===\n";
    for (Course* course : allCourses) {
        if (!course) continue;
        
        cout << *course << "\n";

        if (course->getLecturer() != nullptr) {
            cout << "  Lecturer: " << course->getLecturer()->getName() 
                 << " (" << course->getLecturer()->getId() << ")\n";
        } else {
            cout << "  Lecturer: Not Assigned\n";
        }

        cout << "  Course attendance avg: " 
             << course->getCourseAttendancePercentage() << "%\n";

        const auto& enrolled = course->getEnrolledStudents();
        if (enrolled.empty()) {
            cout << "  Enrolled Students: None\n";
        } else {
            cout << "  Enrolled Students:\n";
            for (const auto* s : enrolled) {
                if (s != nullptr) {
                    cout << "    - [" << s->getId() << "] " << s->getName() << "\n";
                }
            }
        }
        cout << "\n";
    }
}