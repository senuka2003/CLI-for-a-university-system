#ifndef ROTATING_CODE_CAPTURE_H
#define ROTATING_CODE_CAPTURE_H

#include "AttendanceCapture.h"
#include <string>
#include <ctime>

class RotatingCodeCapture : public AttendanceCapture {
private:
    std::string currentCode;
    time_t codeExpiry;
    static const int CODE_VALID_SECONDS = 30;

    std::string generateCode() const;

public:
    RotatingCodeCapture();

    void beginSession() override;
    std::string captureNext() override;
    void endSession() override;

    void regenerateCode();
    bool isCodeExpired() const;
    std::string getCurrentCode() const;
};

#endif // ROTATING_CODE_CAPTURE_H