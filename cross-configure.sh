#!/bin/sh

CONFIG_SHELL=/bin/sh
export CONFIG_SHELL
PREFIX=/usr/local/mingw
TARGET=i586-pc-mingw32
PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"
export PATH
if [ -f "$PREFIX/$TARGET/bin/$TARGET-sdl-config" ]; then
    SDL_CONFIG="$PREFIX/$TARGET/bin/$TARGET-sdl-config"
    export SDL_CONFIG
fi

CPPFLAGS=-I$PREFIX/$TARGET/include LDFLAGS=-L$PREFIX/$TARGET/lib sh configure \
	--target=$TARGET --host=$TARGET --build=x86_64-linux --prefix="$PREFIX/$TARGET" \
	$*
status=$?

exit $status
