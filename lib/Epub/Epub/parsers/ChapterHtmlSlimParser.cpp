#include "ChapterHtmlSlimParser.h"

#include <GfxRenderer.h>
#include <HardwareSerial.h>
#include <JpegToBmpConverter.h>
#include <SDCardManager.h>
#include <expat.h>

#include <cctype>
#include <vector>

#include "../Page.h"
#include "../PageImage.h"
#include "Epub.h"

const char* HEADER_TAGS[] = {"h1", "h2", "h3", "h4", "h5", "h6"};
constexpr int NUM_HEADER_TAGS = sizeof(HEADER_TAGS) / sizeof(HEADER_TAGS[0]);

// Minimum file size (in bytes) to show progress bar - smaller chapters don't benefit from it
constexpr size_t MIN_SIZE_FOR_PROGRESS = 50 * 1024;  // 50KB

const char* BLOCK_TAGS[] = {"p", "li", "div", "br", "blockquote"};
constexpr int NUM_BLOCK_TAGS = sizeof(BLOCK_TAGS) / sizeof(BLOCK_TAGS[0]);

const char* BOLD_TAGS[] = {"b", "strong"};
constexpr int NUM_BOLD_TAGS = sizeof(BOLD_TAGS) / sizeof(BOLD_TAGS[0]);

const char* ITALIC_TAGS[] = {"i", "em"};
constexpr int NUM_ITALIC_TAGS = sizeof(ITALIC_TAGS) / sizeof(ITALIC_TAGS[0]);

const char* IMAGE_TAGS[] = {"img"};
constexpr int NUM_IMAGE_TAGS = sizeof(IMAGE_TAGS) / sizeof(IMAGE_TAGS[0]);

const char* SKIP_TAGS[] = {"head", "table"};
constexpr int NUM_SKIP_TAGS = sizeof(SKIP_TAGS) / sizeof(SKIP_TAGS[0]);

// Image size constraints - smaller than cover images to leave room for text
constexpr int INLINE_IMAGE_MAX_WIDTH = 474;   // 480 - 6 (margins)
constexpr int INLINE_IMAGE_MAX_HEIGHT = 600;  // Leave room for surrounding text

bool isWhitespace(const char c) { return c == ' ' || c == '\r' || c == '\n' || c == '\t'; }

// Check if file extension indicates JPEG format
bool isJpegFile(const std::string& path) {
  if (path.length() < 4) return false;
  std::string ext = path.substr(path.length() - 4);
  for (char& c : ext) c = static_cast<char>(tolower(c));
  if (ext == ".jpg" || ext == "jpeg") return true;
  if (path.length() >= 5) {
    ext = path.substr(path.length() - 5);
    for (char& c : ext) c = static_cast<char>(tolower(c));
    if (ext == ".jpeg") return true;
  }
  return false;
}

// Normalize path by resolving .. and . components
std::string normalizePath(const std::string& basePath, const std::string& relativePath) {
  // If relativePath is already absolute (starts with /), return as-is
  if (!relativePath.empty() && relativePath[0] == '/') {
    return relativePath.substr(1);  // Remove leading slash for EPUB internal paths
  }

  // Combine base path and relative path
  std::string combined = basePath + relativePath;

  // Simple normalization: resolve .. components
  std::string result;
  std::vector<std::string> parts;
  size_t start = 0;
  for (size_t i = 0; i <= combined.length(); i++) {
    if (i == combined.length() || combined[i] == '/') {
      std::string part = combined.substr(start, i - start);
      if (part == "..") {
        if (!parts.empty()) parts.pop_back();
      } else if (part != "." && !part.empty()) {
        parts.push_back(part);
      }
      start = i + 1;
    }
  }

  for (size_t i = 0; i < parts.size(); i++) {
    if (i > 0) result += "/";
    result += parts[i];
  }
  return result;
}

// given the start and end of a tag, check to see if it matches a known tag
bool matches(const char* tag_name, const char* possible_tags[], const int possible_tag_count) {
  for (int i = 0; i < possible_tag_count; i++) {
    if (strcmp(tag_name, possible_tags[i]) == 0) {
      return true;
    }
  }
  return false;
}

