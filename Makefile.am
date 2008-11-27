ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2

doc_DATA = README

mpdincludedir = $(includedir)/mpd
mpdinclude_HEADERS = \
	include/mpd/client.h \
	include/mpd/connection.h \
	include/mpd/idle.h \
	include/mpd/output.h \
	include/mpd/return_element.h \
	include/mpd/song.h

AM_CFLAGS += -Iinclude

lib_LTLIBRARIES = src/libmpdclient.la

src_libmpdclient_la_SOURCES = \
	src/str_pool.c src/str_pool.h \
	src/resolver.c src/resolver.h \
	src/connection.c \
	src/output.c \
	src/return_element.c \
	src/idle.c \
	src/song.c \
	src/libmpdclient.c

src_libmpdclient_la_LDFLAGS = -version-info @LIBMPDCLIENT_LIBTOOL_VERSION@ \
	-Wl,--version-script=libmpdclient.ld

noinst_PROGRAMS = src/example

src_example_SOURCES = src/example.c
src_example_LDADD = src/libmpdclient.la
