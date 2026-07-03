#ifndef JOIN_H
#define JOIN_H

#include <stddef.h>
#include "util/macros.h"
#include "util/string-view.h"
#include "view.h"

void join_lines(View *view, StringView delim) NONNULL_ARGS;

#endif
