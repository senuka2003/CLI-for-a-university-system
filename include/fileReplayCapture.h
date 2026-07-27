#ifndef FILE_REPLAY_CAPTURE_H
#define FILE_REPLAY_CAPTURE_H

#include "AttendanceCapture.h"
#include <string>
#include <fstream>

class FileReplayCapture : public AttendanceCapture {
private:
    std::string inputFile;
    std::ifstream fileStream;

public:
    explicit FileReplayCapture(std::string inputFile);

    void beginSession() override;
    std::string captureNext() override;
    void endSession() override;
};

#endif // FILE_REPLAY_CAPTURE_H