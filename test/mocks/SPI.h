#pragma once

// Mock SPI.h for native testing
// Provides no-op implementations

#include <cstdint>
#include <cstddef>

// SPI modes
#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

// Bit order
#define MSBFIRST 1
#define LSBFIRST 0

class SPISettings {
public:
    SPISettings() : clock_(1000000), bitOrder_(MSBFIRST), dataMode_(SPI_MODE0) {}
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
        : clock_(clock), bitOrder_(bitOrder), dataMode_(dataMode) {}

private:
    uint32_t clock_;
    uint8_t bitOrder_;
    uint8_t dataMode_;
};

class SPIClass {
public:
    void begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1) {}
    void end() {}

    void beginTransaction(SPISettings settings) {}
    void endTransaction() {}

    uint8_t transfer(uint8_t data) { return 0; }
    uint16_t transfer16(uint16_t data) { return 0; }
    uint32_t transfer32(uint32_t data) { return 0; }

    void transfer(void* buf, size_t count) {}
    void transferBytes(const uint8_t* data, uint8_t* out, uint32_t size) {}
    void transferBits(uint32_t data, uint32_t* out, uint8_t bits) {}

    void writeBytes(const uint8_t* data, uint32_t size) {}
    void writePixels(const void* data, uint32_t size) {}

    void setBitOrder(uint8_t bitOrder) {}
    void setDataMode(uint8_t dataMode) {}
    void setFrequency(uint32_t freq) {}
};

extern SPIClass SPI;
