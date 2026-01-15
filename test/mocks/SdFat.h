#pragma once

// Mock SdFat for native testing
// Provides basic file operations using standard C++ file I/O

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>

#include "Print.h"

// ============================================================================
// FsFile mock - wraps std::fstream and inherits from Print
// ============================================================================

class FsFile : public Print {
public:
    FsFile() : isOpen_(false), position_(0) {}

    // Move constructor
    FsFile(FsFile&& other) noexcept
        : file_(std::move(other.file_)),
          memoryContent_(std::move(other.memoryContent_)),
          useMemory_(other.useMemory_),
          isOpen_(other.isOpen_),
          position_(other.position_) {
        other.isOpen_ = false;
        other.position_ = 0;
        other.useMemory_ = false;
    }

    // Move assignment
    FsFile& operator=(FsFile&& other) noexcept {
        if (this != &other) {
            close();
            file_ = std::move(other.file_);
            memoryContent_ = std::move(other.memoryContent_);
            useMemory_ = other.useMemory_;
            isOpen_ = other.isOpen_;
            position_ = other.position_;
            other.isOpen_ = false;
            other.position_ = 0;
            other.useMemory_ = false;
        }
        return *this;
    }

    // Delete copy operations
    FsFile(const FsFile&) = delete;
    FsFile& operator=(const FsFile&) = delete;

    ~FsFile() {
        close();
    }

    // Open from in-memory string (for testing)
    bool openFromString(const std::string& content) {
        close();
        memoryContent_ = content;
        useMemory_ = true;
        isOpen_ = true;
        position_ = 0;
        return true;
    }

    // Open a real file
    bool open(const char* path, int mode = 0) {
        close();
        useMemory_ = false;

        std::ios_base::openmode fmode = std::ios::binary;
        if (mode & 0x01) fmode |= std::ios::out;  // O_WRITE
        if (mode & 0x02) fmode |= std::ios::in;   // O_READ
        if (mode & 0x10) fmode |= std::ios::trunc; // O_TRUNC

        file_.open(path, fmode);
        isOpen_ = file_.is_open();
        position_ = 0;
        return isOpen_;
    }

    void close() {
        if (file_.is_open()) {
            file_.close();
        }
        memoryContent_.clear();
        useMemory_ = false;
        isOpen_ = false;
        position_ = 0;
    }

    bool isOpen() const { return isOpen_; }

    operator bool() const { return isOpen_; }

    // Read a single byte (returns -1 on EOF or error)
    int read() {
        if (!isOpen_) return -1;

        if (useMemory_) {
            if (position_ >= memoryContent_.size()) return -1;
            return static_cast<unsigned char>(memoryContent_[position_++]);
        } else {
            int c = file_.get();
            if (c != EOF) position_++;
            return c;
        }
    }

    size_t read(void* buf, size_t count) {
        if (!isOpen_) return 0;

        if (useMemory_) {
            size_t avail = memoryContent_.size() - position_;
            size_t toRead = (count < avail) ? count : avail;
            memcpy(buf, memoryContent_.data() + position_, toRead);
            position_ += toRead;
            return toRead;
        } else {
            file_.read(reinterpret_cast<char*>(buf), count);
            size_t bytesRead = file_.gcount();
            position_ += bytesRead;
            return bytesRead;
        }
    }

    // Override Print::write for single byte
    size_t write(uint8_t c) override {
        return write(&c, 1);
    }

    size_t write(const void* buf, size_t count) {
        if (!isOpen_) return 0;

        if (useMemory_) {
            const char* data = reinterpret_cast<const char*>(buf);
            if (position_ >= memoryContent_.size()) {
                memoryContent_.append(data, count);
            } else {
                memoryContent_.replace(position_, count, data, count);
            }
            position_ += count;
            return count;
        } else {
            file_.write(reinterpret_cast<const char*>(buf), count);
            position_ += count;
            return count;
        }
    }

    // Override Print::write for buffer
    size_t write(const uint8_t* buf, size_t count) override {
        return write(static_cast<const void*>(buf), count);
    }

    int available() {
        if (!isOpen_) return 0;

        if (useMemory_) {
            return memoryContent_.size() - position_;
        } else {
            auto current = file_.tellg();
            file_.seekg(0, std::ios::end);
            auto end = file_.tellg();
            file_.seekg(current);
            return static_cast<int>(end - current);
        }
    }

    size_t size() {
        if (!isOpen_) return 0;

        if (useMemory_) {
            return memoryContent_.size();
        } else {
            auto current = file_.tellg();
            file_.seekg(0, std::ios::end);
            auto fileSize = file_.tellg();
            file_.seekg(current);
            return static_cast<size_t>(fileSize);
        }
    }

    bool seek(size_t pos) {
        if (!isOpen_) return false;

        if (useMemory_) {
            if (pos <= memoryContent_.size()) {
                position_ = pos;
                return true;
            }
            return false;
        } else {
            file_.seekg(pos);
            file_.seekp(pos);
            position_ = pos;
            return !file_.fail();
        }
    }

    // Seek relative to current position
    bool seekCur(int32_t offset) {
        if (!isOpen_) return false;

        int64_t newPos = static_cast<int64_t>(position_) + offset;
        if (newPos < 0) return false;

        return seek(static_cast<size_t>(newPos));
    }

    // Seek to end
    bool seekEnd(int32_t offset = 0) {
        if (!isOpen_) return false;

        size_t fileSize = size();
        int64_t newPos = static_cast<int64_t>(fileSize) + offset;
        if (newPos < 0) return false;

        return seek(static_cast<size_t>(newPos));
    }

    size_t position() {
        return position_;
    }

    // curPosition is an alias for position
    size_t curPosition() {
        return position_;
    }

    void flush() {
        if (file_.is_open()) {
            file_.flush();
        }
    }

    // Get memory content (for testing)
    const std::string& getMemoryContent() const { return memoryContent_; }

private:
    std::fstream file_;
    std::string memoryContent_;
    bool useMemory_ = false;
    bool isOpen_ = false;
    size_t position_ = 0;
};

// ============================================================================
// SdFat mock - basic file system operations
// ============================================================================

class SdFat {
public:
    bool begin(uint8_t csPin = 0, uint32_t spiSpeed = 0) {
        return true;  // Always succeeds in mock
    }

    bool exists(const char* path) {
        std::ifstream f(path);
        return f.good();
    }

    bool mkdir(const char* path, bool createParents = true) {
        // No-op for mock - could use std::filesystem if needed
        return true;
    }

    bool remove(const char* path) {
        return std::remove(path) == 0;
    }

    bool rename(const char* oldPath, const char* newPath) {
        return std::rename(oldPath, newPath) == 0;
    }
};

// Global instance
extern SdFat SD;

// ============================================================================
// File open modes (Arduino SD library style)
// ============================================================================

#define O_READ    0x02
#define O_WRITE   0x01
#define O_RDWR    0x03
#define O_APPEND  0x04
#define O_CREAT   0x08
#define O_TRUNC   0x10
#define O_EXCL    0x20

#define FILE_READ   O_READ
#define FILE_WRITE  (O_READ | O_WRITE | O_CREAT)
