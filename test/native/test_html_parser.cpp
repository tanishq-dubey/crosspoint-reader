// Unit tests for ChapterHtmlSlimParser HTML element handling
// Tests list bullets, blockquotes, tables, pre blocks, and horizontal rules

#include <unity.h>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>

// Mock includes
#include "Arduino.h"
#include "EInkDisplay.h"
#include "SdFat.h"
#include "TestFont.h"

// Library includes
#include "GfxRenderer.h"
#include "Page.h"
#include "blocks/TextBlock.h"
#include "parsers/ChapterHtmlSlimParser.h"

// Test fixture globals
static EInkDisplay* testDisplay = nullptr;
static GfxRenderer* testRenderer = nullptr;
static std::vector<std::unique_ptr<Page>> collectedPages;

// Font ID used for testing
static constexpr int TEST_FONT_ID = 0;

// Test viewport dimensions (portrait mode)
static constexpr int TEST_VIEWPORT_WIDTH = 474;  // 480 - margins
static constexpr int TEST_VIEWPORT_HEIGHT = 780; // 800 - margins

// Create a simple test font that has monospace-like characteristics
static EpdGlyph testGlyphs[128];
static EpdUnicodeInterval testIntervals[1];
static uint8_t testBitmap[1] = {0};
static EpdFontData testFontData;
static EpdFont* testFont = nullptr;
static EpdFontFamily* testFontFamily = nullptr;

void initTestFont() {
    // Initialize all glyphs to the same size (monospace style)
    for (int i = 0; i < 128; i++) {
        testGlyphs[i].width = 8;
        testGlyphs[i].height = 16;
        testGlyphs[i].advanceX = 10;  // Each character is 10 pixels wide
        testGlyphs[i].left = 0;
        testGlyphs[i].top = 14;
        testGlyphs[i].dataLength = 0;
        testGlyphs[i].dataOffset = 0;
    }

    // Set space width
    testGlyphs[32].advanceX = 10;  // Space

    // Unicode intervals
    testIntervals[0].first = 0;
    testIntervals[0].last = 127;
    testIntervals[0].offset = 0;

    // Font data
    testFontData.bitmap = testBitmap;
    testFontData.glyph = testGlyphs;
    testFontData.intervals = testIntervals;
    testFontData.intervalCount = 1;
    testFontData.advanceY = 20;
    testFontData.ascender = 14;
    testFontData.descender = 4;
    testFontData.is2Bit = false;

    testFont = new EpdFont(&testFontData);
    testFontFamily = new EpdFontFamily(testFont, testFont, testFont, testFont);
}

void cleanupTestFont() {
    delete testFontFamily;
    delete testFont;
    testFontFamily = nullptr;
    testFont = nullptr;
}

// Helper function to create a temporary HTML file and return its path
std::string createTempHtmlFile(const std::string& content) {
    static int fileCounter = 0;
    std::string filename = "/tmp/test_html_" + std::to_string(fileCounter++) + ".html";

    std::ofstream file(filename);
    file << "<!DOCTYPE html><html><head><title>Test</title></head><body>";
    file << content;
    file << "</body></html>";
    file.close();

    return filename;
}

// Page callback for collecting parsed pages
void pageCallback(std::unique_ptr<Page> page) {
    collectedPages.push_back(std::move(page));
}

// Helper to parse HTML and collect pages
bool parseHtml(const std::string& htmlContent) {
    collectedPages.clear();

    std::string filepath = createTempHtmlFile(htmlContent);

    ChapterHtmlSlimParser parser(
        filepath,
        *testRenderer,
        nullptr,  // No epub for unit tests
        "",       // No base path
        "",       // No image cache dir
        TEST_FONT_ID,
        1.0f,     // Line compression
        true,     // Extra paragraph spacing
        0,        // Paragraph alignment (justified)
        TEST_VIEWPORT_WIDTH,
        TEST_VIEWPORT_HEIGHT,
        pageCallback,
        nullptr   // No progress callback
    );

    bool result = parser.parseAndBuildPages();

    // Clean up temp file
    std::remove(filepath.c_str());

    return result;
}

// Helper to get text content from all pages
std::string getAllPagesText() {
    std::string result;
    for (const auto& page : collectedPages) {
        // Note: We can't easily extract text from pages without inspecting blocks
        // This is a placeholder - real tests should inspect block properties
    }
    return result;
}

// ============================================================================
// Test Setup/Teardown
// ============================================================================

void setUp() {
    testDisplay = new EInkDisplay();
    testRenderer = new GfxRenderer(*testDisplay);
    initTestFont();
    testRenderer->insertFont(TEST_FONT_ID, *testFontFamily);
    collectedPages.clear();
}

void tearDown() {
    collectedPages.clear();
    cleanupTestFont();
    delete testRenderer;
    delete testDisplay;
    testRenderer = nullptr;
    testDisplay = nullptr;
}

// ============================================================================
// Unordered List Tests
// ============================================================================

