#include "RotatingCodeCapture.h"
#include <cstdlib>

using namespace std;

RotatingCodeCapture::RotatingCodeCapture() : currentCode(""), codeExpiry(0) {}

string RotatingCodeCapture::generateCode() const {
    int code = 1000 + (rand() % 9000); // 4-digit code
    return to_string(code);
}

void RotatingCodeCapture::beginSession() {
    regenerateCode();
}

void RotatingCodeCapture::regenerateCode() {
    currentCode = generateCode();
    codeExpiry = time(nullptr) + CODE_VALID_SECONDS;
}

bool RotatingCodeCapture::isCodeExpired() const {
    return time(nullptr) > codeExpiry;
}

string RotatingCodeCapture::getCurrentCode() const {
    return currentCode;
}

string RotatingCodeCapture::captureNext() {
    if (isCodeExpired()) {
        regenerateCode();
    }
    return currentCode;
}

void RotatingCodeCapture::endSession() {
    currentCode.clear();
}