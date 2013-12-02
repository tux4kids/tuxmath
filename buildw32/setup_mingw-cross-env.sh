#!/bin/sh

# Simple script to setup mingw-cross-env and build the libraries needed to cross-build tuxmath for Windows.
# WARNING - this could be improved - in particular it will clobber any existing data at $MINGW_DIR!

#FIXME improve this by prompting user for location where mingw-cross-env is to be installed.

MINGW_DIR=/opt/mingw-cross-env

# Download and unpack mingw-cross-env
echo Downloading mingw-cross-env:
wget http://download.savannah.nongnu.org/releases/mingw-cross-env/mingw-cross-env-2.16.tar.gz
echo Unpacking tar archive:
tar xzf mingw-cross-env-2.16.tar.gz
echo Removing old installation from $MINGW_DIR, if present:
rm -rf $MINGW_DIR
echo Moving unpacked mingw-cross-env installation to $MINGW_DIR:
mv mingw-cross-env-2.16 $MINGW_DIR
echo Building libs needed by tux4kids:
cp t4k_common.mk $MINGW_DIR/src
cd $MINGW_DIR
make t4k_common
