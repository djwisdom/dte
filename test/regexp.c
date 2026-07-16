#include <stdlib.h>
#include "test.h"
#include "regexp.h"

static void test_regexp_escape(TestContext *ctx)
{
    static const char pat[] = "^([a-z]+|0-9{3,6}|\\.|-?.*)$";
    ASSERT_TRUE(regexp_is_valid(NULL, pat, REG_NEWLINE));
    String escaped = regexp_escape(strview(pat));

    EXPECT_STRING_EQ_CSTRING (
        &escaped,
        "\\^\\(\\[a-z]\\+\\|0-9\\{3,6}\\|\\\\\\.\\|-\\?\\.\\*\\)\\$"
    );

    // Ensure the escaped pattern matches the original pattern string
    const char *cstr = string_borrow_cstring(&escaped);
    regex_t re;
    ASSERT_TRUE(regexp_compile(NULL, &re, cstr, REG_NEWLINE | REG_NOSUB));
    string_free(&escaped);
    EXPECT_TRUE(regexp_exec(&re, strview(pat), 0, NULL, 0));
    regfree(&re);
}

static const TestEntry tests[] = {
    TEST(test_regexp_escape),
};

const TestGroup regexp_tests = TEST_GROUP(tests);
