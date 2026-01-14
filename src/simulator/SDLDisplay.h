#pragma once

// SDLDisplay - SDL2-based display that provides EInkDisplay-compatible interface
// Used for visual testing of EPUB rendering without physical hardware

#include <SDL2/SDL.h>
#include <cstdint>
#include <cstring>

class SDLDisplay {
public:
    // Refresh modes (compatible with EInkDisplay)
    enum RefreshMode {
        FULL_REFRESH,
        HALF_REFRESH,
        FAST_REFRESH
    };

    // Display dimensions (same as real e-ink hardware)
    static constexpr uint16_t DISPLAY_WIDTH = 800;
    static constexpr uint16_t DISPLAY_HEIGHT = 480;
    static constexpr uint16_t DISPLAY_WIDTH_BYTES = DISPLAY_WIDTH / 8;
    static constexpr uint32_t BUFFER_SIZE = DISPLAY_WIDTH_BYTES * DISPLAY_HEIGHT;

    // Window scaling factor for better visibility on desktop
    static constexpr int SCALE_FACTOR = 1;

    SDLDisplay() {
        frameBuffer = frameBuffer0;
        clearScreen(0xFF);  // Initialize to white
    }

    ~SDLDisplay() {
        shutdown();
    }

    // Initialize SDL window and renderer
    bool begin() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            printf("SDL_Init failed: %s\n", SDL_GetError());
            return false;
        }

        window = SDL_CreateWindow(
            "EPUB Reader Simulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            DISPLAY_WIDTH * SCALE_FACTOR,
            DISPLAY_HEIGHT * SCALE_FACTOR,
            SDL_WINDOW_SHOWN
        );

        if (!window) {
            printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
            return false;
        }

        // Create texture for pixel-perfect rendering
        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGB24,
            SDL_TEXTUREACCESS_STREAMING,
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT
        );

        if (!texture) {
            printf("SDL_CreateTexture failed: %s\n", SDL_GetError());
            return false;
        }

        initialized = true;
        return true;
    }

    void shutdown() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        if (initialized) {
            SDL_Quit();
            initialized = false;
        }
    }

    // Clear screen to specified color (0x00 = black, 0xFF = white)
    void clearScreen(uint8_t color = 0xFF) {
        memset(frameBuffer, color, BUFFER_SIZE);
    }

    // Draw image data to framebuffer
    void drawImage(const uint8_t* imageData, uint16_t x, uint16_t y,
                   uint16_t w, uint16_t h, bool fromProgmem = false) {
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

    // Set entire framebuffer from external buffer
    void setFramebuffer(const uint8_t* bwBuffer) {
        memcpy(frameBuffer, bwBuffer, BUFFER_SIZE);
    }

    // Grayscale operations (no-op for basic simulator)
    void copyGrayscaleBuffers(const uint8_t* lsbBuffer, const uint8_t* msbBuffer) {}
    void copyGrayscaleLsbBuffers(const uint8_t* lsbBuffer) {}
    void copyGrayscaleMsbBuffers(const uint8_t* msbBuffer) {}
    void cleanupGrayscaleBuffers(const uint8_t* bwBuffer) {}

    // Display the framebuffer to SDL window
    void displayBuffer(RefreshMode mode = FAST_REFRESH) {
        if (!initialized || !texture) return;

        // Convert 1-bit framebuffer to RGB24 texture
        uint8_t* pixels;
        int pitch;

        if (SDL_LockTexture(texture, nullptr, (void**)&pixels, &pitch) != 0) {
            printf("SDL_LockTexture failed: %s\n", SDL_GetError());
            return;
        }

        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < DISPLAY_WIDTH; x++) {
                uint32_t byteIdx = y * DISPLAY_WIDTH_BYTES + x / 8;
                uint8_t bitIdx = 7 - (x % 8);
                bool isWhite = (frameBuffer[byteIdx] >> bitIdx) & 1;

                uint8_t color = isWhite ? 255 : 0;  // White or black
                int pixelIdx = y * pitch + x * 3;
                pixels[pixelIdx + 0] = color;  // R
                pixels[pixelIdx + 1] = color;  // G
                pixels[pixelIdx + 2] = color;  // B
            }
        }

        SDL_UnlockTexture(texture);

        // Render texture to screen
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        displayCount++;
    }

    void displayWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        // For simulator, just do a full refresh
        displayBuffer();
    }

    void displayGrayBuffer(bool turnOffScreen = false) {
        displayBuffer();
    }

    void refreshDisplay(RefreshMode mode = FAST_REFRESH, bool turnOffScreen = false) {
        displayBuffer(mode);
    }

    void grayscaleRevert() {}
    void setCustomLUT(bool enabled, const unsigned char* lutData = nullptr) {}
    void deepSleep() {}

    // Access to frame buffer
    uint8_t* getFrameBuffer() const {
        return frameBuffer;
    }

    // ========================================================================
    // SDL-specific methods
    // ========================================================================

    // Process SDL events, returns false if quit requested
    bool processEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
            if (event.type == SDL_KEYDOWN) {
                lastKeyPressed = event.key.keysym.sym;
            }
        }
        return true;
    }

    // Get last key pressed (and clear it)
    SDL_Keycode getLastKey() {
        SDL_Keycode key = lastKeyPressed;
        lastKeyPressed = SDLK_UNKNOWN;
        return key;
    }

    // Wait for a specific amount of time while processing events
    bool waitWithEvents(int milliseconds) {
        Uint32 startTime = SDL_GetTicks();
        while (SDL_GetTicks() - startTime < (Uint32)milliseconds) {
            if (!processEvents()) return false;
            SDL_Delay(10);
        }
        return true;
    }

    // Get display count (for debugging)
    int getDisplayCount() const { return displayCount; }

    // Save current framebuffer as BMP file
    bool saveScreenshot(const char* filename) {
        if (!initialized || !renderer) return false;

        SDL_Surface* surface = SDL_CreateRGBSurface(
            0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 24,
            0x0000FF, 0x00FF00, 0xFF0000, 0
        );

        if (!surface) return false;

        // Copy framebuffer to surface
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            for (int x = 0; x < DISPLAY_WIDTH; x++) {
                uint32_t byteIdx = y * DISPLAY_WIDTH_BYTES + x / 8;
                uint8_t bitIdx = 7 - (x % 8);
                bool isWhite = (frameBuffer[byteIdx] >> bitIdx) & 1;

                uint8_t color = isWhite ? 255 : 0;
                uint8_t* pixel = (uint8_t*)surface->pixels + y * surface->pitch + x * 3;
                pixel[0] = color;  // B
                pixel[1] = color;  // G
                pixel[2] = color;  // R
            }
        }

        int result = SDL_SaveBMP(surface, filename);
        SDL_FreeSurface(surface);

        return result == 0;
    }

private:
    uint8_t frameBuffer0[BUFFER_SIZE];
    uint8_t* frameBuffer = nullptr;
    int displayCount = 0;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    bool initialized = false;
    SDL_Keycode lastKeyPressed = SDLK_UNKNOWN;
};
