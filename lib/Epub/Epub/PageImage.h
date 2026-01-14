#pragma once

#include <memory>
#include <string>

#include "Page.h"

class GfxRenderer;

class PageImage final : public PageElement {
  std::string bmpPath;  // Path to cached BMP file on SD card
  uint16_t width;       // Image width in pixels
  uint16_t height;      // Image height in pixels

 public:
  PageImage(const std::string& bmpPath, uint16_t width, uint16_t height, int16_t xPos, int16_t yPos)
      : PageElement(xPos, yPos), bmpPath(bmpPath), width(width), height(height) {}

  PageElementTag getTag() const override { return TAG_PageImage; }
  void render(GfxRenderer& renderer, int fontId, int xOffset, int yOffset) override;
  bool serialize(FsFile& file) override;
  static std::unique_ptr<PageImage> deserialize(FsFile& file);

  uint16_t getWidth() const { return width; }
  uint16_t getHeight() const { return height; }
  const std::string& getBmpPath() const { return bmpPath; }
};
