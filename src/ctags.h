#ifndef CTAGS_H
#define CTAGS_H

#include <stdbool.h>
#include <stddef.h>
#include "util/macros.h"
#include "util/string-view.h"

typedef struct {
    StringView name; // Name of tag (points into TagFile::buf)
    StringView filename; // File containing tag (points into TagFile::buf)
    char *pattern; // Regex pattern used to locate tag (escaped ex command)
    unsigned long lineno; // Line number in file (mutually exclusive with pattern)
    char kind; // ASCII letter representing type of tag (e.g. f=function)
    bool local; // Indicates if tag is local to file (e.g. "static" in C)
} Tag;

NONNULL_ARGS WARN_UNUSED_RESULT READWRITE(2) WRITEONLY(5)
bool next_tag (
    StringView text,
    size_t *posp,
    StringView prefix,
    bool exact,
    Tag *tag
);

bool parse_ctags_line(Tag *t, StringView line) NONNULL_ARGS;
void free_tag(Tag *t) NONNULL_ARGS;

#endif
