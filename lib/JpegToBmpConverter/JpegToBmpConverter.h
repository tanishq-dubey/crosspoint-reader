#pragma once

#include <cstdint>

class FsFile;
class Print;
class ZipFile;

class JpegToBmpConverter {
  static void writeBmpHeader(Print& bmpOut, int width, int height);
  // [COMMENTED OUT] static uint8_t grayscaleTo2Bit(uint8_t grayscale, int x, int y);
  static unsigned char jpegReadCallback(unsigned char* pBuf, unsigned char buf_size,
                                        unsigned char* pBytes_actually_read, void* pCallback_data);

 public:
  // Convert JPEG to BMP using default cover image size limits (480x800)
  static bool jpegFileToBmpStream(FsFile& jpegFile, Print& bmpOut);

  // Convert JPEG to BMP with custom size limits and return actual output dimensions
  // Returns true on success, false on failure
  // outWidth/outHeight will contain the actual BMP dimensions after scaling
  static bool jpegFileToBmpStreamScaled(FsFile& jpegFile, Print& bmpOut, int maxWidth, int maxHeight,
                                        uint16_t* outWidth, uint16_t* outHeight);
};
