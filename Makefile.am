ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.10 dist-bzip2 subdir-objects

mpdincludedir = $(includedir)/mpd
mpdinclude_HEADERS = \
	include/mpd/async.h \
	include/mpd/audio_format.h \
	include/mpd/client.h \
	include/mpd/capabilities.h \
	include/mpd/compiler.h \
	include/mpd/connection.h \
	include/mpd/database.h \
	include/mpd/directory.h \
	include/mpd/entity.h \
	include/mpd/error.h \
	include/mpd/idle.h \
	include/mpd/list.h \
	include/mpd/mixer.h \
	include/mpd/parser.h \
	include/mpd/password.h \
	include/mpd/player.h \
	include/mpd/playlist.h \
	include/mpd/protocol.h \
	include/mpd/queue.h \
	include/mpd/recv.h \
	include/mpd/response.h \
	include/mpd/send.h \
	include/mpd/status.h \
	include/mpd/stats.h \
	include/mpd/tag.h \
	include/mpd/output.h \
	include/mpd/pair.h \
	include/mpd/search.h \
	include/mpd/song.h

AM_CPPFLAGS += -I$(srcdir)/include

lib_LTLIBRARIES = src/libmpdclient.la

src_libmpdclient_la_SOURCES = \
	src/async.c src/iasync.h \
	src/buffer.h \
	src/internal.h \
	src/ierror.c src/ierror.h \
	src/resolver.c src/resolver.h \
	src/capabilities.c \
	src/connection.c \
	src/database.c \
	src/directory.c \
	src/rdirectory.c \
	src/error.c \
	src/output.c \
	src/coutput.c \
	src/entity.c \
	src/idle.c \
	src/iso8601.h \
	src/iso8601.c \
	src/list.c \
	src/mixer.c \
	src/parser.c \
	src/password.c \
	src/player.c \
	src/playlist.c \
	src/rplaylist.c \
	src/cplaylist.c \
	src/queue.c \
	src/quote.c src/quote.h \
	src/recv.c \
	src/response.c \
	src/run.c src/run.h \
	src/search.c \
	src/send.c src/isend.h \
	src/socket.c src/socket.h \
	src/song.c \
	src/status.c \
	src/cstatus.c \
	src/stats.c \
	src/cstats.c \
	src/sync.c src/sync.h \
	src/tag.c

src_libmpdclient_la_LDFLAGS = -version-info @LIBMPDCLIENT_LIBTOOL_VERSION@ \
	-no-undefined

if HAVE_GNU_LD
src_libmpdclient_la_LDFLAGS += -Wl,--version-script=$(srcdir)/libmpdclient.ld
endif

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
	-D__CHAR_BIT__=8
	-I$(shell $(CC) -print-file-name=include) \
	-I$(shell $(CC) -print-file-name=include-fixed)
sparse-check:
	$(SPARSE) -I. $(CFLAGS) $(AM_CPPFLAGS) $(SPARSE_FLAGS) $(SPARSE_CPPFLAGS) $(addprefix $(srcdir)/,$(src_libmpdclient_la_SOURCES))

#
# Documentation
#

doc_DATA = README COPYING NEWS

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

uninstall-local:
	rm -f $(DESTDIR)$(docdir)/html/index.html \
	$(DESTDIR)$(docdir)/html/*.css \
	$(DESTDIR)$(docdir)/html/*.gif \
	$(DESTDIR)$(docdir)/html/*.png

upload: doc/api/html/index.html
	rsync -vpruz --delete doc/api/html/ \
		cirrus@www.musicpd.org:/var/www/musicpd.org/www/doc/libmpdclient/ \
		--chmod=Dug+rwx,Do+rx,Fug+rw,Fo+r

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
	autogen.sh \
	$(doc_DATA)
