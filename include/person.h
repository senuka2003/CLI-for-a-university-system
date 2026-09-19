#ifndef PERSON_H
#define PERSON_H

#include <string>

class Person {
protected:
    std::string id;
    std::string name;
    std::string email;
    std::string password;

public:
    Person(std::string id, std::string name, std::string email, std::string password);
    virtual ~Person() = default;
    virtual void displayMenu() const = 0;
    virtual std::string getRole() const = 0;

    std::string getId() const;
    std::string getName() const;
    std::string getEmail() const;
    bool checkPassword(const std::string& attempt) const;
};

#endif // PERSON_H
