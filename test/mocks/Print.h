#pragma once

// Mock Print.h for native builds
// Provides the Print base class used by Serial and other output streams

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

class Print {
public:
    virtual ~Print() = default;

    virtual size_t write(uint8_t c) {
        return putchar(c) == EOF ? 0 : 1;
    }

    virtual size_t write(const uint8_t *buffer, size_t size) {
        size_t n = 0;
        while (size--) {
            if (write(*buffer++)) n++;
            else break;
        }
        return n;
    }

    size_t write(const char *str) {
        if (str == nullptr) return 0;
        return write((const uint8_t *)str, strlen(str));
    }

    size_t write(const char *buffer, size_t size) {
        return write((const uint8_t *)buffer, size);
    }

    // Print interface
    size_t print(const char* str) {
        return write(str);
    }

    size_t print(char c) {
        return write(c);
    }

    size_t print(int n) {
        return printf("%d", n);
    }

    size_t print(unsigned int n) {
        return printf("%u", n);
    }

    size_t print(long n) {
        return printf("%ld", n);
    }

    size_t print(unsigned long n) {
        return printf("%lu", n);
    }

    size_t println() {
        return write('\n');
    }

    size_t println(const char* str) {
        size_t n = print(str);
        n += println();
        return n;
    }

    size_t println(int n) {
        size_t r = print(n);
        r += println();
        return r;
    }
};
