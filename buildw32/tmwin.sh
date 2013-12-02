#!/bin/sh

# An 'all-in-one' script to build the Windows installer executable.

echo "Cleaning build directory:"
./cross-make.sh clean>/dev/null
./cross-make.sh distclean>/dev/null
echo "Running autoreconf --install .."
autoreconf --install ..
echo "Running ./cross-configure.sh --host=i686-pc-mingw32"
./cross-configure.sh --host=i686-pc-mingw32
echo "Running ./cross-make.sh"
./cross-make.sh
echo "Running ./cross-make.sh dist (to generate gmo files)"
./cross-make.sh dist
echo "Building NSIS installer file"
./cross-make.sh nsis

