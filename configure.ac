AC_PREREQ(2.60)
AC_INIT(MPD client library for C, 2.0~git, max@duempel.org, libmpdclient)
AC_CONFIG_SRCDIR([src/connection.c])
AC_CONFIG_AUX_DIR(build)
AM_INIT_AUTOMAKE([foreign 1.9 dist-bzip2])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

LIBMPDCLIENT_LIBTOOL_VERSION=2:0:0
AC_SUBST(LIBMPDCLIENT_LIBTOOL_VERSION)

# Remove the check for c++ and fortran compiler
m4_defun([_LT_AC_LANG_CXX_CONFIG], [:])
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])

dnl Check for programs
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_LIBTOOL


dnl
dnl initialize variables
dnl

set -- $CFLAGS


dnl
dnl Check for libraries
dnl

dnl Try to find the socket functions
AC_CHECK_FUNC([socket],
	[],
	[AC_CHECK_LIB([socket], [socket], [LIBS="$LIBS -lsocket"],
		[AC_MSG_ERROR(No UNIX socket API found)])]
)

dnl Try to find the gethostbyname functions
AC_CHECK_FUNC([gethostbyname],
	[],
	[AC_CHECK_LIB([nsl],[gethostbyname],[LIBS="$LIBS -lnsl"],
		[AC_MSG_ERROR(No UNIX gethostbyname API found)])]
)


dnl
dnl build options
dnl

AC_ARG_ENABLE(werror,
	AS_HELP_STRING([--enable-werror],
		[Treat warnings as errors (default: disabled)]),
	ENABLE_WERROR=$enableval,
	ENABLE_WERROR=no)

if test "x$ENABLE_WERROR" = xyes; then
	AM_CFLAGS="$AM_CFLAGS -Werror -pedantic-errors"
fi

AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug],
		[Enable debugging (default: disabled)]),
	ENABLE_DEBUG=$enableval,
	ENABLE_DEBUG=no)

if test "x$ENABLE_DEBUG" = xno; then
	AM_CFLAGS="$AM_CFLAGS -DNDEBUG"
fi


dnl
dnl CFLAGS
dnl

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CPPFLAGS)

CHECK_CFLAG([-Wall])
CHECK_CFLAG([-Wextra])
CHECK_CFLAG([-Wno-deprecated-declarations])
CHECK_CFLAG([-Wmissing-prototypes])
CHECK_CFLAG([-Wdeclaration-after-statement])
CHECK_CFLAG([-Wshadow])
CHECK_CFLAG([-Wpointer-arith])
CHECK_CFLAG([-Wstrict-prototypes])
CHECK_CFLAG([-Wcast-qual])
CHECK_CFLAG([-Wwrite-strings])


dnl
dnl Compile-time options
dnl

dnl Default host
AC_MSG_CHECKING([for default MPD host])
AC_ARG_WITH([default-host],
	AC_HELP_STRING([--with-default-host=ARG],
		[Default MPD host @<:@localhost@:>@]),
	[DEFAULT_HOST="$withval"],
	[DEFAULT_HOST="localhost"])
AC_MSG_RESULT([$DEFAULT_HOST])
AC_DEFINE_UNQUOTED([DEFAULT_HOST], ["$DEFAULT_HOST"], [Default MPD host])

dnl Default port
AC_MSG_CHECKING([for default MPD port])
AC_ARG_WITH([default-port],
	AC_HELP_STRING([--with-default-port=ARG],
		[Default port @<:@6600@:>@]),
	[DEFAULT_PORT="$withval"],
	[DEFAULT_PORT="6600"])
AC_MSG_RESULT([$DEFAULT_PORT])
AC_DEFINE_UNQUOTED([DEFAULT_PORT], [$DEFAULT_PORT], [Default MPD port])
AC_DEFINE_UNQUOTED([DEFAULT_PORT_STR], ["$DEFAULT_PORT"], [Default MPD port])


dnl
dnl Done
dnl

AC_OUTPUT([Makefile libmpdclient.pc])
