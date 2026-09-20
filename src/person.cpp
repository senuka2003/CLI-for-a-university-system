#include "person.h"
#include <utility>

using namespace std;

Person::Person(string id, string name, string email, string password)
    : id(move(id)), name(move(name)), email(move(email)), password(move(password)) {}

string Person::getId() const { 
    return id; 
}

string Person::getName() const { 
    return name; 
}

string Person::getEmail() const { 
    return email; 
}

string Person::getPassword() const { 
    return password; 
}

bool Person::checkPassword(const string& attempt) const { 
    return attempt == password; 
}

void Person::setId(const string& newId) { 
    id = newId; 
}

void Person::setName(const string& newName) { 
    name = newName; 
}

void Person::setEmail(const string& newEmail) { 
    email = newEmail; 
}

void Person::setPassword(const string& newPassword) { 
    password = newPassword; 
}