// start a new text block if needed
void ChapterHtmlSlimParser::startNewTextBlock(const TextBlock::Style style) {
  if (currentTextBlock) {
    // already have a text block running and it is empty - just reuse it
    if (currentTextBlock->isEmpty()) {
      currentTextBlock->setStyle(style);
      return;
    }

    makePages();
  }
  currentTextBlock.reset(new ParsedText(style, extraParagraphSpacing));
}

void XMLCALL ChapterHtmlSlimParser::startElement(void* userData, const XML_Char* name, const XML_Char** atts) {
  auto* self = static_cast<ChapterHtmlSlimParser*>(userData);

  // Middle of skip
  if (self->skipUntilDepth < self->depth) {
    self->depth += 1;
    return;
  }

  if (matches(name, IMAGE_TAGS, NUM_IMAGE_TAGS)) {
    // Extract src attribute and process the image
    if (self->epub && atts != nullptr) {
      for (int i = 0; atts[i]; i += 2) {
        if (strcmp(atts[i], "src") == 0) {
          self->processImage(atts[i + 1]);
          break;
        }
      }
    }
    self->skipUntilDepth = self->depth;
    self->depth += 1;
    return;
  }

  if (matches(name, SKIP_TAGS, NUM_SKIP_TAGS)) {
    // start skip
    self->skipUntilDepth = self->depth;
    self->depth += 1;
    return;
  }

  // Skip blocks with role="doc-pagebreak" and epub:type="pagebreak"
  if (atts != nullptr) {
    for (int i = 0; atts[i]; i += 2) {
      if (strcmp(atts[i], "role") == 0 && strcmp(atts[i + 1], "doc-pagebreak") == 0 ||
          strcmp(atts[i], "epub:type") == 0 && strcmp(atts[i + 1], "pagebreak") == 0) {
        self->skipUntilDepth = self->depth;
        self->depth += 1;
        return;
      }
    }
  }

  if (matches(name, HEADER_TAGS, NUM_HEADER_TAGS)) {
    self->startNewTextBlock(TextBlock::CENTER_ALIGN);
    self->boldUntilDepth = std::min(self->boldUntilDepth, self->depth);
  } else if (matches(name, BLOCK_TAGS, NUM_BLOCK_TAGS)) {
    if (strcmp(name, "br") == 0) {
      self->startNewTextBlock(self->currentTextBlock->getStyle());
    } else {
      self->startNewTextBlock((TextBlock::Style)self->paragraphAlignment);
    }
  } else if (matches(name, BOLD_TAGS, NUM_BOLD_TAGS)) {
    self->boldUntilDepth = std::min(self->boldUntilDepth, self->depth);
  } else if (matches(name, ITALIC_TAGS, NUM_ITALIC_TAGS)) {
    self->italicUntilDepth = std::min(self->italicUntilDepth, self->depth);
  }

  self->depth += 1;
}

