#!/bin/sh
set -eu

# See:
# • Commit 264990a8c98b605d56a4d119cec82dd9d972a0ff
# • https://www.man7.org/linux/man-pages/man1/tr.1.html#BUGS
# • https://pubs.opengroup.org/onlinepubs/9799919799/utilities/tr.html#tag_20_126_08
NAME="$(echo "$1" | LC_ALL=C tr '[:lower:]' '[:upper:]')"

test "$2"
shift 1

if "$@"; then
    echo "#define HAVE_$NAME 1"
else
    echo "#define HAVE_$NAME 0"
fi
