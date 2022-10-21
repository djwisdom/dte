#include <errno.h>
#include <unistd.h>
#include "window.h"
#include "error.h"
#include "file-history.h"
#include "load-save.h"
#include "lock.h"
#include "move.h"
#include "util/path.h"
#include "util/str-util.h"
#include "util/strtonum.h"
#include "util/xmalloc.h"

Window *new_window(void)
{
    return xnew0(Window, 1);
}

View *window_add_buffer(Window *w, Buffer *b)
{
    View *v = xnew0(View, 1);
    v->buffer = b;
    v->window = w;
    v->cursor.head = &b->blocks;
    v->cursor.blk = BLOCK(b->blocks.next);

    ptr_array_append(&b->views, v);
    ptr_array_append(&w->views, v);
    w->update_tabbar = true;
    return v;
}

View *window_open_empty_buffer(EditorState *e, Window *w)
{
    return window_add_buffer(w, open_empty_buffer(e));
}

View *window_open_buffer (
    EditorState *e,
    Window *w,
    const char *filename,
    bool must_exist,
    const Encoding *encoding
) {
    if (unlikely(filename[0] == '\0')) {
        error_msg("Empty filename not allowed");
        return NULL;
    }

    bool dir_missing = false;
    char *absolute = path_absolute(filename);
    if (absolute) {
        // Already open?
        Buffer *b = find_buffer(&e->buffers, absolute);
        if (b) {
            if (!streq(absolute, b->abs_filename)) {
                const char *bufname = buffer_filename(b);
                char *s = short_filename(absolute, &e->home_dir);
                info_msg("%s and %s are the same file", s, bufname);
                free(s);
            }
            free(absolute);
            return window_get_view(w, b);
        }
    } else {
        // Let load_buffer() create error message
        dir_missing = (errno == ENOENT);
    }

    /*
    /proc/$PID/fd/ contains symbolic links to files that have been opened
    by process $PID. Some of the files may have been deleted but can still
    be opened using the symbolic link but not by using the absolute path.

    # create file
    mkdir /tmp/x
    echo foo > /tmp/x/file

    # in another shell: keep the file open
    tail -f /tmp/x/file

    # make the absolute path unavailable
    rm /tmp/x/file
    rmdir /tmp/x

    # this should still succeed
    dte /proc/$(pidof tail)/fd/3
    */

    Buffer *b = buffer_new(e, encoding);
    if (!load_buffer(b, filename, e->options.filesize_limit, must_exist)) {
        remove_and_free_buffer(&e->buffers, b);
        free(absolute);
        return NULL;
    }
    if (unlikely(b->file.mode == 0 && dir_missing)) {
        // New file in non-existing directory. This is usually a mistake.
        error_msg("Error opening %s: Directory does not exist", filename);
        remove_and_free_buffer(&e->buffers, b);
        free(absolute);
        return NULL;
    }

    if (absolute) {
        b->abs_filename = absolute;
    } else {
        // FIXME: obviously wrong
        b->abs_filename = xstrdup(filename);
    }
    update_short_filename(b, &e->home_dir);

    if (e->options.lock_files) {
        if (!lock_file(b->abs_filename)) {
            b->readonly = true;
        } else {
            b->locked = true;
        }
    }

    if (b->file.mode != 0 && !b->readonly && access(filename, W_OK)) {
        error_msg("No write permission to %s, marking read-only", filename);
        b->readonly = true;
    }

    return window_add_buffer(w, b);
}

View *window_get_view(Window *w, Buffer *b)
{
    View *v = window_find_view(w, b);
    if (!v) {
        // Open the buffer in other window to this window
        v = window_add_buffer(w, b);
        v->cursor = ((View*)b->views.ptrs[0])->cursor;
    }
    return v;
}

View *window_find_view(Window *w, Buffer *b)
{
    for (size_t i = 0, n = b->views.count; i < n; i++) {
        View *v = b->views.ptrs[i];
        if (v->window == w) {
            return v;
        }
    }
    // Buffer isn't open in this window
    return NULL;
}

View *window_find_unclosable_view(Window *w)
{
    // Check active view first
    if (w->view && !view_can_close(w->view)) {
        return w->view;
    }
    for (size_t i = 0, n = w->views.count; i < n; i++) {
        View *v = w->views.ptrs[i];
        if (!view_can_close(v)) {
            return v;
        }
    }
    return NULL;
}

