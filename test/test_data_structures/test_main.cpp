// Unit tests for data structures used in HTML parsing
// Tests ListContext, TableCell, TableRow, TableData structures
// These don't require the full renderer/display stack

#include <unity.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <memory>

// ============================================================================
// Test versions of the structures from ChapterHtmlSlimParser.h
// ============================================================================

// Constants for HTML element rendering
constexpr int LIST_INDENT_PX = 24;
constexpr int MAX_LIST_NESTING = 6;
constexpr int BLOCKQUOTE_INDENT_PX = 32;

// Font style enum
enum Style : uint8_t { REGULAR = 0, BOLD = 1, ITALIC = 2, BOLD_ITALIC = 3 };

// List context for tracking ordered/unordered lists
struct ListContext {
    bool isOrdered;
    int itemNumber;
};

// Table cell for collecting table content during parsing
struct TableCell {
    std::string text;
    Style style = REGULAR;
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

// ============================================================================
// Test Setup/Teardown
// ============================================================================

void setUp() {}
void tearDown() {}

// ============================================================================
// ListContext Tests
// ============================================================================

void test_list_context_unordered() {
    ListContext ctx = {false, 0};
    TEST_ASSERT_FALSE(ctx.isOrdered);
    TEST_ASSERT_EQUAL(0, ctx.itemNumber);
}

void test_list_context_ordered() {
    ListContext ctx = {true, 1};
    TEST_ASSERT_TRUE(ctx.isOrdered);
    TEST_ASSERT_EQUAL(1, ctx.itemNumber);
}

void test_list_context_increment() {
    ListContext ctx = {true, 1};
    ctx.itemNumber++;
    TEST_ASSERT_EQUAL(2, ctx.itemNumber);
    ctx.itemNumber++;
    TEST_ASSERT_EQUAL(3, ctx.itemNumber);
}

void test_list_nesting_stack() {
    std::vector<ListContext> listStack;

    // Push unordered list
    listStack.push_back({false, 0});
    TEST_ASSERT_EQUAL(1, listStack.size());

    // Push nested ordered list
    listStack.push_back({true, 1});
    TEST_ASSERT_EQUAL(2, listStack.size());

    // Verify top is the ordered list
    TEST_ASSERT_TRUE(listStack.back().isOrdered);

    // Pop back to unordered
    listStack.pop_back();
    TEST_ASSERT_EQUAL(1, listStack.size());
    TEST_ASSERT_FALSE(listStack.back().isOrdered);
}

void test_list_indent_calculation() {
    // Test indent calculation for various nesting levels
    int indent1 = LIST_INDENT_PX * 1;
    int indent2 = LIST_INDENT_PX * 2;
    int indent3 = LIST_INDENT_PX * 3;

    TEST_ASSERT_EQUAL(24, indent1);
    TEST_ASSERT_EQUAL(48, indent2);
    TEST_ASSERT_EQUAL(72, indent3);

    // Test max nesting
    int maxIndent = LIST_INDENT_PX * MAX_LIST_NESTING;
    TEST_ASSERT_EQUAL(144, maxIndent);
}

// ============================================================================
// TableCell Tests
// ============================================================================

void test_table_cell_default() {
    TableCell cell;
    TEST_ASSERT_TRUE(cell.text.empty());
    TEST_ASSERT_EQUAL(REGULAR, cell.style);
}

void test_table_cell_with_text() {
    TableCell cell;
    cell.text = "Hello";
    TEST_ASSERT_EQUAL_STRING("Hello", cell.text.c_str());
}

void test_table_cell_with_style() {
    TableCell cell;
    cell.text = "Bold Header";
    cell.style = BOLD;
    TEST_ASSERT_EQUAL_STRING("Bold Header", cell.text.c_str());
    TEST_ASSERT_EQUAL(BOLD, cell.style);
}

void test_table_cell_append() {
    TableCell cell;
    cell.text += "First ";
    cell.text += "Second";
    TEST_ASSERT_EQUAL_STRING("First Second", cell.text.c_str());
}

// ============================================================================
// TableRow Tests
// ============================================================================

void test_table_row_empty() {
    TableRow row;
    TEST_ASSERT_TRUE(row.cells.empty());
    TEST_ASSERT_EQUAL(0, row.cells.size());
}

void test_table_row_single_cell() {
    TableRow row;
    TableCell cell;
    cell.text = "A";
    row.cells.push_back(cell);

    TEST_ASSERT_EQUAL(1, row.cells.size());
    TEST_ASSERT_EQUAL_STRING("A", row.cells[0].text.c_str());
}

void test_table_row_multiple_cells() {
    TableRow row;

    TableCell cell1, cell2, cell3;
    cell1.text = "Name";
    cell1.style = BOLD;
    cell2.text = "Age";
    cell2.style = BOLD;
    cell3.text = "City";
    cell3.style = BOLD;

    row.cells.push_back(cell1);
    row.cells.push_back(cell2);
    row.cells.push_back(cell3);

    TEST_ASSERT_EQUAL(3, row.cells.size());
    TEST_ASSERT_EQUAL_STRING("Name", row.cells[0].text.c_str());
    TEST_ASSERT_EQUAL_STRING("Age", row.cells[1].text.c_str());
    TEST_ASSERT_EQUAL_STRING("City", row.cells[2].text.c_str());
}

// ============================================================================
// TableData Tests
// ============================================================================

void test_table_data_initial_state() {
    TableData table;
    TEST_ASSERT_TRUE(table.rows.empty());
    TEST_ASSERT_EQUAL(-1, table.currentRow);
    TEST_ASSERT_EQUAL(-1, table.currentCell);
    TEST_ASSERT_FALSE(table.inCell);
}

void test_table_data_start_row() {
    TableData table;

    // Simulate starting a new row
    table.rows.push_back(TableRow());
    table.currentRow = 0;
    table.currentCell = -1;

    TEST_ASSERT_EQUAL(1, table.rows.size());
    TEST_ASSERT_EQUAL(0, table.currentRow);
}

void test_table_data_add_cell() {
    TableData table;

    // Start row
    table.rows.push_back(TableRow());
    table.currentRow = 0;

    // Start cell
    table.rows[0].cells.push_back(TableCell());
    table.currentCell = 0;
    table.inCell = true;

    // Add text to cell
    table.rows[0].cells[0].text += "Content";

    TEST_ASSERT_EQUAL(1, table.rows.size());
    TEST_ASSERT_EQUAL(1, table.rows[0].cells.size());
    TEST_ASSERT_EQUAL_STRING("Content", table.rows[0].cells[0].text.c_str());
}

void test_table_data_complete_table() {
    TableData table;

    // Row 1: Header
    table.rows.push_back(TableRow());
    table.currentRow = 0;

    TableCell h1, h2;
    h1.text = "Name";
    h1.style = BOLD;
    h2.text = "Value";
    h2.style = BOLD;
    table.rows[0].cells.push_back(h1);
    table.rows[0].cells.push_back(h2);

    // Row 2: Data
    table.rows.push_back(TableRow());
    table.currentRow = 1;

    TableCell d1, d2;
    d1.text = "Width";
    d2.text = "100";
    table.rows[1].cells.push_back(d1);
    table.rows[1].cells.push_back(d2);

    // Verify structure
    TEST_ASSERT_EQUAL(2, table.rows.size());
    TEST_ASSERT_EQUAL(2, table.rows[0].cells.size());
    TEST_ASSERT_EQUAL(2, table.rows[1].cells.size());

    // Verify content
    TEST_ASSERT_EQUAL_STRING("Name", table.rows[0].cells[0].text.c_str());
    TEST_ASSERT_EQUAL(BOLD, table.rows[0].cells[0].style);
    TEST_ASSERT_EQUAL_STRING("Width", table.rows[1].cells[0].text.c_str());
    TEST_ASSERT_EQUAL(REGULAR, table.rows[1].cells[0].style);
}

// ============================================================================
// Blockquote Nesting Tests
// ============================================================================

void test_blockquote_indent() {
    int depth1 = BLOCKQUOTE_INDENT_PX * 1;
    int depth2 = BLOCKQUOTE_INDENT_PX * 2;
    int depth3 = BLOCKQUOTE_INDENT_PX * 3;

    TEST_ASSERT_EQUAL(32, depth1);
    TEST_ASSERT_EQUAL(64, depth2);
    TEST_ASSERT_EQUAL(96, depth3);
}

void test_blockquote_depth_tracking() {
    int blockquoteDepth = 0;

    // Enter blockquote
    blockquoteDepth++;
    TEST_ASSERT_EQUAL(1, blockquoteDepth);

    // Nested blockquote
    blockquoteDepth++;
    TEST_ASSERT_EQUAL(2, blockquoteDepth);

    // Exit inner
    blockquoteDepth--;
    TEST_ASSERT_EQUAL(1, blockquoteDepth);

    // Exit outer
    blockquoteDepth--;
    TEST_ASSERT_EQUAL(0, blockquoteDepth);
}

// ============================================================================
// Bullet Character Tests
// ============================================================================

void test_bullet_character_utf8() {
    // The bullet character used for unordered lists
    const char* bullet = "\u2022";  // BULLET (•)
    TEST_ASSERT_EQUAL_STRING("\u2022", bullet);

    // Verify it's multi-byte UTF-8
    TEST_ASSERT_EQUAL(3, strlen(bullet));  // UTF-8 encoded bullet is 3 bytes
}

void test_ordered_list_numbering() {
    // Test generating numbered prefixes
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%d.", 1);
    TEST_ASSERT_EQUAL_STRING("1.", buffer);

    snprintf(buffer, sizeof(buffer), "%d.", 10);
    TEST_ASSERT_EQUAL_STRING("10.", buffer);

    snprintf(buffer, sizeof(buffer), "%d.", 99);
    TEST_ASSERT_EQUAL_STRING("99.", buffer);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // ListContext tests
    RUN_TEST(test_list_context_unordered);
    RUN_TEST(test_list_context_ordered);
    RUN_TEST(test_list_context_increment);
    RUN_TEST(test_list_nesting_stack);
    RUN_TEST(test_list_indent_calculation);

    // TableCell tests
    RUN_TEST(test_table_cell_default);
    RUN_TEST(test_table_cell_with_text);
    RUN_TEST(test_table_cell_with_style);
    RUN_TEST(test_table_cell_append);

    // TableRow tests
    RUN_TEST(test_table_row_empty);
    RUN_TEST(test_table_row_single_cell);
    RUN_TEST(test_table_row_multiple_cells);

    // TableData tests
    RUN_TEST(test_table_data_initial_state);
    RUN_TEST(test_table_data_start_row);
    RUN_TEST(test_table_data_add_cell);
    RUN_TEST(test_table_data_complete_table);

    // Blockquote tests
    RUN_TEST(test_blockquote_indent);
    RUN_TEST(test_blockquote_depth_tracking);

    // Bullet/numbering tests
    RUN_TEST(test_bullet_character_utf8);
    RUN_TEST(test_ordered_list_numbering);

    return UNITY_END();
}
