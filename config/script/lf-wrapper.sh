#!/bin/sh
set -eu

TMPFILE="$(mktemp)"
trap "rm -f '$TMPFILE'" INT QUIT TERM HUP EXIT

lf -selection-path "$TMPFILE" \
    -command 'map <esc> quit' \
    -command 'map <enter> open' \
    -command 'map <tab> :toggle; down' \
    "$@" >/dev/tty

cat "$TMPFILE"
