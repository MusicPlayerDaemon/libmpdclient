ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2

doc_DATA = README

lib_LTLIBRARIES = src/libmpdclient.la

src_libmpdclient_la_SOURCES = \
	src/str_pool.c src/str_pool.h \
	src/resolver.c src/resolver.h \
	src/song.c src/song.h \
	src/libmpdclient.c src/libmpdclient.h

src_libmpdclient_la_LDFLAGS = -version-info @LIBMPDCLIENT_LIBTOOL_VERSION@ \
	-Wl,--version-script=libmpdclient.ld

noinst_PROGRAMS = src/example

src_example_SOURCES = src/example.c
src_example_LDADD = src/libmpdclient.la
