#include "ui.h"
#include "command/error.h"

static void print_message(Terminal *term, const StyleMap *styles, const char *msg, bool is_error)
{
    BuiltinStyleEnum c = BSE_COMMANDLINE;
    if (msg[0]) {
        c = is_error ? BSE_ERRORMSG : BSE_INFOMSG;
    }

    TermOutputBuffer *obuf = &term->obuf;
    set_builtin_style(term, styles, c);

    for (size_t i = 0; msg[i]; ) {
        CodePoint u = u_get_char(msg, i + 4, &i);
        if (!term_put_char(obuf, u)) {
            break;
        }
    }
}

static size_t print_command(Terminal *term, const StyleMap *styles, const CommandLine *cmdline, char prefix)
{
    const String *buf = &cmdline->buf;
    TermOutputBuffer *obuf = &term->obuf;

    // Width of characters up to and including cursor position
    size_t w = 1; // ":" (prefix)

    for (size_t i = 0; i <= cmdline->pos && i < buf->len; ) {
        CodePoint u = u_get_char(buf->buffer, buf->len, &i);
        w += u_char_width(u);
    }
    if (cmdline->pos == buf->len) {
        w++;
    }
    if (w > term->width) {
        obuf->scroll_x = w - term->width;
    }

    set_builtin_style(term, styles, BSE_COMMANDLINE);
    term_put_char(obuf, prefix);

    size_t x = obuf->x - obuf->scroll_x;
    for (size_t i = 0; i < buf->len; ) {
        BUG_ON(obuf->x > obuf->scroll_x + obuf->width);
        CodePoint u = u_get_char(buf->buffer, buf->len, &i);
        if (!term_put_char(obuf, u)) {
            break;
        }
        if (i <= cmdline->pos) {
            x = obuf->x - obuf->scroll_x;
        }
    }

    return x;
}

size_t update_command_line (
    Terminal *term,
    const ErrorBuffer *err,
    const StyleMap *styles,
    const CommandLine *cmdline,
    const SearchState *search,
    const CommandSet *cmds
) {
    term_output_reset(term, 0, term->width, 0);
    term_move_cursor(&term->obuf, 0, term->height - 1);

    size_t cmdline_x = 0;
    if (cmds == &normal_commands) {
        print_message(term, styles, err->buf, err->is_error);
    } else {
        bool search_mode = (cmds == &search_mode_commands);
        char prefix = search_mode ? (search->reverse ? '?' : '/') : ':';
        cmdline_x = print_command(term, styles, cmdline, prefix);
    }

    term_clear_eol(term);
    return cmdline_x;
}
