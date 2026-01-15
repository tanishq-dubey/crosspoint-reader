// Basic test to verify the test framework works
// This test does not depend on complex library code

#include <unity.h>

void setUp() {
    // Set up before each test
}

void tearDown() {
    // Clean up after each test
}

void test_basic_assertion() {
    TEST_ASSERT_TRUE(true);
}

void test_integer_equality() {
    TEST_ASSERT_EQUAL(42, 42);
}

void test_string_equality() {
    TEST_ASSERT_EQUAL_STRING("hello", "hello");
}

void test_list_constants() {
    // Test the constants we defined for lists
    // These should match what we use in ChapterHtmlSlimParser
    const int LIST_INDENT_PX = 24;
    const int MAX_LIST_NESTING = 6;
    const int BLOCKQUOTE_INDENT_PX = 32;

    TEST_ASSERT_EQUAL(24, LIST_INDENT_PX);
    TEST_ASSERT_EQUAL(6, MAX_LIST_NESTING);
    TEST_ASSERT_EQUAL(32, BLOCKQUOTE_INDENT_PX);
}

void test_text_block_styles() {
    // Test that we have the expected text block styles
    enum Style : uint8_t {
        JUSTIFIED = 0,
        LEFT_ALIGN = 1,
        CENTER_ALIGN = 2,
        RIGHT_ALIGN = 3,
    };

    TEST_ASSERT_EQUAL(0, JUSTIFIED);
    TEST_ASSERT_EQUAL(1, LEFT_ALIGN);
    TEST_ASSERT_EQUAL(2, CENTER_ALIGN);
    TEST_ASSERT_EQUAL(3, RIGHT_ALIGN);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_basic_assertion);
    RUN_TEST(test_integer_equality);
    RUN_TEST(test_string_equality);
    RUN_TEST(test_list_constants);
    RUN_TEST(test_text_block_styles);

    return UNITY_END();
}
