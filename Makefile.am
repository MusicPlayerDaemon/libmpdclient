ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2

mpdincludedir = $(includedir)/mpd
mpdinclude_HEADERS = \
	include/mpd/async.h \
	include/mpd/client.h \
	include/mpd/command.h \
	include/mpd/connection.h \
	include/mpd/cpos.h \
	include/mpd/directory.h \
	include/mpd/entity.h \
	include/mpd/error.h \
	include/mpd/idle.h \
	include/mpd/list.h \
	include/mpd/parser.h \
	include/mpd/protocol.h \
	include/mpd/recv.h \
	include/mpd/response.h \
	include/mpd/run.h \
	include/mpd/send.h \
	include/mpd/status.h \
	include/mpd/stats.h \
	include/mpd/tag.h \
	include/mpd/output.h \
	include/mpd/pair.h \
	include/mpd/search.h \
	include/mpd/song.h \
	include/mpd/stored_playlist.h

AM_CPPFLAGS += -I$(srcdir)/include

lib_LTLIBRARIES = src/libmpdclient.la

src_libmpdclient_la_SOURCES = \
	src/async.c \
	src/buffer.h \
	src/internal.h \
	src/str_pool.c src/str_pool.h \
	src/ierror.c src/ierror.h \
	src/resolver.c src/resolver.h \
	src/command.c \
	src/connection.c \
	src/cpos.c \
	src/directory.c \
	src/output.c \
	src/entity.c \
	src/idle.c \
	src/iso8601.h \
	src/iso8601.c \
	src/list.c \
	src/parser.c \
	src/quote.c src/quote.h \
	src/recv.c \
	src/response.c \
	src/run.c \
	src/search.c \
	src/send.c \
	src/socket.c src/socket.h \
	src/song.c \
	src/status.c \
	src/stats.c \
	src/cstats.c \
	src/stored_playlist.c \
	src/sync.c src/sync.h \
	src/tag.c

src_libmpdclient_la_LDFLAGS = -version-info @LIBMPDCLIENT_LIBTOOL_VERSION@ \
	-Wl,--version-script=$(srcdir)/libmpdclient.ld

noinst_PROGRAMS = src/example test/test

src_example_SOURCES = src/example.c
src_example_LDADD = src/libmpdclient.la

test_test_SOURCES = test/main.c
test_test_LDADD = src/libmpdclient.la


#
# sparse
#

SPARSE = sparse
SPARSE_FLAGS =
SPARSE_CPPFLAGS = $(DEFAULT_INCLUDES) \
	-I$(shell $(CC) -print-file-name=include) \
	-I$(shell $(CC) -print-file-name=include-fixed)
sparse-check:
	$(SPARSE) -I. $(CFLAGS) $(AM_CPPFLAGS) $(SPARSE_FLAGS) $(SPARSE_CPPFLAGS) $(src_libmpdclient_la_SOURCES)

#
# Documentation
#

doc_DATA = README COPYING

if DOXYGEN

doc/api/html/index.html: doc/doxygen.conf $(wildcard include/mpd/*.h)
	$(DOXYGEN) $<

all-local: doc/api/html/index.html

mostlyclean-local:
	rm -rf doc/api

install-data-local: doc/api/html/index.html
	$(mkinstalldirs) $(DESTDIR)$(docdir)/html
	$(INSTALL_DATA) -c -m 644 doc/api/html/*.html doc/api/html/*.css \
		doc/api/html/*.png doc/api/html/*.gif \
		$(DESTDIR)$(docdir)/html

endif

#
# Installation
#

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmpdclient.pc

#
# Distribution
#

EXTRA_DIST = \
	libmpdclient.ld \
	libmpdclient.pc.in \
	$(doc_DATA)