void XMLCALL ChapterHtmlSlimParser::characterData(void* userData, const XML_Char* s, const int len) {
  auto* self = static_cast<ChapterHtmlSlimParser*>(userData);

  // Middle of skip
  if (self->skipUntilDepth < self->depth) {
    return;
  }

  EpdFontFamily::Style fontStyle = EpdFontFamily::REGULAR;
  if (self->boldUntilDepth < self->depth && self->italicUntilDepth < self->depth) {
    fontStyle = EpdFontFamily::BOLD_ITALIC;
  } else if (self->boldUntilDepth < self->depth) {
    fontStyle = EpdFontFamily::BOLD;
  } else if (self->italicUntilDepth < self->depth) {
    fontStyle = EpdFontFamily::ITALIC;
  }

  for (int i = 0; i < len; i++) {
    if (isWhitespace(s[i])) {
      // Currently looking at whitespace, if there's anything in the partWordBuffer, flush it
      if (self->partWordBufferIndex > 0) {
        self->partWordBuffer[self->partWordBufferIndex] = '\0';
        self->currentTextBlock->addWord(self->partWordBuffer, fontStyle);
        self->partWordBufferIndex = 0;
      }
      // Skip the whitespace char
      continue;
    }

    // Skip soft-hyphen with UTF-8 representation (U+00AD) = 0xC2 0xAD
    const XML_Char SHY_BYTE_1 = static_cast<XML_Char>(0xC2);
    const XML_Char SHY_BYTE_2 = static_cast<XML_Char>(0xAD);
    // 1. Check for the start of the 2-byte Soft Hyphen sequence
    if (s[i] == SHY_BYTE_1) {
      // 2. Check if the next byte exists AND if it completes the sequence
      //    We must check i + 1 < len to prevent reading past the end of the buffer.
      if ((i + 1 < len) && (s[i + 1] == SHY_BYTE_2)) {
        // Sequence 0xC2 0xAD found!
        // Skip the current byte (0xC2) and the next byte (0xAD)
        i++;       // Increment 'i' one more time to skip the 0xAD byte
        continue;  // Skip the rest of the loop and move to the next iteration
      }
    }

    // If we're about to run out of space, then cut the word off and start a new one
    if (self->partWordBufferIndex >= MAX_WORD_SIZE) {
      self->partWordBuffer[self->partWordBufferIndex] = '\0';
      self->currentTextBlock->addWord(self->partWordBuffer, fontStyle);
      self->partWordBufferIndex = 0;
    }

    self->partWordBuffer[self->partWordBufferIndex++] = s[i];
  }

  // If we have > 750 words buffered up, perform the layout and consume out all but the last line
  // There should be enough here to build out 1-2 full pages and doing this will free up a lot of
  // memory.
  // Spotted when reading Intermezzo, there are some really long text blocks in there.
  if (self->currentTextBlock->size() > 750) {
    Serial.printf("[%lu] [EHP] Text block too long, splitting into multiple pages\n", millis());
    self->currentTextBlock->layoutAndExtractLines(
        self->renderer, self->fontId, self->viewportWidth,
        [self](const std::shared_ptr<TextBlock>& textBlock) { self->addLineToPage(textBlock); }, false);
  }
}

void XMLCALL ChapterHtmlSlimParser::endElement(void* userData, const XML_Char* name) {
  auto* self = static_cast<ChapterHtmlSlimParser*>(userData);

  if (self->partWordBufferIndex > 0) {
    // Only flush out part word buffer if we're closing a block tag or are at the top of the HTML file.
    // We don't want to flush out content when closing inline tags like <span>.
    // Currently this also flushes out on closing <b> and <i> tags, but they are line tags so that shouldn't happen,
    // text styling needs to be overhauled to fix it.
    const bool shouldBreakText =
        matches(name, BLOCK_TAGS, NUM_BLOCK_TAGS) || matches(name, HEADER_TAGS, NUM_HEADER_TAGS) ||
        matches(name, BOLD_TAGS, NUM_BOLD_TAGS) || matches(name, ITALIC_TAGS, NUM_ITALIC_TAGS) || self->depth == 1;

    if (shouldBreakText) {
      EpdFontFamily::Style fontStyle = EpdFontFamily::REGULAR;
      if (self->boldUntilDepth < self->depth && self->italicUntilDepth < self->depth) {
        fontStyle = EpdFontFamily::BOLD_ITALIC;
      } else if (self->boldUntilDepth < self->depth) {
        fontStyle = EpdFontFamily::BOLD;
      } else if (self->italicUntilDepth < self->depth) {
        fontStyle = EpdFontFamily::ITALIC;
      }

      self->partWordBuffer[self->partWordBufferIndex] = '\0';
      self->currentTextBlock->addWord(self->partWordBuffer, fontStyle);
      self->partWordBufferIndex = 0;
    }
  }

  self->depth -= 1;

  // Leaving skip
  if (self->skipUntilDepth == self->depth) {
    self->skipUntilDepth = INT_MAX;
  }

  // Leaving bold
  if (self->boldUntilDepth == self->depth) {
    self->boldUntilDepth = INT_MAX;
  }

  // Leaving italic
  if (self->italicUntilDepth == self->depth) {
    self->italicUntilDepth = INT_MAX;
  }
}

