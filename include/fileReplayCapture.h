#ifndef FILE_REPLAY_CAPTURE_H
#define FILE_REPLAY_CAPTURE_H

#include "attendanceCapture.h"
#include <string>
#include <fstream>
#include <vector>

class FileReplayCapture : public AttendanceCapture {
private:
    std::string filePath;
    std::vector<std::string> lines;
    size_t currentIndex;

public:
    FileReplayCapture(const std::string& path = "data/attendance_replay.txt");
    ~FileReplayCapture() override;

    void beginSession() override;
    std::string captureNext() override;
    void endSession() override;
};

#endif // FILE_REPLAY_CAPTURE_H