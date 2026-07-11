#ifndef UTIL_STRTONUM_H
#define UTIL_STRTONUM_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "macros.h"
#include "string-view.h"

extern const uint8_t hex_decode_table[64];

enum {
    HEX_INVALID = 0xF0,
};

// Decodes a single, hexadecimal digit and returns a numerical value
// between 0-15, or HEX_INVALID for invalid digits
static inline unsigned int hex_decode(unsigned char c)
{
    c -= '0'; // Lookup table starts at '0'
    return hex_decode_table[MIN(c, 63)];
}

static inline bool ascii_isxdigit(unsigned char c)
{
    return hex_decode(c) <= 0xF;
}

static inline size_t ascii_hex_prefix_length(StringView str)
{
    size_t i = 0;
    while (i < str.length && ascii_isxdigit(str.data[i])) {
        i++;
    }
    return i;
}

WARN_UNUSED_RESULT
static inline size_t buf_parse_hex_uint(StringView str, unsigned int *valp)
{
    unsigned int val = 0;
    size_t i;
    for (i = 0; i < str.length; i++) {
        unsigned int x = hex_decode(str.data[i]);
        if (unlikely(x > 0xF)) {
            break;
        }
        if (unlikely(val > (UINT_MAX >> 4))) {
            return 0; // Overflow
        }
        val = val << 4 | x;
    }
    *valp = val;
    return i;
}

size_t size_str_width(size_t x) CONST_FN WARN_UNUSED_RESULT;
size_t buf_parse_uintmax(StringView str, uintmax_t *valp) WARN_UNUSED_RESULT WRITEONLY(2);
size_t buf_parse_ulong(StringView str, unsigned long *valp) WARN_UNUSED_RESULT WRITEONLY(2);
size_t buf_parse_uint(StringView str, unsigned int *valp) WARN_UNUSED_RESULT WRITEONLY(2);
size_t buf_parse_size(StringView str, size_t *valp) WARN_UNUSED_RESULT WRITEONLY(2);
bool str_to_int(const char *str, int *valp) NONNULL_ARGS WARN_UNUSED_RESULT WRITEONLY(2);
bool str_to_uint(const char *str, unsigned int *valp) NONNULL_ARGS WARN_UNUSED_RESULT WRITEONLY(2);
bool str_to_size(const char *str, size_t *valp) NONNULL_ARGS WARN_UNUSED_RESULT WRITEONLY(2);
bool str_to_ulong(const char *str, unsigned long *valp) NONNULL_ARGS WARN_UNUSED_RESULT WRITEONLY(2);
bool str_to_uintmax(const char *str, uintmax_t *valp) NONNULL_ARGS WARN_UNUSED_RESULT WRITEONLY(2);
bool str_to_filepos(const char *str, size_t *linep, size_t *colp) NONNULL_ARGS WARN_UNUSED_RESULT WRITEONLY(2) WRITEONLY(3);
bool str_to_xfilepos(StringView sv, size_t *linep, size_t *colp) NONNULL_ARGS WARN_UNUSED_RESULT WRITEONLY(2) WRITEONLY(3);
StringView parse_file_line_col(const char *str, size_t *linep, size_t *colp) NONNULL_ARGS WARN_UNUSED_RESULT WRITEONLY(2) WRITEONLY(3);
intmax_t parse_filesize(const char *str) NONNULL_ARGS WARN_UNUSED_RESULT;

#endif
