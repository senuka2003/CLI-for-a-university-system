#ifndef USER_RECORD_H
#define USER_RECORD_H

#include <string>
#include <sstream>
#include <stdexcept>

class UserRecord {
public:
    std::string id;
    std::string name;
    std::string email;
    std::string password;
    std::string role; // "Student", "Lecturer", "Admin"

    UserRecord() = default;
    UserRecord(std::string id, std::string name, std::string email, std::string password, std::string role)
        : id(std::move(id)), name(std::move(name)), email(std::move(email)), 
          password(std::move(password)), role(std::move(role)) {}

    std::string serialize() const {
        return id + "," + name + "," + email + "," + password + "," + role;
    }

    static UserRecord deserialize(const std::string& line) {
        std::stringstream ss(line);
        std::string id, name, email, pass, role;
        if (!std::getline(ss, id, ',') || !std::getline(ss, name, ',') ||
            !std::getline(ss, email, ',') || !std::getline(ss, pass, ',') ||
            !std::getline(ss, role, ',')) {
            throw std::runtime_error("Malformed user record: " + line);
        }
        return UserRecord(id, name, email, pass, role);
    }
};

#endif // USER_RECORD_H