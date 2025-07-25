#ifndef EDITORCONFIG_EDITORCONFIG_H
#define EDITORCONFIG_EDITORCONFIG_H

#include <stdbool.h>
#include "util/macros.h"

typedef enum {
    INDENT_STYLE_UNSPECIFIED,
    INDENT_STYLE_TAB,
    INDENT_STYLE_SPACE,
} EditorConfigIndentStyle;

typedef struct {
    unsigned int indent_size : 4;
    unsigned int tab_width : 4;
    unsigned int max_line_length : 10;
    unsigned int indent_style : 2; // EditorConfigIndentStyle
    unsigned int indent_size_is_tab : 1; // bool
} EditorConfigOptions;

EditorConfigOptions get_editorconfig_options(const char *pathname) NONNULL_ARGS WARN_UNUSED_RESULT;

#endif
