#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "shift.h"
#include "block-iter.h"
#include "buffer.h"
#include "change.h"
#include "indent.h"
#include "move.h"
#include "selection.h"
#include "util/xmalloc.h"
#include "view.h"

static char *alloc_indent(const Buffer *buffer, size_t count, size_t *sizep)
{
    size_t size;
    int ch;
    if (use_spaces_for_indent(buffer)) {
        ch = ' ';
        size = count * buffer->options.indent_width;
    } else {
        ch = '\t';
        size = count;
    }
    char *indent = xmalloc(size);
    memset(indent, ch, size);
    *sizep = size;
    return indent;
}

static void shift_right(View *view, size_t nr_lines, size_t count)
{
    size_t indent_size;
    char *indent = alloc_indent(view->buffer, count, &indent_size);
    size_t i = 0;
    while (1) {
        IndentInfo info;
        StringView line;
        fetch_this_line(&view->cursor, &line);
        get_indent_info(view->buffer, &line, &info);
        if (info.wsonly) {
            if (info.bytes) {
                // Remove indentation
                buffer_delete_bytes(view, info.bytes);
            }
        } else if (info.sane) {
            // Insert whitespace
            buffer_insert_bytes(view, indent, indent_size);
        } else {
            // Replace whole indentation with sane one
            size_t size;
            char *buf = alloc_indent(view->buffer, info.level + count, &size);
            buffer_replace_bytes(view, info.bytes, buf, size);
            free(buf);
        }
        if (++i == nr_lines) {
            break;
        }
        block_iter_eat_line(&view->cursor);
    }
    free(indent);
}

static void shift_left(View *view, size_t nr_lines, size_t count)
{
    const size_t indent_width = view->buffer->options.indent_width;
    const bool space_indent = use_spaces_for_indent(view->buffer);
    for (size_t i = 0; true; ) {
        IndentInfo info;
        StringView line;
        fetch_this_line(&view->cursor, &line);
        get_indent_info(view->buffer, &line, &info);
        if (info.wsonly) {
            if (info.bytes) {
                // Remove indentation
                buffer_delete_bytes(view, info.bytes);
            }
        } else if (info.level && info.sane) {
            size_t n = count;
            if (n > info.level) {
                n = info.level;
            }
            if (space_indent) {
                n *= indent_width;
            }
            buffer_delete_bytes(view, n);
        } else if (info.bytes) {
            // Replace whole indentation with sane one
            if (info.level > count) {
                size_t size;
                char *buf = alloc_indent(view->buffer, info.level - count, &size);
                buffer_replace_bytes(view, info.bytes, buf, size);
                free(buf);
            } else {
                buffer_delete_bytes(view, info.bytes);
            }
        }
        if (++i == nr_lines) {
            break;
        }
        block_iter_eat_line(&view->cursor);
    }
}

static void do_shift_lines(View *view, int count, size_t nr_lines)
{
    begin_change_chain();
    block_iter_bol(&view->cursor);
    if (count > 0) {
        shift_right(view, nr_lines, count);
    } else {
        shift_left(view, nr_lines, -count);
    }
    end_change_chain(view);
}

void shift_lines(View *view, int count)
{
    unsigned int width = view->buffer->options.indent_width;
    BUG_ON(width > INDENT_WIDTH_MAX);
    long x = view_get_preferred_x(view) + (count * width);
    if (x < 0) {
        x = 0;
    }

    if (view->selection == SELECT_NONE) {
        do_shift_lines(view, count, 1);
        goto out;
    }

    SelectionInfo info;
    view->selection = SELECT_LINES;
    init_selection(view, &info);
    view->cursor = info.si;
    size_t nr_lines = get_nr_selected_lines(&info);
    do_shift_lines(view, count, nr_lines);
    if (info.swapped) {
        // Cursor should be at beginning of selection
        block_iter_bol(&view->cursor);
        view->sel_so = block_iter_get_offset(&view->cursor);
        while (--nr_lines) {
            block_iter_prev_line(&view->cursor);
        }
    } else {
        BlockIter save = view->cursor;
        while (--nr_lines) {
            block_iter_prev_line(&view->cursor);
        }
        view->sel_so = block_iter_get_offset(&view->cursor);
        view->cursor = save;
    }

out:
    move_to_preferred_x(view, x);
}
