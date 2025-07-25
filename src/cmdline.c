#include <stdlib.h>
#include <string.h>
#include "cmdline.h"
#include "command/args.h"
#include "command/macro.h"
#include "commands.h"
#include "completion.h"
#include "copy.h"
#include "editor.h"
#include "history.h"
#include "options.h"
#include "regexp.h"
#include "search.h"
#include "selection.h"
#include "terminal/osc52.h"
#include "util/arith.h"
#include "util/ascii.h"
#include "util/bsearch.h"
#include "util/debug.h"
#include "util/log.h"
#include "util/utf8.h"

static void cmdline_delete(CommandLine *c)
{
    size_t pos = c->pos;
    if (pos == c->buf.len) {
        return;
    }

    u_get_char(c->buf.buffer, c->buf.len, &pos);
    string_remove(&c->buf, c->pos, pos - c->pos);
}

void cmdline_clear(CommandLine *c)
{
    string_clear(&c->buf);
    c->pos = 0;
    c->search_pos = NULL;
}

void cmdline_free(CommandLine *c)
{
    cmdline_clear(c);
    string_free(&c->buf);
    free(c->search_text);
    reset_completion(c);
}

static void set_text(CommandLine *c, const char *text)
{
    size_t text_len = strlen(text);
    c->pos = text_len;
    string_clear(&c->buf);
    string_append_buf(&c->buf, text, text_len);
}

void cmdline_set_text(CommandLine *c, const char *text)
{
    c->search_pos = NULL;
    set_text(c, text);
}

// Reset command completion and history search state (after cursor
// position or buffer is changed)
static bool cmdline_soft_reset(CommandLine *c)
{
    c->search_pos = NULL;
    maybe_reset_completion(c);
    return true;
}

static bool cmd_bol(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    e->cmdline.pos = 0;
    return cmdline_soft_reset(&e->cmdline);
}

static bool cmd_cancel(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    cmdline_clear(c);
    pop_input_mode(e);
    reset_completion(c);
    return true;
}

static bool cmd_clear(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    cmdline_clear(&e->cmdline);
    return true;
}

static bool cmd_copy(EditorState *e, const CommandArgs *a)
{
    static const FlagMapping map[] = {
        {'b', TCOPY_CLIPBOARD},
        {'p', TCOPY_PRIMARY},
    };

    Terminal *term = &e->terminal;
    TermCopyFlags flags = cmdargs_convert_flags(a, map, ARRAYLEN(map));
    bool internal = cmdargs_has_flag(a, 'i') || flags == 0;
    bool osc52 = flags && (term->features & TFLAG_OSC52_COPY);
    String *buf = &e->cmdline.buf;
    size_t len = buf->len;

    if (internal) {
        char *str = string_clone_cstring(buf);
        record_copy(&e->clipboard, str, len, false);
    }

    if (osc52) {
        bool ok = term_osc52_copy(&term->obuf, strview_from_string(buf), flags);
        LOG_ERRNO_ON(!ok, "term_osc52_copy");
    }

    return true;
}

static bool cmd_delete(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    cmdline_delete(c);
    return cmdline_soft_reset(c);
}

static bool cmd_delete_eol(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    c->buf.len = c->pos;
    return cmdline_soft_reset(c);
}

static bool cmd_delete_word(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    const unsigned char *buf = c->buf.buffer;
    const size_t len = c->buf.len;
    size_t i = c->pos;

    if (i == len) {
        return true;
    }

    while (i < len && is_word_byte(buf[i])) {
        i++;
    }

    while (i < len && !is_word_byte(buf[i])) {
        i++;
    }

    string_remove(&c->buf, c->pos, i - c->pos);
    return cmdline_soft_reset(c);
}

static bool cmd_eol(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    c->pos = c->buf.len;
    return cmdline_soft_reset(c);
}

static bool cmd_erase(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    if (c->pos > 0) {
        u_prev_char(c->buf.buffer, &c->pos);
        cmdline_delete(c);
    }
    return cmdline_soft_reset(c);
}

static bool cmd_erase_bol(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    string_remove(&c->buf, 0, c->pos);
    c->pos = 0;
    return cmdline_soft_reset(c);
}

static bool cmd_erase_word(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    size_t i = c->pos;
    if (i == 0) {
        return true;
    }

    // open /path/to/file^W => open /path/to/

    // erase whitespace
    while (i && ascii_isspace(c->buf.buffer[i - 1])) {
        i--;
    }

    // erase non-word bytes
    while (i && !is_word_byte(c->buf.buffer[i - 1])) {
        i--;
    }

    // erase word bytes
    while (i && is_word_byte(c->buf.buffer[i - 1])) {
        i--;
    }

    string_remove(&c->buf, i, c->pos - i);
    c->pos = i;
    return cmdline_soft_reset(c);
}