static void window_remove_views(EditorState *e, Window *w)
{
    while (w->views.count > 0) {
        View *v = w->views.ptrs[w->views.count - 1];
        remove_view(e, v);
    }
}

// NOTE: w->frame isn't removed
void window_free(EditorState *e, Window *w)
{
    window_remove_views(e, w);
    free(w->views.ptrs);
    w->frame = NULL;
    free(w);
}

// Remove view from v->window and v->buffer->views and free it.
size_t remove_view(EditorState *e, View *v)
{
    Window *w = v->window;
    if (v == w->prev_view) {
        w->prev_view = NULL;
    }
    if (v == e->view) {
        e->view = NULL;
        e->buffer = NULL;
    }

    size_t idx = ptr_array_idx(&w->views, v);
    BUG_ON(idx >= w->views.count);
    ptr_array_remove_idx(&w->views, idx);
    w->update_tabbar = true;

    Buffer *b = v->buffer;
    ptr_array_remove(&b->views, v);
    if (b->views.count == 0) {
        if (b->options.file_history && b->abs_filename) {
            FileHistory *hist = &e->file_history;
            file_history_add(hist, v->cy + 1, v->cx_char + 1, b->abs_filename);
        }
        remove_and_free_buffer(&e->buffers, b);
    }

    free(v);
    return idx;
}

void window_close_current_view(EditorState *e, Window *w)
{
    size_t idx = remove_view(e, w->view);
    if (w->prev_view) {
        w->view = w->prev_view;
        w->prev_view = NULL;
        return;
    }
    if (w->views.count == 0) {
        window_open_empty_buffer(e, w);
    }
    if (w->views.count == idx) {
        idx--;
    }
    w->view = w->views.ptrs[idx];
}

static void restore_cursor_from_history(const FileHistory *hist, View *v)
{
    unsigned long row, col;
    if (file_history_find(hist, v->buffer->abs_filename, &row, &col)) {
        move_to_line(v, row);
        move_to_column(v, col);
    }
}

void set_view(EditorState *e, View *v)
{
    if (e->view == v) {
        return;
    }

    // Forget previous view when changing view using any other command but open
    if (e->window) {
        e->window->prev_view = NULL;
    }

    e->view = v;
    e->buffer = v->buffer;
    e->window = v->window;
    e->window->view = v;

    if (!v->buffer->setup) {
        buffer_setup(e, v->buffer);
        if (v->buffer->options.file_history && v->buffer->abs_filename) {
            restore_cursor_from_history(&e->file_history, v);
        }
    }

    // view.cursor can be invalid if same buffer was modified from another view
    if (v->restore_cursor) {
        v->cursor.blk = BLOCK(v->buffer->blocks.next);
        block_iter_goto_offset(&v->cursor, v->saved_cursor_offset);
        v->restore_cursor = false;
        v->saved_cursor_offset = 0;
    }

    // Save cursor states of views sharing same buffer
    for (size_t i = 0, n = v->buffer->views.count; i < n; i++) {
        View *other = v->buffer->views.ptrs[i];
        if (other != v) {
            other->saved_cursor_offset = block_iter_get_offset(&other->cursor);
            other->restore_cursor = true;
        }
    }
}

View *window_open_new_file(EditorState *e, Window *w)
{
    View *prev = w->view;
    View *v = window_open_empty_buffer(e, w);
    set_view(e, v);
    w->prev_view = prev;
    return v;
}

// If window contains only one untouched buffer it'll be closed after
// opening another file. This is done because closing the last buffer
// causes an empty buffer to be opened (windows must contain at least
// one buffer).
static bool is_useless_empty_view(const View *v)
{
    if (v == NULL || v->window->views.count != 1) {
        return false;
    }
    if (v->buffer->abs_filename || v->buffer->change_head.nr_prev != 0) {
        return false;
    }
    if (v->buffer->display_filename) {
        return false;
    }
    return true;
}

View *window_open_file(EditorState *e, Window *w, const char *filename, const Encoding *encoding)
{
    View *prev = w->view;
    bool useless = is_useless_empty_view(prev);
    View *v = window_open_buffer(e, w, filename, false, encoding);
    if (v) {
        set_view(e, v);
        if (useless) {
            remove_view(e, prev);
        } else {
            w->prev_view = prev;
        }
    }
    return v;
}

void window_open_files(EditorState *e, Window *w, char **filenames, const Encoding *encoding)
{
    View *empty = w->view;
    bool useless = is_useless_empty_view(empty);
    bool first = true;
    for (size_t i = 0; filenames[i]; i++) {
        View *v = window_open_buffer(e, w, filenames[i], false, encoding);
        if (v && first) {
            set_view(e, v);
            first = false;
        }
    }
    if (useless && w->view != empty) {
        remove_view(e, empty);
    }
}

