#!/bin/sh -e

rm -rf config.cache build
mkdir build
libtoolize --force $LIBTOOLIZE_FLAGS
aclocal -I m4
autoconf
autoheader
automake --add-missing $AUTOMAKE_FLAGS

if test x$NOCONFIGURE = x; then
	echo "./configure $*"
	./configure $*
fi
