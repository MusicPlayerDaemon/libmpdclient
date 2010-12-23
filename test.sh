#!/bin/sh -e
#
# This shell script tests the build of libmpdclient with various
# compile-time options.
#
# Author: Max Kellermann <max@duempel.org>

MAKE="make -j4"
PREFIX=/tmp/libmpdclient
rm -rf $PREFIX

export CFLAGS="-O3"

test -x configure || NOCONFIGURE=1 ./autogen.sh

# debug build
CFLAGS="-O0 -ggdb" ./configure --prefix=$PREFIX/debug --enable-debug --enable-werror
$MAKE clean
$MAKE install

# no TCP
CFLAGS="-O0 -ggdb" ./configure --prefix=$PREFIX/notcp --enable-debug --enable-werror --disable-tcp
$MAKE clean
$MAKE install

# release build
CFLAGS="-O3" ./configure --prefix=$PREFIX/release --disable-debug --enable-werror
$MAKE clean
$MAKE install

# dietlibc build
CC="diet -Os gcc -nostdinc" ./configure --prefix=$PREFIX/diet --disable-debug --disable-shared
$MAKE clean
$MAKE install

# dietlibc build, no TCP
CC="diet -Os gcc -nostdinc" ./configure --prefix=$PREFIX/diet-notcp --disable-debug --disable-shared --disable-tcp
$MAKE clean
$MAKE install

# WIN32
CFLAGS="-O3" ./configure --prefix=$PREFIX/win32 --host=i586-mingw32msvc --disable-debug --enable-werror
$MAKE clean
$MAKE install