bool ChapterHtmlSlimParser::parseAndBuildPages() {
  startNewTextBlock((TextBlock::Style)this->paragraphAlignment);

  const XML_Parser parser = XML_ParserCreate(nullptr);
  int done;

  if (!parser) {
    Serial.printf("[%lu] [EHP] Couldn't allocate memory for parser\n", millis());
    return false;
  }

  FsFile file;
  if (!SdMan.openFileForRead("EHP", filepath, file)) {
    XML_ParserFree(parser);
    return false;
  }

  // Get file size for progress calculation
  const size_t totalSize = file.size();
  size_t bytesRead = 0;
  int lastProgress = -1;

  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, characterData);

  do {
    void* const buf = XML_GetBuffer(parser, 1024);
    if (!buf) {
      Serial.printf("[%lu] [EHP] Couldn't allocate memory for buffer\n", millis());
      XML_StopParser(parser, XML_FALSE);                // Stop any pending processing
      XML_SetElementHandler(parser, nullptr, nullptr);  // Clear callbacks
      XML_SetCharacterDataHandler(parser, nullptr);
      XML_ParserFree(parser);
      file.close();
      return false;
    }

    const size_t len = file.read(buf, 1024);

    if (len == 0 && file.available() > 0) {
      Serial.printf("[%lu] [EHP] File read error\n", millis());
      XML_StopParser(parser, XML_FALSE);                // Stop any pending processing
      XML_SetElementHandler(parser, nullptr, nullptr);  // Clear callbacks
      XML_SetCharacterDataHandler(parser, nullptr);
      XML_ParserFree(parser);
      file.close();
      return false;
    }

    // Update progress (call every 10% change to avoid too frequent updates)
    // Only show progress for larger chapters where rendering overhead is worth it
    bytesRead += len;
    if (progressFn && totalSize >= MIN_SIZE_FOR_PROGRESS) {
      const int progress = static_cast<int>((bytesRead * 100) / totalSize);
      if (lastProgress / 10 != progress / 10) {
        lastProgress = progress;
        progressFn(progress);
      }
    }

    done = file.available() == 0;

    if (XML_ParseBuffer(parser, static_cast<int>(len), done) == XML_STATUS_ERROR) {
      Serial.printf("[%lu] [EHP] Parse error at line %lu:\n%s\n", millis(), XML_GetCurrentLineNumber(parser),
                    XML_ErrorString(XML_GetErrorCode(parser)));
      XML_StopParser(parser, XML_FALSE);                // Stop any pending processing
      XML_SetElementHandler(parser, nullptr, nullptr);  // Clear callbacks
      XML_SetCharacterDataHandler(parser, nullptr);
      XML_ParserFree(parser);
      file.close();
      return false;
    }
  } while (!done);

  XML_StopParser(parser, XML_FALSE);                // Stop any pending processing
  XML_SetElementHandler(parser, nullptr, nullptr);  // Clear callbacks
  XML_SetCharacterDataHandler(parser, nullptr);
  XML_ParserFree(parser);
  file.close();

  // Process last page if there is still text
  if (currentTextBlock) {
    makePages();
    completePageFn(std::move(currentPage));
    currentPage.reset();
    currentTextBlock.reset();
  }

  return true;
}

void ChapterHtmlSlimParser::addLineToPage(std::shared_ptr<TextBlock> line) {
  const int lineHeight = renderer.getLineHeight(fontId) * lineCompression;

  if (currentPageNextY + lineHeight > viewportHeight) {
    completePageFn(std::move(currentPage));
    currentPage.reset(new Page());
    currentPageNextY = 0;
  }

  currentPage->elements.push_back(std::make_shared<PageLine>(line, 0, currentPageNextY));
  currentPageNextY += lineHeight;
}

void ChapterHtmlSlimParser::makePages() {
  if (!currentTextBlock) {
    Serial.printf("[%lu] [EHP] !! No text block to make pages for !!\n", millis());
    return;
  }

  if (!currentPage) {
    currentPage.reset(new Page());
    currentPageNextY = 0;
  }

  const int lineHeight = renderer.getLineHeight(fontId) * lineCompression;
  currentTextBlock->layoutAndExtractLines(
      renderer, fontId, viewportWidth,
      [this](const std::shared_ptr<TextBlock>& textBlock) { addLineToPage(textBlock); });
  // Extra paragraph spacing if enabled
  if (extraParagraphSpacing) {
    currentPageNextY += lineHeight / 2;
  }
}

