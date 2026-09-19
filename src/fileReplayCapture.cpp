#include "FileReplayCapture.h"
#include "Exceptions.h"

using namespace std;

FileReplayCapture::FileReplayCapture(string inputFile) : inputFile(move(inputFile)) {}

void FileReplayCapture::beginSession() {
    fileStream.open(inputFile);
    if (!fileStream.is_open()) {
        throw DataCorruptionException(inputFile, "cannot open capture file");
    }
}

string FileReplayCapture::captureNext() {
    string line;
    if (!std::getline(fileStream, line)) {
        return ""; // no more events
    }
    if (line.empty()) {
        return captureNext(); // skip blank lines quietly
    }

    // Expected format: "studentId,timestamp"
    auto commaPos = line.find(',');
    if (commaPos == string::npos || commaPos == 0) {
        throw DataCorruptionException(inputFile, "malformed line: " + line);
    }
    return line.substr(0, commaPos);
}

void FileReplayCapture::endSession() {
    if (fileStream.is_open()) fileStream.close();
}