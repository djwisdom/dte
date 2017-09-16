#include "editor.h"
#include "common.h"
#include "path.h"
#include "encoding.h"

#include <locale.h>
#include <langinfo.h>

static void fail(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

static void test_relative_filename(void)
{
    static const struct rel_test {
        const char *cwd;
        const char *path;
        const char *result;
    } tests[] = {
        // NOTE: at most 2 ".." components allowed in relative name
        { "/", "/", "/" },
        { "/", "/file", "file" },
        { "/a/b/c/d", "/a/b/file", "../../file" },
        { "/a/b/c/d/e", "/a/b/file", "/a/b/file" }, // "../../../file" contains too many ".." components
        { "/a/foobar", "/a/foo/file", "../foo/file" },
    };

    for (int i = 0; i < ARRAY_COUNT(tests); i++) {
        const struct rel_test *t = &tests[i];
        char *result = relative_filename(t->path, t->cwd);
        if (!streq(t->result, result)) {
            fail("relative_filename(%s, %s) -> %s, expected %s\n", t->path, t->cwd, result, t->result);
        }
        free(result);
    }
}

static void test_detect_encoding_from_bom(void)
{
    static const struct bom_test {
        const char *encoding;
        const unsigned char *text;
        size_t size;
    } tests[] = {
        {"UTF-8", "\xef\xbb\xbfHello", 8},
        {"UTF-32BE", "\x00\x00\xfe\xffHello", 9},
        {"UTF-32LE", "\xff\xfe\x00\x00Hello", 9},
        {"UTF-16BE", "\xfe\xffHello", 7},
        {"UTF-16LE", "\xff\xfeHello", 7},
    };

    for (size_t i = 0; i < ARRAY_COUNT(tests); i++) {
        const struct bom_test *t = &tests[i];
        const char *result = detect_encoding_from_bom(t->text, t->size);
        if (!result || !streq(result, t->encoding)) {
            fail (
                "%s: test #%zd failed: got %s, expected %s\n",
                __func__, i + 1, result ? result : "(null)", t->encoding
            );
        }
    }
}

int main(int argc, char *argv[])
{
    const char *home = getenv("HOME");

    if (!home) {
        home = "";
    }
    editor.home_dir = xstrdup(home);

    setlocale(LC_CTYPE, "");
    editor.charset = nl_langinfo(CODESET);
    if (streq(editor.charset, "UTF-8")) {
        term_utf8 = true;
    }

    test_relative_filename();
    test_detect_encoding_from_bom();
    return 0;
}
