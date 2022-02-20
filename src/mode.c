#include "mode.h"
#include "bind.h"
#include "buffer.h"
#include "change.h"
#include "cmdline.h"
#include "command/macro.h"
#include "completion.h"
#include "history.h"
#include "misc.h"
#include "search.h"
#include "shift.h"
#include "terminal/input.h"
#include "util/debug.h"
#include "util/str-util.h"
#include "util/unicode.h"
#include "view.h"

static void normal_mode_keypress(EditorState *e, KeyCode key)
{
    View *view = e->view;
    switch (key) {
    case KEY_TAB:
        if (view->selection == SELECT_LINES) {
            shift_lines(view, 1);
            return;
        }
        break;
    case MOD_SHIFT | KEY_TAB:
        if (view->selection == SELECT_LINES) {
            shift_lines(view, -1);
            return;
        }
        break;
    case KEY_PASTE: {
        size_t size;
        char *text = term_read_paste(&size);
        begin_change(CHANGE_MERGE_NONE);
        insert_text(view, text, size, true);
        end_change();
        macro_insert_text_hook(text, size);
        free(text);
        return;
        }
    }

    if (u_is_unicode(key)) {
        insert_ch(view, key);
        macro_insert_char_hook(key);
    } else {
        handle_binding(&e->bindings[INPUT_NORMAL], key);
    }
}

static void cmdline_insert_paste(CommandLine *c)
{
    size_t size;
    char *text = term_read_paste(&size);
    strn_replace_byte(text, size, '\n', ' ');
    string_insert_buf(&c->buf, c->pos, text, size);
    c->pos += size;
    free(text);
}

void handle_input(EditorState *e, KeyCode key)
{
    InputMode mode = e->input_mode;
    if (mode == INPUT_NORMAL) {
        normal_mode_keypress(e, key);
        return;
    }

    BUG_ON(mode != INPUT_COMMAND && mode != INPUT_SEARCH);
    CommandLine *c = &e->cmdline;
    if (key == KEY_PASTE) {
        cmdline_insert_paste(c);
        c->search_pos = NULL;
        goto reset;
    }

    if (u_is_unicode(key) && key != KEY_TAB && key != KEY_ENTER) {
        c->pos += string_insert_ch(&c->buf, c->pos, key);
        goto reset;
    }

    handle_binding(&e->bindings[mode], key);
    return;

reset:
    if (mode == INPUT_COMMAND) {
        reset_completion(c);
    }
}
