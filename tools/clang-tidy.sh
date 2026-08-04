#!/bin/sh
set -eu

test "$#" -gt 0 || {
    echo "Usage: $0 FILE..."
    exit 64
}

# See also: -Wno-unterminated-string-initialization comment in mk/compiler.sh
CFLAGS='
    -std=gnu11
    -O2
    -Wall
    -Wextra
    -Wundef
    -Wcomma
    -Wno-unterminated-string-initialization
    -DDEBUG=3
    -D_FILE_OFFSET_BITS=64
    -Isrc
    -Itools/mock-headers
'

# shellcheck disable=SC2086
${CLANGTIDY:-clang-tidy} "$@" -- $CFLAGS 2>&1 |
    sed -E '/^[0-9]+ warnings? .*generated\.$/d' >&2

# TODO: Replace the above command with:
#
#   exec ${CLANGTIDY:-clang-tidy} "$@" -- $CFLAGS >&2
#
# ...when clang-tidy 22 is widespread enough for the sed filter
# to not be needed.
#
# See also:
# • https://github.com/llvm/llvm-project/pull/154012
# • https://releases.llvm.org/22.1.0/tools/clang/tools/extra/docs/ReleaseNotes.html#:~:text=suppressing%20diagnostic%20count%20messages
