#include "rotatingCodeCapture.h"
#include <cstdlib>

using namespace std;

string RotatingCodeCapture::currentCode = "";
time_t RotatingCodeCapture::codeExpiry = 0;

RotatingCodeCapture::RotatingCodeCapture() {
}

string RotatingCodeCapture::generateCode() const {
    int code = 1000 + (rand() % 9000);
    return to_string(code);
}

void RotatingCodeCapture::beginSession() {
    if (currentCode.empty() || isCodeExpired()) {
        regenerateCode();
    }
}

void RotatingCodeCapture::regenerateCode() {
    currentCode = generateCode();
    codeExpiry = time(nullptr) + CODE_VALID_SECONDS;
}

bool RotatingCodeCapture::isCodeExpired() const {
    return time(nullptr) > codeExpiry;
}

string RotatingCodeCapture::getCurrentCode() const {
    if (isCodeExpired()) {
        return "";
    }
    return currentCode;
}

string RotatingCodeCapture::captureNext() {
    if (currentCode.empty() || isCodeExpired()) {
        regenerateCode();
    }
    return currentCode;
}

void RotatingCodeCapture::endSession() {

}