static bool do_history_prev(const History *hist, CommandLine *c, bool prefix_search)
{
    if (!c->search_pos) {
        free(c->search_text);
        c->search_text = string_clone_cstring(&c->buf);
    }

    const char *search_text = prefix_search ? c->search_text : "";
    if (history_search_forward(hist, &c->search_pos, search_text)) {
        BUG_ON(!c->search_pos);
        set_text(c, c->search_pos->text);
    }

    maybe_reset_completion(c);
    return true;
}

static bool do_history_next(const History *hist, CommandLine *c, bool prefix_search)
{
    if (!c->search_pos) {
        goto out;
    }

    const char *search_text = prefix_search ? c->search_text : "";
    if (history_search_backward(hist, &c->search_pos, search_text)) {
        BUG_ON(!c->search_pos);
        set_text(c, c->search_pos->text);
    } else {
        set_text(c, c->search_text);
        c->search_pos = NULL;
    }

out:
    maybe_reset_completion(c);
    return true;
}

static bool cmd_search_history_next(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    bool prefix_search = !cmdargs_has_flag(a, 'S');
    return do_history_next(&e->search_history, &e->cmdline, prefix_search);
}

static bool cmd_search_history_prev(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    bool prefix_search = !cmdargs_has_flag(a, 'S');
    return do_history_prev(&e->search_history, &e->cmdline, prefix_search);
}

static bool cmd_command_history_next(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    bool prefix_search = !cmdargs_has_flag(a, 'S');
    return do_history_next(&e->command_history, &e->cmdline, prefix_search);
}

static bool cmd_command_history_prev(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    bool prefix_search = !cmdargs_has_flag(a, 'S');
    return do_history_prev(&e->command_history, &e->cmdline, prefix_search);
}

static bool cmd_left(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    if (c->pos) {
        u_prev_char(c->buf.buffer, &c->pos);
    }
    return cmdline_soft_reset(c);
}

static bool cmd_paste(EditorState *e, const CommandArgs *a)
{
    const Clipboard *clip = &e->clipboard;
    const size_t len = clip->len;
    if (len == 0) {
        return true;
    }

    CommandLine *c = &e->cmdline;
    char newline_replacement = cmdargs_has_flag(a, 'n') ? ';' : ' ';
    string_insert_buf(&c->buf, c->pos, clip->buf, len);
    strn_replace_byte(c->buf.buffer + c->pos, len, '\n', newline_replacement);
    c->pos += cmdargs_has_flag(a, 'm') ? len : 0;
    return cmdline_soft_reset(c);
}

static bool cmd_right(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    if (c->pos < c->buf.len) {
        u_get_char(c->buf.buffer, c->buf.len, &c->pos);
    }
    return cmdline_soft_reset(c);
}

static bool cmd_toggle(EditorState *e, const CommandArgs *a)
{
    const char *option_name = a->args[0];
    bool global = cmdargs_has_flag(a, 'g');
    size_t nr_values = a->nr_args - 1;
    if (nr_values == 0) {
        return toggle_option(e, option_name, global, false);
    }

    char **values = a->args + 1;
    return toggle_option_values(e, option_name, global, false, values, nr_values);
}

static bool cmd_word_bwd(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    if (c->pos <= 1) {
        c->pos = 0;
        return cmdline_soft_reset(c);
    }

    const unsigned char *const buf = c->buf.buffer;
    size_t i = c->pos - 1;

    while (i > 0 && !is_word_byte(buf[i])) {
        i--;
    }

    while (i > 0 && is_word_byte(buf[i])) {
        i--;
    }

    if (i > 0) {
        i++;
    }

    c->pos = i;
    return cmdline_soft_reset(c);
}

static bool cmd_word_fwd(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    CommandLine *c = &e->cmdline;
    const unsigned char *buf = c->buf.buffer;
    const size_t len = c->buf.len;
    size_t i = c->pos;

    while (i < len && is_word_byte(buf[i])) {
        i++;
    }

    while (i < len && !is_word_byte(buf[i])) {
        i++;
    }

    c->pos = i;
    return cmdline_soft_reset(c);
}

static bool cmd_complete_next(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    complete_command_next(e);
    return true;
}

static bool cmd_complete_prev(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    complete_command_prev(e);
    return true;
}

static bool cmd_direction(EditorState *e, const CommandArgs *a)
{
    BUG_ON(a->nr_args);
    toggle_search_direction(&e->search);
    return true;
}

static bool cmd_command_mode_accept(EditorState *e, const CommandArgs *a)
{
    CommandLine *c = &e->cmdline;
    reset_completion(c);
    pop_input_mode(e);

    const char *str = string_borrow_cstring(&c->buf);
    cmdline_clear(c);
    if (!cmdargs_has_flag(a, 'H') && str[0] != ' ') {
        // This is done before handle_command() because "command [text]"
        // can modify the contents of the command-line
        history_append(&e->command_history, str);
    }

    e->err.command_name = NULL;
    return handle_normal_command(e, str, true);
}

