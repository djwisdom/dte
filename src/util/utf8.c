#include <stdbool.h>
#include <stdint.h>
#include "utf8.h"
#include "ascii.h"
#include "debug.h"
#include "numtostr.h"

enum {
    I = -1, // Invalid byte
    C = 0,  // Continuation byte
};

// https://en.wikipedia.org/wiki/UTF-8#Byte_map
// https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506
static const int8_t seq_len_table[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 00..0F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 10..1F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 20..2F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 30..3F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 40..4F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 50..5F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 60..6F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 70..7F
    C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, // 80..8F
    C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, // 90..9F
    C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, // A0..AF
    C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, // B0..BF
    I, I, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // C0..CF
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // D0..DF
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // E0..EF
    4, 4, 4, 4, 4, I, I, I, I, I, I, I, I, I, I, I  // F0..FF
};

static int u_seq_len(unsigned char first_byte)
{
    int8_t len = seq_len_table[first_byte];
    BUG_ON(len < I || len > UTF8_MAX_SEQ_LEN);
    return len;
}

// https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27288
static bool u_is_continuation_byte(unsigned char u)
{
    // (u & 0b11000000) == 0b10000000
    return (u & 0xC0) == 0x80;
}

// https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506
// https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G31703:~:text=%E2%80%9Cnon%2Dshortest%20form%E2%80%9D
// https://en.wikipedia.org/wiki/UTF-8#Overlong_encodings
// https://en.wikipedia.org/wiki/UTF-8#Error_handling
static bool u_is_overlong_sequence(CodePoint u, size_t seq_len)
{
    BUG_ON(seq_len > UTF8_MAX_SEQ_LEN);
    return u_char_size(u) != seq_len;
}

/*
 * Unicode §3.9.4: "A conformant encoding form conversion will treat any
 * ill-formed code unit sequence as an error condition. (See conformance
 * clause C10.)"
 *
 * C10: "When a process interprets a code unit sequence which purports
 * to be in a Unicode character encoding form, it shall treat ill-formed
 * code unit sequences as an error condition and shall not interpret such
 * sequences as characters."
 *
 * Unicode §3.9.3:
 *
 * • "Before the Unicode Standard, Version 3.1, the problematic “non-shortest
 *   form” byte sequences in UTF-8 were those where BMP characters could be
 *   represented in more than one way. These sequences are ill-formed, because
 *   they are not allowed by Table 3-7."
 * • "Because surrogate code points are not Unicode scalar values, any UTF-8
 *   byte sequence that would otherwise map to code points U+D800..U+DFFF
 *   is ill-formed."
 *
 * See also:
 *
 * • https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G31737
 * • https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G23402
 * • https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G27506
 * • https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G31703
 */
static bool u_is_illformed(CodePoint u, size_t seq_len)
{
    return u_is_overlong_sequence(u, seq_len) || u_is_surrogate(u);
}

/*
 * Len  Mask         Note
 * -------------------------------------------------
 * 1    0111 1111    Not supported by this function!
 * 2    0001 1111
 * 3    0000 1111
 * 4    0000 0111
 * 5    0000 0011    Forbidden by RFC 3629
 * 6    0000 0001    Forbidden by RFC 3629
 */
static unsigned int u_get_first_byte_mask(unsigned int seq_len)
{
    BUG_ON(seq_len < 2 || seq_len > UTF8_MAX_SEQ_LEN);
    return (0x80 >> seq_len) - 1;
}

size_t u_str_width(const unsigned char *str)
{
    size_t i = 0, w = 0;
    while (str[i]) {
        w += u_char_width(u_str_get_char(str, &i));
    }
    return w;
}

