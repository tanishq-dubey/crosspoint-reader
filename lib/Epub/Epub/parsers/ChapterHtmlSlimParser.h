#pragma once

#include <expat.h>

#include <climits>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../ParsedText.h"
#include "../blocks/TextBlock.h"

class Epub;
class Page;
class GfxRenderer;

#define MAX_WORD_SIZE 200

// Constants for HTML element rendering
constexpr int LIST_INDENT_PX = 24;       // Indent per list nesting level
constexpr int MAX_LIST_NESTING = 6;      // Max nesting to prevent overflow
constexpr int BLOCKQUOTE_INDENT_PX = 32; // Indent for blockquotes

// List context for tracking ordered/unordered lists
struct ListContext {
  bool isOrdered;   // true for <ol>, false for <ul>
  int itemNumber;   // Current item number for <ol>
};

// Table cell for collecting table content during parsing
struct TableCell {
  std::string text;
  EpdFontFamily::Style style = EpdFontFamily::REGULAR;
};

// Table row containing cells
struct TableRow {
  std::vector<TableCell> cells;
};

// Table data collected during parsing
struct TableData {
  std::vector<TableRow> rows;
  int currentRow = -1;
  int currentCell = -1;
  bool inCell = false;
};

class ChapterHtmlSlimParser {
  const std::string& filepath;
  GfxRenderer& renderer;
  std::function<void(std::unique_ptr<Page>)> completePageFn;
  std::function<void(int)> progressFn;  // Progress callback (0-100)
  int depth = 0;
  int skipUntilDepth = INT_MAX;
  int boldUntilDepth = INT_MAX;
  int italicUntilDepth = INT_MAX;
  // buffer for building up words from characters, will auto break if longer than this
  // leave one char at end for null pointer
  char partWordBuffer[MAX_WORD_SIZE + 1] = {};
  int partWordBufferIndex = 0;
  std::unique_ptr<ParsedText> currentTextBlock = nullptr;
  std::unique_ptr<Page> currentPage = nullptr;
  int16_t currentPageNextY = 0;
  int fontId;
  float lineCompression;
  bool extraParagraphSpacing;
  uint8_t paragraphAlignment;
  uint16_t viewportWidth;
  uint16_t viewportHeight;

  // Image support
  Epub* epub = nullptr;         // For resource extraction
  std::string contentBasePath;  // Base path for resolving relative image URLs
  std::string imageCacheDir;    // Directory for cached BMP files
  int imageCounter = 0;         // Counter for unique image filenames

  // List state tracking
  std::vector<ListContext> listStack;  // Stack for nested lists

  // Blockquote state tracking
  int blockquoteDepth = 0;  // Track nested blockquotes

  // Table state tracking
  std::unique_ptr<TableData> tableData;
  int tableDepth = 0;  // Track nested tables (skip nested ones)

  // Pre/code block state
  bool inPreBlock = false;

  void startNewTextBlock(TextBlock::Style style);
  void startNewTextBlockWithMargin(TextBlock::Style style, uint16_t leftMargin);
  void makePages();
  void processImage(const char* srcAttr);
  void addImageToPage(const std::string& bmpPath, uint16_t width, uint16_t height);
  void addHorizontalRuleToPage();
  void renderTable();
  // XML callbacks
  static void XMLCALL startElement(void* userData, const XML_Char* name, const XML_Char** atts);
  static void XMLCALL characterData(void* userData, const XML_Char* s, int len);
  static void XMLCALL endElement(void* userData, const XML_Char* name);

 public:
  explicit ChapterHtmlSlimParser(const std::string& filepath, GfxRenderer& renderer, Epub* epub,
                                 const std::string& contentBasePath, const std::string& imageCacheDir, const int fontId,
                                 const float lineCompression, const bool extraParagraphSpacing,
                                 const uint8_t paragraphAlignment, const uint16_t viewportWidth,
                                 const uint16_t viewportHeight,
                                 const std::function<void(std::unique_ptr<Page>)>& completePageFn,
                                 const std::function<void(int)>& progressFn = nullptr)
      : filepath(filepath),
        renderer(renderer),
        epub(epub),
        contentBasePath(contentBasePath),
        imageCacheDir(imageCacheDir),
        fontId(fontId),
        lineCompression(lineCompression),
        extraParagraphSpacing(extraParagraphSpacing),
        paragraphAlignment(paragraphAlignment),
        viewportWidth(viewportWidth),
        viewportHeight(viewportHeight),
        completePageFn(completePageFn),
        progressFn(progressFn) {}
  ~ChapterHtmlSlimParser() = default;
  bool parseAndBuildPages();
  void addLineToPage(std::shared_ptr<TextBlock> line);
};
