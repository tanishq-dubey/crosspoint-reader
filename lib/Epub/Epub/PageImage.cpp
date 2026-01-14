#include "PageImage.h"

#include <Bitmap.h>
#include <GfxRenderer.h>
#include <HardwareSerial.h>
#include <SDCardManager.h>
#include <Serialization.h>

void PageImage::render(GfxRenderer& renderer, const int fontId, const int xOffset, const int yOffset) {
  FsFile bmpFile;
  if (!SdMan.openFileForRead("IMG", bmpPath, bmpFile)) {
    Serial.printf("[%lu] [IMG] Failed to open BMP: %s\n", millis(), bmpPath.c_str());
    return;
  }

  Bitmap bitmap(bmpFile, false);  // No additional dithering (already dithered during conversion)
  const BmpReaderError err = bitmap.parseHeaders();
  if (err != BmpReaderError::Ok) {
    Serial.printf("[%lu] [IMG] BMP parse error: %s\n", millis(), Bitmap::errorToString(err));
    bmpFile.close();
    return;
  }

  // Use existing drawBitmap which reads row-by-row for memory efficiency
  renderer.drawBitmap(bitmap, xPos + xOffset, yPos + yOffset, width, height);
  bmpFile.close();
}

bool PageImage::serialize(FsFile& file) {
  serialization::writePod(file, xPos);
  serialization::writePod(file, yPos);
  serialization::writePod(file, width);
  serialization::writePod(file, height);
  serialization::writeString(file, bmpPath);
  return true;
}

std::unique_ptr<PageImage> PageImage::deserialize(FsFile& file) {
  int16_t xPos;
  int16_t yPos;
  uint16_t imgWidth;
  uint16_t imgHeight;
  std::string path;

  serialization::readPod(file, xPos);
  serialization::readPod(file, yPos);
  serialization::readPod(file, imgWidth);
  serialization::readPod(file, imgHeight);
  serialization::readString(file, path);

  return std::unique_ptr<PageImage>(new PageImage(path, imgWidth, imgHeight, xPos, yPos));
}