static bool cmd_search_mode_accept(EditorState *e, const CommandArgs *a)
{
    CommandLine *c = &e->cmdline;
    bool add_to_history = !cmdargs_has_flag(a, 'H');
    const char *pat = NULL;

    if (c->buf.len > 0) {
        String *s = &c->buf;
        if (cmdargs_has_flag(a, 'e')) {
            // Escape the regex; to match as plain text
            char *original = string_clone_cstring(s);
            size_t origlen = string_clear(s);
            size_t bufsize = xmul(2, origlen) + 1;
            char *buf = string_reserve_space(s, bufsize);
            s->len = regexp_escapeb(buf, bufsize, original, origlen);
            BUG_ON(s->len < origlen);
            free(original);
        }

        pat = string_borrow_cstring(s);
        search_set_regexp(&e->search, pat);
        if (add_to_history) {
            history_append(&e->search_history, pat);
        }
    }

    if (e->macro.recording) {
        macro_search_hook(&e->macro, pat, e->search.reverse, add_to_history);
    }

    // Unselect, unless selection mode is active
    view_set_selection_type(e->view, e->view->select_mode);

    e->err.command_name = NULL;
    bool found = search_next(e->view, &e->search, e->options.case_sensitive_search);
    cmdline_clear(c);
    pop_input_mode(e);
    return found;
}

// Note that some of the `Command::flags` entries here aren't actually
// used in the `cmd` handler and are only included to mirror commands
// of the same name in normal mode. This is done as a convenience for
// allowing key binding commands like e.g. `bind -cns C-M-c 'copy -bk'`
// to be used, instead of needing 2 different commands (with and without
// the `-k` flag for normal vs. command/search modes).

#define CMD(name, flags, min, max, func) \
    {name, flags, 0, min, max, func}

static const Command common_cmds[] = {
    CMD("bol", "st", 0, 0, cmd_bol), // Ignored flags: s, t
    CMD("cancel", "", 0, 0, cmd_cancel),
    CMD("clear", "Ii", 0, 0, cmd_clear), // Ignored flags: I, i
    CMD("copy", "bikp", 0, 0, cmd_copy), // Ignored flag: k
    CMD("delete", "", 0, 0, cmd_delete),
    CMD("delete-eol", "n", 0, 0, cmd_delete_eol), // Ignored flag: n
    CMD("delete-word", "s", 0, 0, cmd_delete_word), // Ignored flag: s
    CMD("eol", "", 0, 0, cmd_eol),
    CMD("erase", "", 0, 0, cmd_erase),
    CMD("erase-bol", "", 0, 0, cmd_erase_bol),
    CMD("erase-word", "s", 0, 0, cmd_erase_word), // Ignored flag: s
    CMD("left", "", 0, 0, cmd_left),
    CMD("paste", "acmn", 0, 0, cmd_paste), // Ignored flags: a, c
    CMD("right", "", 0, 0, cmd_right),
    CMD("toggle", "gv", 1, -1, cmd_toggle), // Ignored flag: v
    CMD("word-bwd", "s", 0, 0, cmd_word_bwd), // Ignored flag: s
    CMD("word-fwd", "s", 0, 0, cmd_word_fwd), // Ignored flag: s
};

static const Command search_cmds[] = {
    CMD("accept", "eH", 0, 0, cmd_search_mode_accept),
    CMD("direction", "", 0, 0, cmd_direction),
    CMD("history-next", "S", 0, 0, cmd_search_history_next),
    CMD("history-prev", "S", 0, 0, cmd_search_history_prev),
};

static const Command command_cmds[] = {
    CMD("accept", "H", 0, 0, cmd_command_mode_accept),
    CMD("complete-next", "", 0, 0, cmd_complete_next),
    CMD("complete-prev", "", 0, 0, cmd_complete_prev),
    CMD("history-next", "S", 0, 0, cmd_command_history_next),
    CMD("history-prev", "S", 0, 0, cmd_command_history_prev),
};

static const Command *find_cmd_mode_command(const char *name)
{
    const Command *cmd = BSEARCH(name, common_cmds, command_cmp);
    return cmd ? cmd : BSEARCH(name, command_cmds, command_cmp);
}

static const Command *find_search_mode_command(const char *name)
{
    const Command *cmd = BSEARCH(name, common_cmds, command_cmp);
    return cmd ? cmd : BSEARCH(name, search_cmds, command_cmp);
}

const CommandSet cmd_mode_commands = {
    .lookup = find_cmd_mode_command
};

const CommandSet search_mode_commands = {
    .lookup = find_search_mode_command
};
