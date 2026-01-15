# SignalOS

**A DWS (Dubey Web Services) Project**
*"It's your internet, take it back"*

---

SignalOS is open-source firmware for the **Xteink X4** e-paper reader, built on the excellent foundation of [CrossPoint Reader](https://github.com/daveallie/crosspoint-reader) by Dave Allie. We're continuing that work with enhanced HTML rendering, a testing infrastructure, and our own take on what an open e-reader experience should be.

![](./docs/images/cover.jpg)

## Why SignalOS?

E-paper devices are perfect for reading. Low power, easy on the eyes, and distraction-free. But most e-readers are locked down, treating you as a guest on hardware you own.

The Xteink X4 is affordable and capable, but ships with closed firmware. CrossPoint Reader proved that a fully open alternative was possible. SignalOS picks up that torch and runs with it.

**Our goals:**
- Keep it open. Every line of code, every decision, out in the open.
- Make EPUBs look good. Lists with real bullets, tables that make sense, blockquotes that stand out.
- Stay lean. The ESP32-C3 has ~380KB of RAM. We respect that constraint.
- Enable development without hardware. Test your changes before flashing.

## What's New in SignalOS

Building on CrossPoint Reader's solid EPUB parsing foundation, we've added:

**Rich HTML Rendering**
- Ordered and unordered lists with proper bullets (•) and numbering
- Blockquotes with Kindle-style left indent and vertical line
- Tables rendered as clean pipe-separated columns
- Horizontal rules, preformatted code blocks, definition lists
- Figure captions

**Development Infrastructure**
- Native unit tests (no hardware required)
- SDL-based visual simulator for desktop testing
- Mock hardware layer for isolated testing

## Features

- [x] EPUB 2 and EPUB 3 support
- [x] JPEG images in EPUBs
- [x] Rich HTML elements (lists, tables, blockquotes, code blocks)
- [x] Reading position persistence
- [x] Nested folder navigation
- [x] Cover image as sleep screen
- [x] WiFi book upload
- [x] Over-the-air updates
- [x] Configurable fonts and layout
- [x] Screen rotation
- [ ] Custom user fonts
- [ ] Full Unicode support
- [ ] Cover art in file browser

## Quick Start

### Install via Web

1. Connect your Xteink X4 via USB-C
2. Visit https://xteink.dve.al/
3. Click "Flash SignalOS firmware"

### Install from Source

```bash
# Clone with submodules
git clone --recursive https://github.com/tanishq-dubey/crosspoint-reader
cd crosspoint-reader

# Build and flash
~/.platformio/penv/bin/platformio run --target upload
```

## Development

### Prerequisites

- PlatformIO Core (or VS Code with PlatformIO extension)
- Python 3.8+
- USB-C cable and Xteink X4 (for device testing)
- SDL2 (optional, for simulator)

### Running Tests

We use PlatformIO's Unity framework for native testing. Tests run on your development machine, no hardware needed:

```bash
# Run all tests
~/.platformio/penv/bin/platformio test -e native

# Run specific suite
~/.platformio/penv/bin/platformio test -e native -f "test_data_structures"
```

Current test coverage:
- `test_basic` - Framework sanity checks
- `test_data_structures` - List contexts, table structures, blockquote nesting

### SDL Simulator

See your rendering changes without flashing hardware:

```bash
# Install SDL2
sudo apt install libsdl2-dev  # Linux
brew install sdl2             # macOS

# Build and run
~/.platformio/penv/bin/platformio run -e simulator
./.pio/build/simulator/program test/fixtures/test_mixed.html
```

**Controls:** Arrow keys for pages, `S` for screenshot, `Q` to quit.

### Project Layout

```
├── src/
│   ├── main/           # Firmware entry point
│   └── simulator/      # SDL simulator
├── lib/
│   ├── Epub/           # EPUB parsing, HTML rendering, page layout
│   ├── GfxRenderer/    # Drawing primitives, text rendering
│   └── EpdFont/        # Font handling
├── test/
│   ├── mocks/          # Arduino/hardware mocks for native builds
│   ├── fixtures/       # Sample HTML for testing
│   └── test_*/         # Test suites
└── platformio.ini      # Build configurations
```

## How It Works

### Memory Management

With only ~380KB of RAM on the ESP32-C3, we cache aggressively to SD card. First chapter load parses and caches; subsequent reads come from cache.

### Cache Structure

```
.signalOS/
└── epub_<hash>/
    ├── book.bin        # Metadata (title, author, spine, TOC)
    ├── progress.bin    # Reading position
    ├── cover.bmp       # Generated cover image
    └── sections/
        ├── 0.bin       # Chapter 0 layout data
        ├── 1.bin       # Chapter 1 layout data
        └── ...
```

Delete `.signalOS/` to clear all cached data.

## Acknowledgments

SignalOS exists because of the work that came before:

- **[CrossPoint Reader](https://github.com/daveallie/crosspoint-reader)** by Dave Allie - The foundation we built on. CrossPoint proved an open Xteink X4 firmware was viable and provided the EPUB parsing, caching architecture, and display handling that SignalOS extends.

- **[diy-esp32-epub-reader](https://github.com/atomic14/diy-esp32-epub-reader)** by atomic14 - Inspiration for the original CrossPoint project.

## Contributing

Found a bug? Want a feature? PRs welcome.

1. Fork the repo
2. Create a branch (`feature/better-tables`)
3. Make changes, add tests if applicable
4. Submit a PR

Check the [discussions](https://github.com/tanishq-dubey/crosspoint-reader/discussions) for ideas and ongoing work.

---

**SignalOS** is not affiliated with Xteink or any hardware manufacturer.

A **[DWS](https://github.com/tanishq-dubey)** project. It's your internet, take it back.
