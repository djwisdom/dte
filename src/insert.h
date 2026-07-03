#ifndef INSERT_H
#define INSERT_H

#include <stdbool.h>
#include <stddef.h>
#include "util/macros.h"
#include "util/string-view.h"
#include "util/unicode.h"
#include "view.h"

typedef enum {
    NLI_NO_INDENT, // Never indent
    NLI_AUTO_INDENT, // Insert auto-indent, as if pressing Enter at EOL
    NLI_COPY_INDENT, // Copy indent from current line
} NewlineIndentType;

void insert_text(View *view, StringView text, bool move_after) NONNULL_ARGS;
void insert_ch(View *view, CodePoint ch) NONNULL_ARGS;
void new_line(View *view, bool above_cursor, NewlineIndentType indent_type) NONNULL_ARGS;

#endif
