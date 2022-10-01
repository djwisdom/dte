#include <stdint.h>
#include "status.h"
#include "search.h"
#include "selection.h"
#include "util/debug.h"
#include "util/str-util.h"
#include "util/utf8.h"
#include "util/xsnprintf.h"

typedef enum {
    STATUS_INVALID = 0,
    STATUS_ESCAPED_PERCENT,
    STATUS_ENCODING,
    STATUS_MISC,
    STATUS_IS_CRLF,
    STATUS_SEPARATOR_LONG,
    STATUS_CURSOR_COL_BYTES,
    STATUS_TOTAL_ROWS,
    STATUS_BOM,
    STATUS_FILENAME,
    STATUS_MODIFIED,
    STATUS_LINE_ENDING,
    STATUS_OVERWRITE,
    STATUS_SCROLL_POSITION,
    STATUS_READONLY,
    STATUS_SEPARATOR,
    STATUS_FILETYPE,
    STATUS_UNICODE,
    STATUS_CURSOR_COL,
    STATUS_CURSOR_ROW,
} FormatSpecifierType;

static const uint8_t format_specifiers[] = {
    ['%'] = STATUS_ESCAPED_PERCENT,
    ['E'] = STATUS_ENCODING,
    ['M'] = STATUS_MISC,
    ['N'] = STATUS_IS_CRLF,
    ['S'] = STATUS_SEPARATOR_LONG,
    ['X'] = STATUS_CURSOR_COL_BYTES,
    ['Y'] = STATUS_TOTAL_ROWS,
    ['b'] = STATUS_BOM,
    ['f'] = STATUS_FILENAME,
    ['m'] = STATUS_MODIFIED,
    ['n'] = STATUS_LINE_ENDING,
    ['o'] = STATUS_OVERWRITE,
    ['p'] = STATUS_SCROLL_POSITION,
    ['r'] = STATUS_READONLY,
    ['s'] = STATUS_SEPARATOR,
    ['t'] = STATUS_FILETYPE,
    ['u'] = STATUS_UNICODE,
    ['x'] = STATUS_CURSOR_COL,
    ['y'] = STATUS_CURSOR_ROW,
};

typedef struct {
    char *buf;
    size_t size;
    size_t pos;
    size_t separator;
    const Window *win;
    const GlobalOptions *opts;
    InputMode input_mode;
} Formatter;

#define add_status_literal(f, s) add_status_bytes(f, s, STRLEN(s))

static void add_ch(Formatter *f, char ch)
{
    f->buf[f->pos++] = ch;
}

static void add_separator(Formatter *f)
{
    while (f->separator && f->pos < f->size) {
        add_ch(f, ' ');
        f->separator--;
    }
}

static void add_status_str(Formatter *f, const char *str)
{
    BUG_ON(!str);
    if (unlikely(!str[0])) {
        return;
    }
    add_separator(f);
    size_t idx = 0;
    while (f->pos < f->size && str[idx]) {
        u_set_char(f->buf, &f->pos, u_str_get_char(str, &idx));
    }
}

static void add_status_bytes(Formatter *f, const char *str, size_t len)
{
    if (unlikely(len == 0)) {
        return;
    }
    add_separator(f);
    if (f->pos >= f->size) {
        return;
    }
    const size_t avail = f->size - f->pos;
    len = MIN(len, avail);
    memcpy(f->buf + f->pos, str, len);
    f->pos += len;
}

PRINTF(2)
static void add_status_format(Formatter *f, const char *format, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, format);
    size_t len = xvsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    add_status_bytes(f, buf, len);
}

static void add_status_pos(Formatter *f)
{
    size_t lines = f->win->view->buffer->nl;
    int h = f->win->edit_h;
    long pos = f->win->view->vy;
    if (lines <= h) {
        if (pos) {
            add_status_literal(f, "Bot");
        } else {
            add_status_literal(f, "All");
        }
    } else if (pos == 0) {
        add_status_literal(f, "Top");
    } else if (pos + h - 1 >= lines) {
        add_status_literal(f, "Bot");
    } else {
        const long d = lines - (h - 1);
        add_status_format(f, "%2ld%%", (pos * 100 + d / 2) / d);
    }
}

static void add_misc_status(Formatter *f)
{
    static const struct {
        const char str[24];
        size_t len;
    } css_strs[] = {
        [CSS_FALSE] = {STRN("[case-sensitive = false]")},
        [CSS_TRUE] = {STRN("[case-sensitive = true]")},
        [CSS_AUTO] = {STRN("[case-sensitive = auto]")},
    };

    if (f->input_mode == INPUT_SEARCH) {
        SearchCaseSensitivity css = f->opts->case_sensitive_search;
        BUG_ON(css >= ARRAYLEN(css_strs));
        add_status_bytes(f, css_strs[css].str, css_strs[css].len);
        return;
    }

    const View *v = f->win->view;
    if (v->selection == SELECT_NONE) {
        return;
    }

    SelectionInfo si;
    init_selection(v, &si);
    bool is_lines = (v->selection == SELECT_LINES);
    const char *unit = is_lines ? "line" : "char";
    size_t n = is_lines ? get_nr_selected_lines(&si) : get_nr_selected_chars(&si);
    add_status_format(f, "[%zu %s%s]", n, unit, likely(n != 1) ? "s" : "");
}

