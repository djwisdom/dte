#include "bind.h"
#include "change.h"
#include "edit.h"
#include "editor.h"
#include "input-special.h"
#include "terminal/input.h"
#include "util/unicode.h"
#include "view.h"
#include "window.h"

static void insert_paste(void)
{
    size_t size;
    char *text = term_read_paste(&size);

    // Because this is not a command (see run_command()) you have to
    // call begin_change() to avoid merging this change into previous
    begin_change(CHANGE_MERGE_NONE);
    insert_text(text, size);
    end_change();

    free(text);
}

static void normal_mode_keypress(KeyCode key)
{
    char buf[4];
    int count;
    if (special_input_keypress(key, buf, &count)) {
        if (count) {
            begin_change(CHANGE_MERGE_NONE);
            buffer_insert_bytes(buf, count);
            end_change();
            block_iter_skip_bytes(&window->view->cursor, count);
        }
        return;
    }

    if (nr_pressed_keys()) {
        handle_binding(key);
        return;
    }

    switch (key) {
    case '\t':
        if (view->selection == SELECT_LINES) {
            shift_lines(1);
            return;
        }
        break;
    case MOD_SHIFT | '\t':
        if (view->selection == SELECT_LINES) {
            shift_lines(-1);
            return;
        }
        break;
    case KEY_PASTE:
        insert_paste();
        return;
    }

    if (u_is_unicode(key)) {
        insert_ch(key);
    } else {
        handle_binding(key);
    }
}

const EditorModeOps normal_mode_ops = {
    .keypress = normal_mode_keypress,
    .update = normal_update,
};
