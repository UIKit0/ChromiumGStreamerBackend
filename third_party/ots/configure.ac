AC_INIT([OTS],
        [5.0.1],
        [https://github.com/khaledhosny/ots/issues],
        [ots],
        [https://github.com/khaledhosny/ots])

AC_CANONICAL_SYSTEM
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_CYGWIN
AC_EXEEXT
AC_OBJEXT
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG([0.20])

have_zlib=true
AX_CHECK_ZLIB([], [have_zlib=false])
if ! $have_zlib; then
	AC_MSG_ERROR([zlib is required but not found])
fi

have_freetype=false
PKG_CHECK_MODULES(FREETYPE, [freetype2], [have_freetype=true], :)
if $have_freetype; then
	AC_DEFINE(HAVE_FREETYPE, 1, [Have FreeType 2 library])
fi
AM_CONDITIONAL(HAVE_FREETYPE, [$have_freetype])

have_pthread=false
AX_PTHREAD([have_pthread=true])
if $have_pthread; then
	GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=1"
else
	GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=0"
fi
AC_SUBST([GTEST_CPPFLAGS])

have_coretext=false
AC_CHECK_TYPE([CTFontRef], [have_coretext=true], [], [#include <ApplicationServices/ApplicationServices.h>])
if $have_coretext; then
	CORETEXT_CFLAGS=
	CORETEXT_LIBS="-framework ApplicationServices"
	AC_DEFINE(HAVE_CORETEXT, 1, [Have Core Text framework])
fi
AC_SUBST(CORETEXT_CFLAGS)
AC_SUBST(CORETEXT_LIBS)
AM_CONDITIONAL(HAVE_CORETEXT, [$have_coretext])

have_systime=false
AC_CHECK_HEADER([sys/time.h], [have_systime=true])
AM_CONDITIONAL(HAVE_SYSTIME, [$have_systime])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