void mark_buffer_tabbars_changed(Buffer *b)
{
    for (size_t i = 0, n = b->views.count; i < n; i++) {
        View *v = b->views.ptrs[i];
        v->window->update_tabbar = true;
    }
}

static int line_numbers_width(const EditorState *e, const Window *win)
{
    if (!e->options.show_line_numbers || !win->view) {
        return 0;
    }
    int width = size_str_width(win->view->buffer->nl) + 1;
    int min = LINE_NUMBERS_MIN_WIDTH;
    return (width < min) ? min : width;
}

static int edit_x_offset(const EditorState *e, const Window *win)
{
    return line_numbers_width(e, win);
}

static int edit_y_offset(const EditorState *e)
{
    return e->options.tab_bar ? 1 : 0;
}

static void set_edit_size(const EditorState *e, Window *win)
{
    int xo = edit_x_offset(e, win);
    int yo = edit_y_offset(e);

    win->edit_w = win->w - xo;
    win->edit_h = win->h - yo - 1; // statusline
    win->edit_x = win->x + xo;
}

void calculate_line_numbers(const EditorState *e, Window *win)
{
    int w = line_numbers_width(e, win);
    if (w != win->line_numbers.width) {
        win->line_numbers.width = w;
        win->line_numbers.first = 0;
        win->line_numbers.last = 0;
        mark_all_lines_changed(win->view->buffer);
    }
    set_edit_size(e, win);
}

void set_window_coordinates(const EditorState *e, Window *win, int x, int y)
{
    win->x = x;
    win->y = y;
    win->edit_x = x + edit_x_offset(e, win);
    win->edit_y = y + edit_y_offset(e);
}

void set_window_size(const EditorState *e, Window *win, int w, int h)
{
    win->w = w;
    win->h = h;
    calculate_line_numbers(e, win);
}

int window_get_scroll_margin(const Window *w, unsigned int scroll_margin)
{
    int max = (w->edit_h - 1) / 2;
    if (scroll_margin > max) {
        return max;
    }
    return scroll_margin;
}

void frame_for_each_window(const Frame *f, void (*func)(Window*, void*), void *data)
{
    if (f->window) {
        func(f->window, data);
        return;
    }
    for (size_t i = 0, n = f->frames.count; i < n; i++) {
        frame_for_each_window(f->frames.ptrs[i], func, data);
    }
}

typedef struct {
    const Window *const target; // Window to search for (set at init.)
    Window *first; // Window passed in first callback invocation
    Window *last; // Window passed in last callback invocation
    Window *prev; // Window immediately before target (if any)
    Window *next; // Window immediately after target (if any)
    bool found; // Set to true when target is found
} WindowCallbackData;

static void find_prev_and_next(Window *w, void *ud)
{
    WindowCallbackData *data = ud;
    data->last = w;
    if (data->found) {
        if (!data->next) {
            data->next = w;
        }
        return;
    }
    if (!data->first) {
        data->first = w;
    }
    if (w == data->target) {
        data->found = true;
        return;
    }
    data->prev = w;
}

Window *prev_window(EditorState *e, Window *w)
{
    WindowCallbackData data = {.target = w};
    frame_for_each_window(e->root_frame, find_prev_and_next, &data);
    BUG_ON(!data.found);
    return data.prev ? data.prev : data.last;
}

Window *next_window(EditorState *e, Window *w)
{
    WindowCallbackData data = {.target = w};
    frame_for_each_window(e->root_frame, find_prev_and_next, &data);
    BUG_ON(!data.found);
    return data.next ? data.next : data.first;
}

void window_close_current(EditorState *e)
{
    Window *window = e->window;
    if (!window->frame->parent) {
        // Don't close last window
        window_remove_views(e, window);
        set_view(e, window_open_empty_buffer(e, window));
        return;
    }

    WindowCallbackData data = {.target = window};
    frame_for_each_window(e->root_frame, find_prev_and_next, &data);
    BUG_ON(!data.found);
    Window *next_or_prev = data.next ? data.next : data.prev;
    BUG_ON(!next_or_prev);

    remove_frame(window->frame);
    e->window = NULL;
    set_view(e, next_or_prev->view);

    mark_everything_changed(e);
    debug_frame(e->root_frame);
}
