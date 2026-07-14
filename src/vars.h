#ifndef VARS_H
#define VARS_H

#include "util/macros.h"
#include "util/ptr-array.h"
#include "util/string-view.h"
#include "util/string.h"

struct EditorState;

char *expand_normal_var(const struct EditorState *e, const char *name) NONNULL_ARGS WARN_UNUSED_RESULT;
void collect_normal_vars(PointerArray *a, StringView prefix, const char *suffix) NONNULL_ARGS;
String dump_normal_vars(struct EditorState *e) NONNULL_ARGS WARN_UNUSED_RESULT;

#endif
