#!/bin/sh

# Run this script to bootstrap the build process.  The script will generate
# the initial Makefiles and other files that are required to configure and
# build the gnx library.

PROJECT=gnx

test -n "$srcdir" || srcdir=$(dirname "$0")
test -n "$srcdir" || srcdir=.

cd "$srcdir"
test -f src/gnx.h || {
    echo "Run this script in the top-level $PROJECT directory."
    exit 1
}

autoreconf --force --install --verbose
automake --add-missing --copy > /dev/null 2>&1