void test_unordered_list_creates_pages() {
    const char* html = "<ul><li>Item 1</li><li>Item 2</li><li>Item 3</li></ul>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

void test_unordered_list_nested() {
    const char* html =
        "<ul>"
        "  <li>Level 1"
        "    <ul>"
        "      <li>Level 2</li>"
        "    </ul>"
        "  </li>"
        "</ul>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Ordered List Tests
// ============================================================================

void test_ordered_list_creates_pages() {
    const char* html = "<ol><li>First</li><li>Second</li><li>Third</li></ol>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

void test_ordered_list_nested() {
    const char* html =
        "<ol>"
        "  <li>Item 1"
        "    <ol>"
        "      <li>Sub-item 1.1</li>"
        "      <li>Sub-item 1.2</li>"
        "    </ol>"
        "  </li>"
        "  <li>Item 2</li>"
        "</ol>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Blockquote Tests
// ============================================================================

void test_blockquote_creates_pages() {
    const char* html = "<blockquote>This is a quoted text.</blockquote>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

void test_blockquote_nested() {
    const char* html =
        "<blockquote>"
        "  First level"
        "  <blockquote>"
        "    Second level"
        "    <blockquote>"
        "      Third level"
        "    </blockquote>"
        "  </blockquote>"
        "</blockquote>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Table Tests
// ============================================================================

void test_simple_table_creates_pages() {
    const char* html =
        "<table>"
        "  <tr><td>Cell 1</td><td>Cell 2</td></tr>"
        "  <tr><td>Cell 3</td><td>Cell 4</td></tr>"
        "</table>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

void test_table_with_headers() {
    const char* html =
        "<table>"
        "  <tr><th>Name</th><th>Age</th></tr>"
        "  <tr><td>Alice</td><td>30</td></tr>"
        "  <tr><td>Bob</td><td>25</td></tr>"
        "</table>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Horizontal Rule Tests
// ============================================================================

void test_horizontal_rule_creates_pages() {
    const char* html = "<p>Before</p><hr><p>After</p>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Pre/Code Block Tests
// ============================================================================

void test_pre_block_creates_pages() {
    const char* html =
        "<pre>\n"
        "function test() {\n"
        "    return 42;\n"
        "}\n"
        "</pre>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

void test_code_inline() {
    const char* html = "<p>Use the <code>printf</code> function.</p>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Definition List Tests
// ============================================================================

void test_definition_list_creates_pages() {
    const char* html =
        "<dl>"
        "  <dt>Term 1</dt>"
        "  <dd>Definition 1</dd>"
        "  <dt>Term 2</dt>"
        "  <dd>Definition 2</dd>"
        "</dl>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Figure/Figcaption Tests
// ============================================================================

void test_figure_with_caption() {
    const char* html =
        "<figure>"
        "  <figcaption>Figure 1: Test caption</figcaption>"
        "</figure>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Mixed Content Tests
// ============================================================================

void test_mixed_content() {
    const char* html =
        "<h1>Title</h1>"
        "<p>Introduction paragraph.</p>"
        "<ul>"
        "  <li>Bullet 1</li>"
        "  <li>Bullet 2</li>"
        "</ul>"
        "<blockquote>A quote</blockquote>"
        "<table><tr><td>A</td><td>B</td></tr></table>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

void test_paragraph_after_list() {
    const char* html =
        "<p>Before list.</p>"
        "<ul><li>Item</li></ul>"
        "<p>After list.</p>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Edge Cases
// ============================================================================

void test_empty_list() {
    const char* html = "<ul></ul>";
    TEST_ASSERT_TRUE(parseHtml(html));
}

void test_empty_table() {
    const char* html = "<table></table>";
    TEST_ASSERT_TRUE(parseHtml(html));
}

void test_deeply_nested_lists() {
    const char* html =
        "<ul>"
        "  <li>L1"
        "    <ul><li>L2"
        "      <ul><li>L3"
        "        <ul><li>L4"
        "          <ul><li>L5"
        "            <ul><li>L6</li></ul>"
        "          </li></ul>"
        "        </li></ul>"
        "      </li></ul>"
        "    </li></ul>"
        "  </li>"
        "</ul>";
    TEST_ASSERT_TRUE(parseHtml(html));
    TEST_ASSERT_GREATER_THAN(0, collectedPages.size());
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Unordered list tests
    RUN_TEST(test_unordered_list_creates_pages);
    RUN_TEST(test_unordered_list_nested);

    // Ordered list tests
    RUN_TEST(test_ordered_list_creates_pages);
    RUN_TEST(test_ordered_list_nested);

    // Blockquote tests
    RUN_TEST(test_blockquote_creates_pages);
    RUN_TEST(test_blockquote_nested);

    // Table tests
    RUN_TEST(test_simple_table_creates_pages);
    RUN_TEST(test_table_with_headers);

    // Horizontal rule tests
    RUN_TEST(test_horizontal_rule_creates_pages);

    // Pre/code tests
    RUN_TEST(test_pre_block_creates_pages);
    RUN_TEST(test_code_inline);

    // Definition list tests
    RUN_TEST(test_definition_list_creates_pages);

    // Figure tests
    RUN_TEST(test_figure_with_caption);

    // Mixed content tests
    RUN_TEST(test_mixed_content);
    RUN_TEST(test_paragraph_after_list);

    // Edge cases
    RUN_TEST(test_empty_list);
    RUN_TEST(test_empty_table);
    RUN_TEST(test_deeply_nested_lists);

    return UNITY_END();
}
