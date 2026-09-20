#ifndef ROTATING_CODE_CAPTURE_H
#define ROTATING_CODE_CAPTURE_H

#include "attendanceCapture.h"
#include <string>
#include <ctime>

class RotatingCodeCapture : public AttendanceCapture {
private:
    static std::string currentCode;
    static time_t codeExpiry;
    static const int CODE_VALID_SECONDS = 600; // 10 minutes

    std::string generateCode() const;
    void regenerateCode();

public:
    RotatingCodeCapture();
    ~RotatingCodeCapture() override = default;

    void beginSession() override;
    std::string captureNext() override;
    void endSession() override;

    bool isCodeExpired() const;
    std::string getCurrentCode() const;
};

#endif // ROTATING_CODE_CAPTURE_H