#include "fileReplayCapture.h"
#include "exceptions.h"
#include <iostream>

using namespace std;

FileReplayCapture::FileReplayCapture(const string& path)
    : filePath(path), currentIndex(0) {}

FileReplayCapture::~FileReplayCapture() {
    endSession();
}

void FileReplayCapture::beginSession() {
    lines.clear();
    currentIndex = 0;
    ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        throw DomainException("FileReplayCapture: Could not open file " + filePath);
    }

    string line;
    while (getline(fileStream, line)) {
        // Trim carriage return if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    fileStream.close();
}

string FileReplayCapture::captureNext() {
    if (currentIndex >= lines.size()) {
        return ""; // lines iwarai
    }

    string line = lines[currentIndex++];

    if (line.length() < 3) {
        throw DomainException("FileReplayCapture: Malformed line encountered -> '" + line + "'");
    }

    return line;
}

void FileReplayCapture::endSession() {
    lines.clear();
    currentIndex = 0;
}