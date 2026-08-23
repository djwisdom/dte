#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "showkey.h"
#include "terminal/input.h"
#include "terminal/key.h"
#include "terminal/mode.h"
#include "terminal/output.h"
#include "terminal/paste.h"
#include "terminal/terminal.h"
#include "util/str-util.h"
#include "util/xreadwrite.h"

static bool is_ignored_key(KeyCode key, Terminal *term)
{
    bool bpaste = (key == KEYCODE_BRACKETED_PASTE);
    if (bpaste || key == KEYCODE_DETECTED_PASTE) {
        term_discard_paste(&term->ibuf, bpaste);
        return true;
    }

    return key == KEY_NONE;
}

ExitCode showkey_loop(unsigned int terminal_query_level)
{
    if (!term_mode_init()) {
        return ec_error("tcgetattr", EC_IO_ERROR);
    }
    if (unlikely(!term_raw())) {
        return ec_error("tcsetattr", EC_IO_ERROR);
    }

    Terminal term = {.obuf = term_outbuf()};
    TermOutputBuffer *obuf = &term.obuf;
    term_init(&term, getenv("TERM"), getenv("COLORTERM"));
    term_enable_private_modes(&term);
    term_put_initial_queries(&term, terminal_query_level);
    term_put_literal(obuf, "\rPress any key combination, or use Ctrl+D to exit\r\n");
    term_output_flush(obuf);

    char line[KEYCODE_STR_BUFSIZE + 8];
    for (KeyCode key = KEY_NONE; key != (MOD_CTRL | 'd'); ) {
        key = term_read_input(&term, 100);
        if (is_ignored_key(key, &term)) {
            continue;
        }

        size_t n = 0;
        if (key == KEYCODE_REDRAW) {
            // Move cursor to column 1 (CR) and erase current line (EL),
            // in case the call chain:
            //
            // • term_read_input() →
            // • term_read_input_legacy() →
            // • term_handle_query_reply() →
            // • term_put_level_2_queries()
            //
            // … emitted something that the terminal decided to print,
            // instead of following the ECMA-48 parsing rules.
            n += copyliteral(line + n, "\r\033[K");
        } else {
            n += copyliteral(line + n, "  ");
            n += keycode_to_str(key, line + n);
            n += copyliteral(line + n, "\r\n");
        }

        (void)!xwrite_all(STDOUT_FILENO, line, n);
    }

    term_restore_private_modes(&term);
    term_output_flush(obuf);
    term_cooked();
    return EC_OK;
}
