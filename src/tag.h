#ifndef TAG_H
#define TAG_H

#include <stdbool.h>
#include <sys/types.h>
#include "ctags.h"
#include "msg.h"
#include "util/macros.h"
#include "util/ptr-array.h"
#include "util/string-view.h"
#include "util/string.h"

// Metadata and contents of a tags(5) file, as loaded by load_tag_file()
typedef struct {
    char *filename; // The absolute path of the tags(5) file
    size_t dirname_len; // The length of the directory part of `filename` (including the last slash)
    char *buf; // The contents of the tags file
    size_t size; // The length of `buf`
    time_t mtime; // The modification time of the tags file (when last loaded)
} TagFile;

bool load_tag_file(TagFile *tf, ErrorBuffer *ebuf) NONNULL_ARGS WARN_UNUSED_RESULT;
void add_message_for_tag(MessageList *messages, Tag *tag, StringView dir) NONNULL_ARGS;
size_t tag_lookup(TagFile *tf, MessageList *messages, ErrorBuffer *ebuf, const StringView *name, const char *filename) NONNULL_ARG(1, 2, 3, 4);
void collect_tags(TagFile *tf, PointerArray *a, const StringView *prefix) NONNULL_ARGS;
String dump_tags(TagFile *tf, ErrorBuffer *ebuf) NONNULL_ARGS;
void tag_file_free(TagFile *tf) NONNULL_ARGS;

#endif