static FormatSpecifierType lookup_format_specifier(unsigned char ch)
{
    if (unlikely(ch >= ARRAYLEN(format_specifiers))) {
        return STATUS_INVALID;
    }
    return format_specifiers[ch];
}

UNITTEST {
    BUG_ON(lookup_format_specifier('%') != STATUS_ESCAPED_PERCENT);
    BUG_ON(lookup_format_specifier('E') != STATUS_ENCODING);
    BUG_ON(lookup_format_specifier('M') != STATUS_MISC);
    BUG_ON(lookup_format_specifier('y') != STATUS_CURSOR_ROW);
    BUG_ON(lookup_format_specifier('z') != STATUS_INVALID);
    BUG_ON(lookup_format_specifier('~') != STATUS_INVALID);
    BUG_ON(lookup_format_specifier('@') != STATUS_INVALID);
    BUG_ON(lookup_format_specifier('?') != STATUS_INVALID);
    BUG_ON(lookup_format_specifier(0x00) != STATUS_INVALID);
    BUG_ON(lookup_format_specifier(0x80) != STATUS_INVALID);
    BUG_ON(lookup_format_specifier(0xFF) != STATUS_INVALID);
}

void sf_format (
    const Window *w,
    const GlobalOptions *opts,
    InputMode mode,
    char *buf,
    size_t size,
    const char *format
) {
    BUG_ON(size < 16);
    Formatter f = {
        .win = w,
        .opts = opts,
        .input_mode = mode,
        .buf = buf,
        .size = size - 5, // Max length of char and terminating NUL
    };

    const View *v = w->view;
    CodePoint u;

    while (f.pos < f.size && *format) {
        unsigned char ch = *format++;
        if (ch != '%') {
            add_separator(&f);
            add_ch(&f, ch);
            continue;
        }

        switch (lookup_format_specifier(*format++)) {
        case STATUS_BOM:
            if (v->buffer->bom) {
                add_status_literal(&f, "BOM");
            }
            break;
        case STATUS_FILENAME:
            add_status_str(&f, buffer_filename(v->buffer));
            break;
        case STATUS_MODIFIED:
            if (buffer_modified(v->buffer)) {
                add_separator(&f);
                add_ch(&f, '*');
            }
            break;
        case STATUS_READONLY:
            if (v->buffer->readonly) {
                add_status_literal(&f, "RO");
            } else if (v->buffer->temporary) {
                add_status_literal(&f, "TMP");
            }
            break;
        case STATUS_CURSOR_ROW:
            add_status_format(&f, "%ld", v->cy + 1);
            break;
        case STATUS_TOTAL_ROWS:
            add_status_format(&f, "%zu", v->buffer->nl);
            break;
        case STATUS_CURSOR_COL:
            add_status_format(&f, "%ld", v->cx_display + 1);
            break;
        case STATUS_CURSOR_COL_BYTES:
            add_status_format(&f, "%ld", v->cx_char + 1);
            if (v->cx_display != v->cx_char) {
                add_status_format(&f, "-%ld", v->cx_display + 1);
            }
            break;
        case STATUS_SCROLL_POSITION:
            add_status_pos(&f);
            break;
        case STATUS_ENCODING:
            add_status_str(&f, v->buffer->encoding.name);
            break;
        case STATUS_MISC:
            add_misc_status(&f);
            break;
        case STATUS_IS_CRLF:
            if (v->buffer->crlf_newlines) {
                add_status_literal(&f, "CRLF");
            }
            break;
        case STATUS_LINE_ENDING:
            if (v->buffer->crlf_newlines) {
                add_status_literal(&f, "CRLF");
            } else {
                add_status_literal(&f, "LF");
            }
            break;
        case STATUS_OVERWRITE:
            if (v->buffer->options.overwrite) {
                add_status_literal(&f, "OVR");
            } else {
                add_status_literal(&f, "INS");
            }
            break;
        case STATUS_SEPARATOR_LONG:
            f.separator = 3;
            break;
        case STATUS_SEPARATOR:
            f.separator = 1;
            break;
        case STATUS_FILETYPE:
            add_status_str(&f, v->buffer->options.filetype);
            break;
        case STATUS_UNICODE:
            if (unlikely(!block_iter_get_char(&v->cursor, &u))) {
                break;
            }
            if (u_is_unicode(u)) {
                add_status_format(&f, "U+%04X", u);
            } else {
                add_status_literal(&f, "Invalid");
            }
            break;
        case STATUS_ESCAPED_PERCENT:
            add_separator(&f);
            add_ch(&f, '%');
            break;
        case STATUS_INVALID:
        default:
            BUG("should be unreachable, due to validate_statusline_format()");
        }
    }

    f.buf[f.pos] = '\0';
}

// Returns the offset of the first invalid format specifier, or 0 if
// the whole format string is valid. It's safe to use 0 to indicate
// "no errors", since it's not possible for there to be an error at
// the very start of the string.
size_t statusline_format_find_error(const char *str)
{
    for (size_t i = 0; str[i]; ) {
        if (str[i++] != '%') {
            continue;
        }
        if (lookup_format_specifier(str[i++]) == STATUS_INVALID) {
            return i - 1;
        }
    }
    return 0;
}