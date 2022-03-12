#ifndef FILE_OPTION_H
#define FILE_OPTION_H

#include "buffer.h"
#include "util/macros.h"
#include "util/ptr-array.h"
#include "util/string-view.h"
#include "util/string.h"

typedef enum {
    FILE_OPTIONS_FILENAME,
    FILE_OPTIONS_FILETYPE,
} FileOptionType;

void add_file_options(PointerArray *file_options, FileOptionType type, StringView str, char **strs, size_t nstrs) NONNULL_ARGS;
void set_file_options(const PointerArray *file_options, Buffer *b) NONNULL_ARGS;
void set_editorconfig_options(Buffer *b) NONNULL_ARGS;
void dump_file_options(const PointerArray *file_options, String *buf);
void free_file_options(PointerArray *file_options);

#endif
