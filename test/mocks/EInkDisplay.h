#pragma once

// Mock EInkDisplay for native testing
// Provides an in-memory framebuffer without actual hardware

#include <cstdint>
#include <cstring>
#include <fstream>

class EInkDisplay {
public:
    // Refresh modes
    enum RefreshMode {
        FULL_REFRESH,
        HALF_REFRESH,
        FAST_REFRESH
    };

    // Display dimensions (same as real hardware)
    static constexpr uint16_t DISPLAY_WIDTH = 800;
    static constexpr uint16_t DISPLAY_HEIGHT = 480;
    static constexpr uint16_t DISPLAY_WIDTH_BYTES = DISPLAY_WIDTH / 8;
    static constexpr uint32_t BUFFER_SIZE = DISPLAY_WIDTH_BYTES * DISPLAY_HEIGHT;

    // Constructor - ignores pin configuration for mock
    EInkDisplay(int8_t sclk = -1, int8_t mosi = -1, int8_t cs = -1,
                int8_t dc = -1, int8_t rst = -1, int8_t busy = -1) {
        frameBuffer = frameBuffer0;
        clearScreen(0xFF);  // Initialize to white
    }

    ~EInkDisplay() = default;

    // Initialize - no-op for mock
    void begin() {}

    // Frame buffer operations
    void clearScreen(uint8_t color = 0xFF) {
        memset(frameBuffer, color, BUFFER_SIZE);
    }

    void drawImage(const uint8_t* imageData, uint16_t x, uint16_t y,
                   uint16_t w, uint16_t h, bool fromProgmem = false) {
        // Copy image data to framebuffer at specified position
        for (uint16_t row = 0; row < h && (y + row) < DISPLAY_HEIGHT; row++) {
            for (uint16_t col = 0; col < w && (x + col) < DISPLAY_WIDTH; col++) {
                uint16_t srcByteIdx = (row * w + col) / 8;
                uint8_t srcBitIdx = 7 - ((row * w + col) % 8);
                uint8_t srcBit = (imageData[srcByteIdx] >> srcBitIdx) & 1;

                uint16_t dstX = x + col;
                uint16_t dstY = y + row;
                uint32_t dstByteIdx = dstY * DISPLAY_WIDTH_BYTES + dstX / 8;
                uint8_t dstBitIdx = 7 - (dstX % 8);

                if (srcBit) {
                    frameBuffer[dstByteIdx] |= (1 << dstBitIdx);
                } else {
                    frameBuffer[dstByteIdx] &= ~(1 << dstBitIdx);
                }
            }
        }
    }

    void setFramebuffer(const uint8_t* bwBuffer) {
        memcpy(frameBuffer, bwBuffer, BUFFER_SIZE);
    }

    // Grayscale operations (no-op for basic mock)
    void copyGrayscaleBuffers(const uint8_t* lsbBuffer, const uint8_t* msbBuffer) {}
    void copyGrayscaleLsbBuffers(const uint8_t* lsbBuffer) {}
    void copyGrayscaleMsbBuffers(const uint8_t* msbBuffer) {}
    void cleanupGrayscaleBuffers(const uint8_t* bwBuffer) {}

    // Display operations (no-op for mock - we just keep the buffer)
    void displayBuffer(RefreshMode mode = FAST_REFRESH) {
        displayCount++;
    }

    void displayWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        displayCount++;
    }

    void displayGrayBuffer(bool turnOffScreen = false) {
        displayCount++;
    }

    void refreshDisplay(RefreshMode mode = FAST_REFRESH, bool turnOffScreen = false) {
        displayCount++;
    }

    void grayscaleRevert() {}
    void setCustomLUT(bool enabled, const unsigned char* lutData = nullptr) {}
    void deepSleep() {}

    // Access to frame buffer
    uint8_t* getFrameBuffer() const {
        return frameBuffer;
    }

    // ========================================================================
    // Test helper methods
    // ========================================================================

    // Get pixel at x,y (returns true if black, false if white)
    bool getPixel(int x, int y) const {
        if (x < 0 || x >= DISPLAY_WIDTH || y < 0 || y >= DISPLAY_HEIGHT) {
            return false;
        }
        uint32_t byteIdx = y * DISPLAY_WIDTH_BYTES + x / 8;
        uint8_t bitIdx = 7 - (x % 8);
        return !((frameBuffer[byteIdx] >> bitIdx) & 1);  // 0 = black, 1 = white
    }

    // Set pixel at x,y (true = black, false = white)
    void setPixel(int x, int y, bool black) {
        if (x < 0 || x >= DISPLAY_WIDTH || y < 0 || y >= DISPLAY_HEIGHT) {
            return;
        }
        uint32_t byteIdx = y * DISPLAY_WIDTH_BYTES + x / 8;
        uint8_t bitIdx = 7 - (x % 8);
        if (black) {
            frameBuffer[byteIdx] &= ~(1 << bitIdx);  // 0 = black
        } else {
            frameBuffer[byteIdx] |= (1 << bitIdx);   // 1 = white
        }
    }

    // Count number of black pixels in a region
    int countBlackPixels(int x, int y, int w, int h) const {
        int count = 0;
        for (int py = y; py < y + h && py < DISPLAY_HEIGHT; py++) {
            for (int px = x; px < x + w && px < DISPLAY_WIDTH; px++) {
                if (getPixel(px, py)) count++;
            }
        }
        return count;
    }

    // Check if a horizontal line exists at y between x1 and x2
    bool hasHorizontalLine(int y, int x1, int x2, int minBlackPixels = -1) const {
        if (minBlackPixels < 0) minBlackPixels = (x2 - x1) * 80 / 100;  // Default: 80% coverage
        int blackCount = 0;
        for (int x = x1; x <= x2 && x < DISPLAY_WIDTH; x++) {
            if (getPixel(x, y)) blackCount++;
        }
        return blackCount >= minBlackPixels;
    }

    // Check if a vertical line exists at x between y1 and y2
    bool hasVerticalLine(int x, int y1, int y2, int minBlackPixels = -1) const {
        if (minBlackPixels < 0) minBlackPixels = (y2 - y1) * 80 / 100;  // Default: 80% coverage
        int blackCount = 0;
        for (int y = y1; y <= y2 && y < DISPLAY_HEIGHT; y++) {
            if (getPixel(x, y)) blackCount++;
        }
        return blackCount >= minBlackPixels;
    }

    // Save framebuffer to PBM file (for visual inspection)
    void saveFrameBufferAsPBM(const char* filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return;

        // PBM header
        file << "P4\n" << DISPLAY_WIDTH << " " << DISPLAY_HEIGHT << "\n";

        // PBM uses 1=black, 0=white (opposite of our framebuffer)
        // So we need to invert
        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
            file.put(~frameBuffer[i]);
        }
    }

    // Save framebuffer to PPM file (grayscale, easier to view)
    void saveFrameBufferAsPPM(const char* filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return;

        // PPM header (grayscale as RGB)
        file << "P6\n" << DISPLAY_WIDTH << " " << DISPLAY_HEIGHT << "\n255\n";

        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < DISPLAY_WIDTH; x++) {
                uint8_t val = getPixel(x, y) ? 0 : 255;  // black or white
                file.put(val);
                file.put(val);
                file.put(val);
            }
        }
    }

    // Get count of display refreshes (for testing)
    int getDisplayCount() const { return displayCount; }
    void resetDisplayCount() { displayCount = 0; }

private:
    uint8_t frameBuffer0[BUFFER_SIZE];
    uint8_t* frameBuffer;
    int displayCount = 0;
};
