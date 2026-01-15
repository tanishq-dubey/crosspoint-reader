#pragma once

// Mock Arduino.h for native testing
// Provides stubs for common Arduino functions used in the codebase

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <string>

// ============================================================================
// Type definitions
// ============================================================================

typedef uint8_t byte;
typedef bool boolean;

// Pin modes
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Pin states
#define LOW 0
#define HIGH 1

// ============================================================================
// Timing functions
// ============================================================================

inline unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

inline unsigned long micros() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
}

inline void delay(unsigned long ms) {
    // No-op for tests - we don't want actual delays
}

inline void delayMicroseconds(unsigned int us) {
    // No-op for tests
}

// ============================================================================
// GPIO functions
// ============================================================================

inline void pinMode(uint8_t pin, uint8_t mode) {
    // No-op for tests
}

inline void digitalWrite(uint8_t pin, uint8_t val) {
    // No-op for tests
}

inline int digitalRead(uint8_t pin) {
    return LOW;  // Default to LOW
}

inline int analogRead(uint8_t pin) {
    return 0;
}

inline void analogWrite(uint8_t pin, int val) {
    // No-op for tests
}

// ============================================================================
// Serial mock
// ============================================================================

class SerialMock {
public:
    void begin(unsigned long baud) {}
    void end() {}

    size_t print(const char* str) {
        printf("%s", str);
        return strlen(str);
    }

    size_t print(int val) {
        return printf("%d", val);
    }

    size_t print(unsigned int val) {
        return printf("%u", val);
    }

    size_t print(long val) {
        return printf("%ld", val);
    }

    size_t print(unsigned long val) {
        return printf("%lu", val);
    }

    size_t print(double val, int digits = 2) {
        return printf("%.*f", digits, val);
    }

    size_t println() {
        printf("\n");
        return 1;
    }

    size_t println(const char* str) {
        printf("%s\n", str);
        return strlen(str) + 1;
    }

    size_t println(int val) {
        return printf("%d\n", val);
    }

    size_t println(unsigned int val) {
        return printf("%u\n", val);
    }

    size_t println(unsigned long val) {
        return printf("%lu\n", val);
    }

    template<typename... Args>
    size_t printf(const char* format, Args... args) {
        return ::printf(format, args...);
    }

    int available() { return 0; }
    int read() { return -1; }
    int peek() { return -1; }
    void flush() {}

    operator bool() { return true; }
};

extern SerialMock Serial;

// ============================================================================
// HardwareSerial compatibility
// ============================================================================

class HardwareSerial : public SerialMock {
public:
    HardwareSerial(int uart_nr) {}
};

// ============================================================================
// String class (minimal implementation)
// ============================================================================

class String : public std::string {
public:
    String() : std::string() {}
    String(const char* str) : std::string(str) {}
    String(const std::string& str) : std::string(str) {}
    String(int val) : std::string(std::to_string(val)) {}
    String(unsigned int val) : std::string(std::to_string(val)) {}
    String(long val) : std::string(std::to_string(val)) {}
    String(unsigned long val) : std::string(std::to_string(val)) {}
    String(float val, int decimalPlaces = 2) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.*f", decimalPlaces, val);
        assign(buf);
    }

    const char* c_str() const { return std::string::c_str(); }
    unsigned int length() const { return std::string::length(); }

    String substring(unsigned int beginIndex) const {
        return String(std::string::substr(beginIndex));
    }

    String substring(unsigned int beginIndex, unsigned int endIndex) const {
        return String(std::string::substr(beginIndex, endIndex - beginIndex));
    }

    int indexOf(char ch) const {
        size_t pos = find(ch);
        return pos == npos ? -1 : (int)pos;
    }

    int indexOf(const char* str) const {
        size_t pos = find(str);
        return pos == npos ? -1 : (int)pos;
    }

    bool startsWith(const char* prefix) const {
        return find(prefix) == 0;
    }

    bool endsWith(const char* suffix) const {
        size_t suffixLen = strlen(suffix);
        if (suffixLen > length()) return false;
        return compare(length() - suffixLen, suffixLen, suffix) == 0;
    }

    void toLowerCase() {
        for (char& c : *this) {
            c = tolower(c);
        }
    }

    void toUpperCase() {
        for (char& c : *this) {
            c = toupper(c);
        }
    }

    long toInt() const {
        return std::stol(*this);
    }

    float toFloat() const {
        return std::stof(*this);
    }
};

// ============================================================================
// PROGMEM compatibility (no-op on native)
// ============================================================================

#define PROGMEM
#define PGM_P const char*
#define PSTR(s) (s)

inline uint8_t pgm_read_byte(const void* addr) {
    return *reinterpret_cast<const uint8_t*>(addr);
}

inline uint16_t pgm_read_word(const void* addr) {
    return *reinterpret_cast<const uint16_t*>(addr);
}

inline uint32_t pgm_read_dword(const void* addr) {
    return *reinterpret_cast<const uint32_t*>(addr);
}

inline void* pgm_read_ptr(const void* addr) {
    return *reinterpret_cast<void* const*>(addr);
}

#define pgm_read_byte_near(addr) pgm_read_byte(addr)
#define pgm_read_word_near(addr) pgm_read_word(addr)
#define pgm_read_dword_near(addr) pgm_read_dword(addr)

// ============================================================================
// Utility macros
// ============================================================================

// Note: Do NOT define min/max as macros - they conflict with std::min/std::max
// in C++ standard library headers. Arduino code should use _min/_max or
// std::min/std::max instead.
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// Provide underscore versions that don't conflict
#ifndef _min
#define _min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef _max
#define _max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef constrain
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#endif

// Note: Do NOT define abs, round, sq as macros - they conflict with std:: functions
// in C++ standard library headers. Arduino code should use std::abs, std::round, etc.
// or use inline template functions below.

#ifndef sq
#define sq(x) ((x)*(x))
#endif

// Use std::abs and std::round from <cmath> instead of macros

#ifndef radians
#define radians(deg) ((deg)*DEG_TO_RAD)
#endif

#ifndef degrees
#define degrees(rad) ((rad)*RAD_TO_DEG)
#endif

#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

// ============================================================================
// Bit manipulation
// ============================================================================

#define bit(b) (1UL << (b))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))
