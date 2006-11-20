#!/bin/sh

DIR="`dirname \"$0\"`/.."

BIN="$DIR/bin"
LIB="$DIR/lib"

DYLD_LIBRARY_PATH="$DYLD_LIBRARY_PATH:$LIB"
export DYLD_LIBRARY_PATH

exec "$BIN/tuxmath"
