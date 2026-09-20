#ifndef ROTATING_CODE_CAPTURE_H
#define ROTATING_CODE_CAPTURE_H

#include "attendanceCapture.h"
#include <string>
#include <ctime>

class RotatingCodeCapture : public AttendanceCapture {
private:
    static constexpr int CODE_VALID_SECONDS = 30;
    
    std::string currentCode;
    time_t codeExpiry;

    std::string generateCode() const;
    void regenerateCode();
    bool isCodeExpired() const;

public:
    RotatingCodeCapture();
    ~RotatingCodeCapture() override = default;

    void beginSession() override;
    std::string captureNext() override;
    void endSession() override;

    std::string getCurrentCode() const;
};

#endif // ROTATING_CODE_CAPTURE_H