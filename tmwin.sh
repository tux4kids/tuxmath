# An 'all-in-one' script to build the Windows installer executable.

make clean
make distclean
autoreconf --install
./cross-configure.sh --with-sdl-prefix --with-included-gettext
./cross-make.sh nsis

