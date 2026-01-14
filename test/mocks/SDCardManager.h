#pragma once

// Mock SDCardManager for native builds
// Provides minimal interface for code that uses the SD card manager

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

// Include our mock SdFat which provides FsFile
#include "SdFat.h"

// Include Arduino.h for String class
#include "Arduino.h"

// Include Print for stream operations
#include "Print.h"

// File open flags (if not already defined)
#ifndef O_RDONLY
#define O_RDONLY O_READ
#endif

#ifndef O_WRONLY
#define O_WRONLY O_WRITE
#endif

#ifndef O_RDWR
// Already defined in SdFat.h
#endif

typedef uint8_t oflag_t;

class SDCardManager {
public:
    SDCardManager() = default;
    ~SDCardManager() = default;

    bool begin() { return true; }
    bool ready() const { return true; }

    std::vector<String> listFiles(const char* path = "/", int maxFiles = 200) {
        return {};  // Return empty list for mock
    }

    String readFile(const char* path) {
        return String("");  // Return empty string for mock
    }

    bool readFileToStream(const char* path, Print& out, size_t chunkSize = 256) {
        return false;  // Not implemented for mock
    }

    size_t readFileToBuffer(const char* path, char* buffer, size_t bufferSize, size_t maxBytes = 0) {
        return 0;  // Not implemented for mock
    }

    bool writeFile(const char* path, const String& content) {
        return true;  // Pretend success for mock
    }

    bool ensureDirectoryExists(const char* path) {
        return true;  // Pretend success for mock
    }

    FsFile open(const char* path, const oflag_t oflag = O_RDONLY) {
        FsFile f;
        f.open(path, oflag);
        return f;
    }

    bool mkdir(const char* path, const bool pFlag = true) {
        return true;
    }

    bool exists(const char* path) {
        return sd.exists(path);
    }

    bool remove(const char* path) {
        return sd.remove(path);
    }

    bool rmdir(const char* path) {
        return true;
    }

    bool openFileForRead(const char* moduleName, const char* path, FsFile& file) {
        return file.open(path, O_READ);
    }

    bool openFileForRead(const char* moduleName, const std::string& path, FsFile& file) {
        return file.open(path.c_str(), O_READ);
    }

    bool openFileForRead(const char* moduleName, const String& path, FsFile& file) {
        return file.open(path.c_str(), O_READ);
    }

    bool openFileForWrite(const char* moduleName, const char* path, FsFile& file) {
        return file.open(path, O_WRITE | O_CREAT | O_TRUNC);
    }

    bool openFileForWrite(const char* moduleName, const std::string& path, FsFile& file) {
        return file.open(path.c_str(), O_WRITE | O_CREAT | O_TRUNC);
    }

    bool openFileForWrite(const char* moduleName, const String& path, FsFile& file) {
        return file.open(path.c_str(), O_WRITE | O_CREAT | O_TRUNC);
    }

    bool removeDir(const char* path) {
        return true;
    }

    static SDCardManager& getInstance() { return instance; }

private:
    static SDCardManager instance;
    SdFat sd;
};

// Define the static instance - will be defined in cpp
// For header-only, we use inline
inline SDCardManager SDCardManager::instance;

#define SdMan SDCardManager::getInstance()
