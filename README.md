# SignalOS

**A DWS (Dubey Web Services) Project**
*"It's your internet, take it back"*

---

Firmware for the **Xteink X4** e-paper display reader (unaffiliated with Xteink).
Built using **PlatformIO** and targeting the **ESP32-C3** microcontroller.

SignalOS is a purpose-built firmware designed to be a drop-in, fully open-source replacement for the official
Xteink firmware. It aims to match or improve upon the standard EPUB reading experience.

![](./docs/images/cover.jpg)

## Motivation

E-paper devices are fantastic for reading, but most commercially available readers are closed systems with limited
customisation. The **Xteink X4** is an affordable, e-paper device, however the official firmware remains closed.
SignalOS exists partly as a fun side-project and partly to open up the ecosystem and truly unlock the device's
potential.

SignalOS aims to:
* Provide a **fully open-source alternative** to the official firmware.
* Offer a **document reader** capable of handling EPUB content on constrained hardware.
* Support **customisable font, layout, and display** options.
* Run purely on the **Xteink X4 hardware**.

This project is **not affiliated with Xteink**; it's built as a community project by DWS.

## Features & Usage

- [x] EPUB parsing and rendering (EPUB 2 and EPUB 3)
- [x] Image support within EPUB (JPEG)
- [x] Rich HTML element support
  - [x] Lists (ordered and unordered with bullets/numbers)
  - [x] Blockquotes with vertical line indicator
  - [x] Tables with pipe-separated columns
  - [x] Horizontal rules
  - [x] Pre-formatted code blocks
  - [x] Definition lists
  - [x] Figures with captions
- [x] Saved reading position
- [x] File explorer with file picker
  - [x] Basic EPUB picker from root directory
  - [x] Support nested folders
  - [ ] EPUB picker with cover art
- [x] Custom sleep screen
  - [x] Cover sleep screen
- [x] Wifi book upload
- [x] Wifi OTA updates
- [x] Configurable font, layout, and display options
  - [ ] User provided fonts
  - [ ] Full UTF support
- [x] Screen rotation

See [the user guide](./USER_GUIDE.md) for instructions on operating SignalOS.

## Installing

### Web (latest firmware)

1. Connect your Xteink X4 to your computer via USB-C
2. Go to https://xteink.dve.al/ and click "Flash SignalOS firmware"

To revert back to the official firmware, you can flash the latest official firmware from https://xteink.dve.al/, or swap
back to the other partition using the "Swap boot partition" button here https://xteink.dve.al/debug.

### Web (specific firmware version)

1. Connect your Xteink X4 to your computer via USB-C
2. Download the `firmware.bin` file from the release of your choice via the [releases page](https://github.com/tanishq-dubey/crosspoint-reader/releases)
3. Go to https://xteink.dve.al/ and flash the firmware file using the "OTA fast flash controls" section

To revert back to the official firmware, you can flash the latest official firmware from https://xteink.dve.al/, or swap
back to the other partition using the "Swap boot partition" button here https://xteink.dve.al/debug.

### Manual

See [Development](#development) below.

## Development

### Prerequisites

* **PlatformIO Core** or **VS Code + PlatformIO IDE**
* Python 3.8+
* USB-C cable for flashing the ESP32-C3
* Xteink X4
* SDL2 (optional, for simulator)

### Checking out the code

SignalOS uses PlatformIO for building and flashing the firmware. To get started, clone the repository:

```bash
git clone --recursive https://github.com/tanishq-dubey/crosspoint-reader

# Or, if you've already cloned without --recursive:
git submodule update --init --recursive
```

### Building & Flashing

Connect your Xteink X4 to your computer via USB-C and run:

```bash
# Build and flash to device
~/.platformio/penv/bin/platformio run --target upload

# Or just build without flashing
~/.platformio/penv/bin/platformio run
```

### Running Tests

SignalOS includes a native testing infrastructure using PlatformIO's Unity test framework:

```bash
# Run all native unit tests
~/.platformio/penv/bin/platformio test -e native

# Run specific test suite
~/.platformio/penv/bin/platformio test -e native -f "test_data_structures"
```

**Test suites available:**
- `test_basic` - Framework verification tests
- `test_data_structures` - HTML parsing data structure tests (lists, tables, blockquotes)

### SDL Simulator

For visual testing without physical hardware, SignalOS includes an SDL-based display simulator:

```bash
# Install SDL2 (Linux)
sudo apt install libsdl2-dev

# Install SDL2 (macOS)
brew install sdl2

# Build the simulator
~/.platformio/penv/bin/platformio run -e simulator

# Run the simulator
./.pio/build/simulator/program [optional_html_file]
```

**Simulator controls:**
- `Left/Right Arrow` - Page navigation
- `S` - Save screenshot (BMP)
- `Q` or `Escape` - Quit

### Project Structure

```
signalOS/
├── src/                    # Main firmware source
│   └── simulator/          # SDL simulator source
├── lib/                    # Libraries
│   ├── Epub/               # EPUB parsing and rendering
│   ├── GfxRenderer/        # Graphics rendering
│   ├── EpdFont/            # E-paper font handling
│   └── ...
├── test/                   # Test infrastructure
│   ├── mocks/              # Hardware mocks for native testing
│   ├── fixtures/           # Test HTML files
│   ├── test_basic/         # Basic framework tests
│   └── test_data_structures/ # Data structure tests
└── platformio.ini          # Build configuration
```

## Internals

SignalOS is pretty aggressive about caching data down to the SD card to minimise RAM usage. The ESP32-C3 only
has ~380KB of usable RAM, so we have to be careful. A lot of the decisions made in the design of the firmware were based
on this constraint.

### Data caching

The first time chapters of a book are loaded, they are cached to the SD card. Subsequent loads are served from the
cache. This cache directory exists at `.signalOS` on the SD card. The structure is as follows:


```
.signalOS/
├── epub_12471232/       # Each EPUB is cached to a subdirectory named `epub_<hash>`
│   ├── progress.bin     # Stores reading progress (chapter, page, etc.)
│   ├── cover.bmp        # Book cover image (once generated)
│   ├── book.bin         # Book metadata (title, author, spine, table of contents, etc.)
│   └── sections/        # All chapter data is stored in the sections subdirectory
│       ├── 0.bin        # Chapter data (screen count, all text layout info, etc.)
│       ├── 1.bin        #     files are named by their index in the spine
│       └── ...
│
└── epub_189013891/
```

Deleting the `.signalOS` directory will clear the entire cache.

Due the way it's currently implemented, the cache is not automatically cleared when a book is deleted and moving a book
file will use a new cache directory, resetting the reading progress.

For more details on the internal file structures, see the [file formats document](./docs/file-formats.md).

## Contributing

Contributions are very welcome!

If you're looking for a way to help out, take a look at the [ideas discussion board](https://github.com/tanishq-dubey/crosspoint-reader/discussions/categories/ideas).
If there's something there you'd like to work on, leave a comment so that we can avoid duplicated effort.

### To submit a contribution:

1. Fork the repo
2. Create a branch (`feature/dithering-improvement`)
3. Make changes
4. Submit a PR

---

SignalOS is **not affiliated with Xteink or any manufacturer of the X4 hardware**.

A **DWS (Dubey Web Services)** project.

Huge shoutout to [**diy-esp32-epub-reader** by atomic14](https://github.com/atomic14/diy-esp32-epub-reader), which was a project I took a lot of inspiration from as I
was making SignalOS.