CodePoint u_prev_char(const unsigned char *str, size_t *idx)
{
    size_t i = *idx;
    unsigned char ch = str[--i];
    if (likely(ch < 0x80)) {
        *idx = i;
        return (CodePoint)ch;
    }

    if (!u_is_continuation_byte(ch)) {
        goto invalid;
    }

    CodePoint u = ch & 0x3f;
    for (unsigned int count = 1, shift = 6; i > 0; ) {
        ch = str[--i];
        unsigned int len = u_seq_len(ch);
        count++;
        if (len == 0) {
            if (count == 4) {
                // Too long sequence
                break;
            }
            u |= (ch & 0x3f) << shift;
            shift += 6;
        } else if (count != len) {
            // Incorrect length
            break;
        } else {
            u |= (ch & u_get_first_byte_mask(len)) << shift;
            if (u_is_illformed(u, len)) {
                break;
            }
            *idx = i;
            return u;
        }
    }

invalid:
    *idx = *idx - 1;
    u = str[*idx];
    return -u;
}

CodePoint u_get_char(const unsigned char *str, size_t size, size_t *idx)
{
    size_t i = *idx;
    CodePoint u = str[i];
    if (likely(u < 0x80)) {
        *idx = i + 1;
        return u;
    }
    return u_get_nonascii(str, size, idx);
}

CodePoint u_get_nonascii(const unsigned char *str, size_t size, size_t *idx)
{
    size_t i = *idx;
    unsigned int first = str[i++];
    int seq_len = u_seq_len(first);
    if (unlikely(seq_len < 2 || seq_len > size - i + 1)) {
        goto invalid;
    }

    unsigned int count = seq_len - 2;
    CodePoint u = first & u_get_first_byte_mask(seq_len);

    do {
        unsigned char ch = str[i++];
        if (!u_is_continuation_byte(ch)) {
            goto invalid;
        }
        u = (u << 6) | (ch & 0x3f);
    } while (count--);

    if (u_is_illformed(u, seq_len)) {
        goto invalid;
    }

    *idx = i;
    return u;

invalid:
    *idx += 1;
    return -first;
}

size_t u_set_char_raw(char *buf, CodePoint u)
{
    unsigned int prefix = 0;
    size_t len = u_char_size(u);
    BUG_ON(len == 0 || len > UTF8_MAX_SEQ_LEN);

    switch (len) {
    case 4:
        buf[3] = (u & 0x3F) | 0x80;
        u >>= 6;
        prefix |= 0xF0;
        // Fallthrough
    case 3:
        buf[2] = (u & 0x3F) | 0x80;
        u >>= 6;
        prefix |= 0xE0;
        // Fallthrough
    case 2:
        buf[1] = (u & 0x3F) | 0x80;
        u >>= 6;
        prefix |= 0xC0;
    }

    buf[0] = (u & 0xFF) | prefix;
    return len;
}

size_t u_set_char(char *buf, CodePoint u)
{
    if (likely(u <= 0x7F)) {
        size_t i = 0;
        if (unlikely(ascii_iscntrl(u))) {
            // Use caret notation for control chars:
            buf[i++] = '^';
            u = (u + 64) & 0x7F;
        }
        buf[i++] = u;
        return i;
    }

    if (u_is_unprintable(u)) {
        return u_set_hex(buf, u);
    }

    BUG_ON(u > 0x10FFFF); // (implied by !u_is_unprintable(u))
    return u_set_char_raw(buf, u);
}

size_t u_set_hex(char buf[U_SET_HEX_LEN], CodePoint u)
{
    buf[0] = '<';
    if (!u_is_unicode(u)) {
        // Invalid byte (negated)
        u *= -1;
        hex_encode_byte(buf + 1, u & 0xFF);
    } else {
        buf[1] = '?';
        buf[2] = '?';
    }
    buf[3] = '>';
    return U_SET_HEX_LEN;
}

// Return the number of bytes that must be skipped at the start of `str`
// in order to trim at least `skip_width` columns of display width. This
// can be used to e.g. obtain the longest suffix of `str` that can be
// displayed in a given number of columns.
size_t u_skip_chars(const char *str, unsigned int skip_width)
{
    size_t idx = 0;
    for (unsigned int w = 0; str[idx] && w < skip_width; ) {
        w += u_char_width(u_str_get_char(str, &idx));
    }
    return idx;
}
