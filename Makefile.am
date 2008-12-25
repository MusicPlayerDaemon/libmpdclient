ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2

doc_DATA = README
EXTRA_DIST = libmpdclient.ld

mpdincludedir = $(includedir)/mpd
mpdinclude_HEADERS = \
	include/mpd/client.h \
	include/mpd/connection.h \
	include/mpd/directory.h \
	include/mpd/entity.h \
	include/mpd/error.h \
	include/mpd/idle.h \
	include/mpd/protocol.h \
	include/mpd/status.h \
	include/mpd/stats.h \
	include/mpd/output.h \
	include/mpd/return_element.h \
	include/mpd/song.h

AM_CPPFLAGS += -I$(srcdir)/include

lib_LTLIBRARIES = src/libmpdclient.la

src_libmpdclient_la_SOURCES = \
	src/internal.h \
	src/str_pool.c src/str_pool.h \
	src/ierror.c src/ierror.h \
	src/resolver.c src/resolver.h \
	src/connection.c \
	src/directory.c \
	src/output.c \
	src/return_element.c \
	src/entity.c \
	src/idle.c \
	src/socket.c src/socket.h \
	src/song.c \
	src/status.c \
	src/stats.c \
	src/libmpdclient.c

src_libmpdclient_la_LDFLAGS = -version-info @LIBMPDCLIENT_LIBTOOL_VERSION@ \
	-Wl,--version-script=$(srcdir)/libmpdclient.ld

noinst_PROGRAMS = src/example

src_example_SOURCES = src/example.c
src_example_LDADD = src/libmpdclient.la


#
# sparse
#

SPARSE = sparse
SPARSE_FLAGS =
SPARSE_CPPFLAGS = $(DEFAULT_INCLUDES) \
	-I$(shell $(CC) -print-file-name=include) \
	-I$(shell $(CC) -print-file-name=include-fixed)
sparse-check:
	for i in $(src_libmpdclient_la_SOURCES); \
	do \
		$(SPARSE) -I. $(CFLAGS) $(AM_CPPFLAGS) $(SPARSE_FLAGS) $(SPARSE_CPPFLAGS) $(srcdir)/$$i || exit; \
	done
