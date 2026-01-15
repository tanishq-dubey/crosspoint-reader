#pragma once

// Test font for unit testing
// Provides a simple monospace-style font where each character is a fixed width

#include <cstdint>
#include <cstring>

// Include the actual font data structures from the library
#include <EpdFontData.h>

// Globally defined test glyphs - one for each ASCII character 32-126
// All characters have the same dimensions for predictable testing
static EpdGlyph makeTestGlyph(uint8_t advanceX = 10) {
    EpdGlyph g = {};
    g.width = 8;
    g.height = 16;
    g.advanceX = advanceX;
    g.left = 0;
    g.top = 14;
    g.dataLength = 0;
    g.dataOffset = 0;
    return g;
}

// Test font helper class for creating and managing test fonts
class TestFontHelper {
public:
    // Fixed width for all characters (monospace-style for testing)
    static constexpr int CHAR_WIDTH = 10;
    static constexpr int CHAR_HEIGHT = 16;
    static constexpr int LINE_HEIGHT = 20;
    static constexpr int SPACE_WIDTH = 10;
    static constexpr int ASCENDER = 14;

    static int getTextWidth(const char* text) {
        int width = 0;
        while (*text) {
            // Handle UTF-8 multi-byte sequences
            uint8_t c = (uint8_t)*text;
            if ((c & 0x80) == 0) {
                // ASCII character
                width += CHAR_WIDTH;
                text++;
            } else if ((c & 0xE0) == 0xC0) {
                // 2-byte UTF-8
                width += CHAR_WIDTH;
                text += 2;
            } else if ((c & 0xF0) == 0xE0) {
                // 3-byte UTF-8
                width += CHAR_WIDTH;
                text += 3;
            } else if ((c & 0xF8) == 0xF0) {
                // 4-byte UTF-8
                width += CHAR_WIDTH;
                text += 4;
            } else {
                // Invalid UTF-8, skip
                text++;
            }
        }
        return width;
    }

    // Get number of UTF-8 characters (not bytes) in a string
    static int getCharCount(const char* text) {
        int count = 0;
        while (*text) {
            uint8_t c = (uint8_t)*text;
            if ((c & 0x80) == 0) {
                text++;
            } else if ((c & 0xE0) == 0xC0) {
                text += 2;
            } else if ((c & 0xF0) == 0xE0) {
                text += 3;
            } else if ((c & 0xF8) == 0xF0) {
                text += 4;
            } else {
                text++;
            }
            count++;
        }
        return count;
    }
};
