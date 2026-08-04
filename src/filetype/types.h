#ifndef FILETYPE_TYPES_H
#define FILETYPE_TYPES_H

#include <stdbool.h>
#include <string.h>
#include "util/macros.h"
#include "util/string-view.h"

// NOLINTNEXTLINE(cert-int09-c,readability-enum-initial-value)
typedef enum {
    ADA,
    ASCIIDOC,
    ASM,
    AWK,
    BATCH,
    BIBTEX,
    C, CPLUSPLUS = C,
    CLOJURE,
    CMAKE,
    COBOL,
    COCCINELLE,
    COFFEESCRIPT,
    CONFIG,
    CONFIG_NTC,
    CSH,
    CSHARP,
    CSS,
    CSV,
    CTAGS,
    D,
    D2,
    DART,
    DEVICETREE,
    DIFF,
    DOCKER,
    DOT,
    DTE,
    ELIXIR,
    ERLANG,
    FISH,
    GCODE,
    GDSCRIPT,
    GETTEXT,
    GITCOMMIT,
    GITIGNORE,
    GITLOG,
    GITNOTE,
    GITREBASE,
    GITSTASH,
    GLSL,
    GNUPLOT,
    GO,
    GOMODULE,
    GPERF,
    GRADLE,
    GRAPHQL,
    GROOVY,
    HARE,
    HASKELL,
    HCL, // https://github.com/hashicorp/hcl
    HTML,
    INDENT,
    INI,
    JAVA,
    JAVASCRIPT,
    JQ,
    JSON,
    JSONC, // JSON with comments
    JULIA,
    KDL,
    KOTLIN,
    LEX,
    LISP,
    LRC, // Lyrics
    LUA,
    M4,
    MAIL,
    MAKE,
    MARKDOWN,
    MESON,
    MOONSCRIPT,
    NFTABLES,
    NGINX,
    NIM,
    NINJA,
    NIX,
    NONE,
    OBJC,
    OCAML,
    ODIN,
    PASCAL,
    PERL,
    PHP,
    PKGCONFIG,
    POSTSCRIPT,
    POWERSHELL,
    PROTOBUF,
    PYTHON,
    RACKET,
    ROBOTSTXT,
    ROFF,
    RPMSPEC,
    RST,
    RUBY,
    RUST,
    SCAD,
    SCALA,
    SCHEME,
    SCSS,
    SED,
    SH,
    SLINT,
    SQL,
    STEP,
    SWIFT,
    TCL,
    TERMINFO,
    TEX,
    TEXINFO,
    TEXMFCNF,
    TLVERILOG,
    TMUX,
    TOML,
    TSV,
    TYPESCRIPT,
    TYPST,
    VALA,
    VCARD,
    VERILOG,
    VHDL,
    VIML,
    WEECHATLOG,
    XML,
    XRESOURCES,
    YACC,
    YAML,
    ZIG,
    NR_BUILTIN_FILETYPES
} FileTypeEnum;

extern const char builtin_filetype_names[NR_BUILTIN_FILETYPES][12];

static inline int ft_compare(const void *key, const void *elem)
{
    const StringView *sv = key;
    const char *ext = elem; // Cast to first member of struct
    int res = memcmp(sv->data, ext, sv->length);
    if (unlikely(res == 0 && ext[sv->length] != '\0')) {
        res = -1;
    }
    return res;
}

FileTypeEnum filetype_from_basename(StringView name) WARN_UNUSED_RESULT;
FileTypeEnum filetype_from_extension(StringView ext) WARN_UNUSED_RESULT;
FileTypeEnum filetype_from_interpreter(StringView name) WARN_UNUSED_RESULT;
FileTypeEnum filetype_from_path(StringView path) WARN_UNUSED_RESULT;
FileTypeEnum filetype_from_signature(StringView line) WARN_UNUSED_RESULT;
FileTypeEnum filetype_from_signature_late(StringView line) WARN_UNUSED_RESULT;
bool is_ignored_extension(StringView sv) WARN_UNUSED_RESULT;

#endif
