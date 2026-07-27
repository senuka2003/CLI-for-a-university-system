#include "Person.h"

using namespace std;

Person::Person(string id, string name, string email, string password)
    : id(move(id)), name(move(name)), email(move(email)), password(move(password)) {}

string Person::getID() const { return id; }
string Person::getName() const { return name; }
string Person::getEmail() const { return email; }
bool Person::checkPassword(const string& attempt) const { return attempt == password; }
