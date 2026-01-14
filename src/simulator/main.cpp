// EPUB Reader Simulator
// SDL-based visual simulator for testing HTML rendering

#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>

#include "SDLDisplay.h"

// For drawing, we'll create simple primitives directly
// since we can't easily use the full GfxRenderer without all its dependencies

// Simple text drawing (placeholder - just draws dots for now)
void drawTestPattern(SDLDisplay& display) {
    uint8_t* fb = display.getFrameBuffer();

    // Clear to white
    display.clearScreen(0xFF);

    // Draw a border
    for (int x = 0; x < SDLDisplay::DISPLAY_WIDTH; x++) {
        // Top border
        int byteIdx = 0 * SDLDisplay::DISPLAY_WIDTH_BYTES + x / 8;
        int bitIdx = 7 - (x % 8);
        fb[byteIdx] &= ~(1 << bitIdx);

        // Bottom border
        byteIdx = (SDLDisplay::DISPLAY_HEIGHT - 1) * SDLDisplay::DISPLAY_WIDTH_BYTES + x / 8;
        fb[byteIdx] &= ~(1 << bitIdx);
    }

    for (int y = 0; y < SDLDisplay::DISPLAY_HEIGHT; y++) {
        // Left border
        int byteIdx = y * SDLDisplay::DISPLAY_WIDTH_BYTES + 0;
        fb[byteIdx] &= ~(1 << 7);

        // Right border
        byteIdx = y * SDLDisplay::DISPLAY_WIDTH_BYTES + (SDLDisplay::DISPLAY_WIDTH - 1) / 8;
        int bitIdx = 7 - ((SDLDisplay::DISPLAY_WIDTH - 1) % 8);
        fb[byteIdx] &= ~(1 << bitIdx);
    }

    // Draw a sample horizontal rule in the middle
    int ruleY = SDLDisplay::DISPLAY_HEIGHT / 2;
    int ruleStartX = 50;
    int ruleEndX = SDLDisplay::DISPLAY_WIDTH - 50;
    for (int x = ruleStartX; x < ruleEndX; x++) {
        int byteIdx = ruleY * SDLDisplay::DISPLAY_WIDTH_BYTES + x / 8;
        int bitIdx = 7 - (x % 8);
        fb[byteIdx] &= ~(1 << bitIdx);
    }

    // Draw some text placeholders (vertical bars representing list bullets)
    int textStartY = 100;
    int lineHeight = 30;

    for (int line = 0; line < 5; line++) {
        int y = textStartY + line * lineHeight;

        // Draw bullet (small filled rectangle)
        int bulletX = 50;
        for (int by = y; by < y + 8; by++) {
            for (int bx = bulletX; bx < bulletX + 8; bx++) {
                int byteIdx = by * SDLDisplay::DISPLAY_WIDTH_BYTES + bx / 8;
                int bitIdx = 7 - (bx % 8);
                fb[byteIdx] &= ~(1 << bitIdx);
            }
        }

        // Draw "text line" (horizontal bar)
        for (int x = 70; x < 400; x++) {
            int byteIdx = (y + 4) * SDLDisplay::DISPLAY_WIDTH_BYTES + x / 8;
            int bitIdx = 7 - (x % 8);
            fb[byteIdx] &= ~(1 << bitIdx);
        }
    }

    // Draw blockquote indicator (vertical line on left)
    int quoteStartY = 350;
    int quoteEndY = 450;
    int quoteLineX = 45;
    for (int y = quoteStartY; y < quoteEndY; y++) {
        for (int x = quoteLineX; x < quoteLineX + 3; x++) {
            int byteIdx = y * SDLDisplay::DISPLAY_WIDTH_BYTES + x / 8;
            int bitIdx = 7 - (x % 8);
            fb[byteIdx] &= ~(1 << bitIdx);
        }
    }
}

// Read HTML file content
std::string readHtmlFile(const char* filename) {
    std::ifstream file(filename);
    if (!file) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printUsage(const char* progName) {
    printf("EPUB Reader Simulator\n");
    printf("Usage: %s [html_file]\n\n", progName);
    printf("If no file is specified, displays a test pattern.\n\n");
    printf("Controls:\n");
    printf("  Left/Right arrow - Page navigation\n");
    printf("  S                - Save screenshot\n");
    printf("  Q or Escape      - Quit\n");
}

int main(int argc, char* argv[]) {
    SDLDisplay display;

    if (!display.begin()) {
        printf("Failed to initialize SDL display\n");
        return 1;
    }

    printf("EPUB Reader Simulator started\n");
    printf("Press Q or Escape to quit\n");

    if (argc > 1) {
        // Load specified HTML file
        std::string html = readHtmlFile(argv[1]);
        if (html.empty()) {
            printf("Failed to read file: %s\n", argv[1]);
            // Fall through to show test pattern
        } else {
            printf("Loaded: %s (%zu bytes)\n", argv[1], html.length());
            // TODO: Parse HTML and render pages
            // For now, just show the content in console
            printf("HTML Content preview:\n%.200s...\n", html.c_str());
        }
    }

    // Draw test pattern
    drawTestPattern(display);
    display.displayBuffer();

    int pageNumber = 1;
    bool running = true;

    while (running) {
        if (!display.processEvents()) {
            break;
        }

        SDL_Keycode key = display.getLastKey();
        switch (key) {
            case SDLK_q:
            case SDLK_ESCAPE:
                running = false;
                break;

            case SDLK_LEFT:
                if (pageNumber > 1) {
                    pageNumber--;
                    printf("Page %d\n", pageNumber);
                    // TODO: Render previous page
                    display.displayBuffer();
                }
                break;

            case SDLK_RIGHT:
                pageNumber++;
                printf("Page %d\n", pageNumber);
                // TODO: Render next page
                display.displayBuffer();
                break;

            case SDLK_s:
                {
                    char filename[64];
                    snprintf(filename, sizeof(filename), "screenshot_%03d.bmp", pageNumber);
                    if (display.saveScreenshot(filename)) {
                        printf("Saved screenshot: %s\n", filename);
                    } else {
                        printf("Failed to save screenshot\n");
                    }
                }
                break;

            default:
                break;
        }

        SDL_Delay(16);  // ~60 FPS
    }

    printf("Simulator ended\n");
    return 0;
}
