# AGENTS.md - CrossPoint Reader

Guidelines for AI coding agents working on the CrossPoint Reader codebase.

## Project Overview

CrossPoint is an ESP32-C3 embedded firmware for an e-paper reader using the Xteink X4 display. Built with PlatformIO, Arduino framework, and C++20.

**Key constraints:**
- ESP32-C3 has ~380KB usable RAM - memory efficiency is critical
- Target hardware: 480x800 e-paper display
- Heavy use of SD card for caching/storage

## Build Commands

```bash
pio run                      # Build default (dev) version
pio run -e gh_release        # Build release version
pio run --target upload      # Flash to device

# Static analysis
pio check --fail-on-defect low --fail-on-defect medium --fail-on-defect high

# Format code (run before committing)
./bin/clang-format-fix       # All tracked C/C++ files
./bin/clang-format-fix -g    # Only modified files (faster)
```

## Testing

No unit tests. Testing requires flashing to the physical device.

## Code Style

Formatting enforced by clang-format. Run `./bin/clang-format-fix` before committing.

### Formatting Rules

- **Indent**: 2 spaces (no tabs)
- **Line length**: 120 characters max
- **Braces**: Attach style (same line as control statement)
- **Pointer alignment**: Left (`int* ptr`, not `int *ptr`)
- **Short statements**: Single-line if/loops allowed (without else)
- **Header guards**: Use `#pragma once`

### Include Order (auto-sorted by clang-format)

```cpp
#include "CorrespondingHeader.h"  // 1. Corresponding header (for .cpp)
#include <Arduino.h>              // 2. System/library headers
#include <cstring>                // 3. C++ standard library
#include "CrossPointSettings.h"   // 4. Project headers (quotes)
```

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase | `CrossPointSettings`, `EpubReaderActivity` |
| Methods/Members | camelCase | `loadFromFile()`, `currentSpineIndex` |
| Constants/Macros | UPPER_SNAKE_CASE | `SETTINGS_FILE_VERSION` |
| Enum values | UPPER_SNAKE_CASE | `PORTRAIT`, `SLEEP_10_MIN` |
| File names | PascalCase | `CrossPointSettings.cpp` |

### File-Local Constants

```cpp
namespace {
constexpr uint8_t SETTINGS_FILE_VERSION = 1;
constexpr char SETTINGS_FILE[] = "/.crosspoint/settings.bin";
}  // namespace
```

### Singleton Pattern

```cpp
class CrossPointSettings {
  static CrossPointSettings instance;
public:
  static CrossPointSettings& getInstance() { return instance; }
};
#define SETTINGS CrossPointSettings::getInstance()
```

## Error Handling

**Return bool for success/failure:**
```cpp
bool saveToFile() const;
bool loadFromFile();
```

**Enum-based error codes for complex operations:**
```cpp
enum DownloadError { OK = 0, HTTP_ERROR, FILE_ERROR, ABORTED };
```

**Serial logging with timestamps:**
```cpp
Serial.printf("[%lu] [TAG] Message: %s\n", millis(), value.c_str());
```
Common tags: `[CPS]` (Settings), `[ERS]` (EpubReader), etc.

**Early return on invalid state:**
```cpp
if (!epub) return;
```

**Thread safety with FreeRTOS mutexes:**
```cpp
xSemaphoreTake(renderingMutex, portMAX_DELAY);
renderScreen();
xSemaphoreGive(renderingMutex);
```

## Architecture

### Activity Pattern (UI State Machine)

- Base class: `Activity` with `onEnter()`, `onExit()`, `loop()`
- Nested screens: `ActivityWithSubactivity`
- Navigation via `std::function<void()>` callbacks

### Directory Structure

```
src/
├── main.cpp                 # Entry point
├── CrossPointSettings.h/cpp # Global settings singleton
├── CrossPointState.h/cpp    # Global app state singleton
├── activities/              # UI screens (boot_sleep, browser, home, network, reader, settings)
├── network/                 # Network utilities
└── util/                    # Utility functions

lib/                         # Libraries (Epub, EpdFont, GfxRenderer, Serialization, etc.)
open-x4-sdk/                 # Hardware SDK (git submodule)
```

## CI Pipeline

Runs on every push/PR:
1. cppcheck static analysis
2. clang-format verification  
3. Build verification

## Common Gotchas

1. **Memory**: ESP32-C3 has limited RAM. Avoid large allocations; use SD card for caching.
2. **Submodules**: Run `git submodule update --init --recursive` after cloning.
3. **Formatting**: CI will fail if code is not formatted. Run `./bin/clang-format-fix`.
4. **Generated files**: Don't edit `lib/EpdFont/builtinFonts/` - they're script-generated.
