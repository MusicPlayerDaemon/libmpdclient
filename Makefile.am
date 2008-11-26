ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2

doc_DATA = README

lib_LTLIBRARIES = libmpdclient.la

libmpdclient_la_SOURCES = \
	libmpdclient.c libmpdclient.h

libmpdclient_la_LDFLAGS = -version-info @LIBMPDCLIENT_LIBTOOL_VERSION@ \
	-Wl,--version-script=libmpdclient.ld

noinst_PROGRAMS = example

example_SOURCES = example.c
example_LDADD = libmpdclient.la