void ChapterHtmlSlimParser::processImage(const char* srcAttr) {
  if (!epub || !srcAttr || srcAttr[0] == '\0') {
    return;
  }

  // Resolve relative path against content base path
  const std::string fullPath = normalizePath(contentBasePath, srcAttr);

  // Check if image is JPEG (only supported format for now)
  if (!isJpegFile(fullPath)) {
    Serial.printf("[%lu] [EHP] Skipping non-JPEG image: %s\n", millis(), fullPath.c_str());
    return;
  }

  // Generate unique BMP path in cache directory
  const std::string bmpPath = imageCacheDir + "/img_" + std::to_string(imageCounter++) + ".bmp";

  // Extract JPEG from EPUB to temp file
  const std::string tmpJpegPath = imageCacheDir + "/.tmp_img.jpg";
  FsFile tmpJpeg;
  if (!SdMan.openFileForWrite("EHP", tmpJpegPath, tmpJpeg)) {
    Serial.printf("[%lu] [EHP] Failed to create temp JPEG file\n", millis());
    return;
  }

  if (!epub->readItemContentsToStream(fullPath, tmpJpeg, 1024)) {
    Serial.printf("[%lu] [EHP] Failed to extract image: %s\n", millis(), fullPath.c_str());
    tmpJpeg.close();
    SdMan.remove(tmpJpegPath.c_str());
    return;
  }
  tmpJpeg.close();

  // Open temp JPEG for reading
  if (!SdMan.openFileForRead("EHP", tmpJpegPath, tmpJpeg)) {
    Serial.printf("[%lu] [EHP] Failed to reopen temp JPEG\n", millis());
    SdMan.remove(tmpJpegPath.c_str());
    return;
  }

  // Create output BMP file
  FsFile bmpFile;
  if (!SdMan.openFileForWrite("EHP", bmpPath, bmpFile)) {
    Serial.printf("[%lu] [EHP] Failed to create BMP file: %s\n", millis(), bmpPath.c_str());
    tmpJpeg.close();
    SdMan.remove(tmpJpegPath.c_str());
    return;
  }

  // Convert JPEG to BMP with scaling
  uint16_t imgWidth = 0;
  uint16_t imgHeight = 0;
  const bool success = JpegToBmpConverter::jpegFileToBmpStreamScaled(tmpJpeg, bmpFile, INLINE_IMAGE_MAX_WIDTH,
                                                                     INLINE_IMAGE_MAX_HEIGHT, &imgWidth, &imgHeight);
  bmpFile.close();
  tmpJpeg.close();
  SdMan.remove(tmpJpegPath.c_str());

  if (!success || imgWidth == 0 || imgHeight == 0) {
    Serial.printf("[%lu] [EHP] Failed to convert image: %s\n", millis(), fullPath.c_str());
    SdMan.remove(bmpPath.c_str());
    return;
  }

  Serial.printf("[%lu] [EHP] Converted image %s -> %s (%dx%d)\n", millis(), fullPath.c_str(), bmpPath.c_str(), imgWidth,
                imgHeight);

  // Flush any pending text block before adding image
  if (currentTextBlock && !currentTextBlock->isEmpty()) {
    makePages();
  }

  // Add image to page
  addImageToPage(bmpPath, imgWidth, imgHeight);
}

void ChapterHtmlSlimParser::addImageToPage(const std::string& bmpPath, const uint16_t width, const uint16_t height) {
  if (!currentPage) {
    currentPage.reset(new Page());
    currentPageNextY = 0;
  }

  // Calculate centered X position
  int16_t xPos = static_cast<int16_t>((viewportWidth - width) / 2);
  if (xPos < 0) xPos = 0;

  // Check if image fits on current page
  if (currentPageNextY + height > viewportHeight) {
    // Image doesn't fit - complete current page, start new one
    completePageFn(std::move(currentPage));
    currentPage.reset(new Page());
    currentPageNextY = 0;
  }

  // Add image element to page
  currentPage->elements.push_back(std::make_shared<PageImage>(bmpPath, width, height, xPos, currentPageNextY));

  // Update Y position for next element
  currentPageNextY += height;

  // Add some spacing after image (half a line height)
  const int lineHeight = renderer.getLineHeight(fontId) * lineCompression;
  currentPageNextY += lineHeight / 2;
}
