dnl
dnl  Copyright © 2008,2009 Keith Packard <keithp@keithp.com>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License along
dnl  with this program; if not, write to the Free Software Foundation, Inc.,
dnl  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
dnl
dnl Process this file with autoconf to create configure.

AC_PREREQ(2.57)
AC_INIT([altos], 0.7.1)
AC_CONFIG_SRCDIR([src/ao.h])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

VERSION_DASH=`echo $VERSION | sed 's/\./-/g'`
AC_SUBST(VERSION_DASH)

dnl ==========================================================================

AM_CONFIG_HEADER(config.h)

AC_ARG_WITH(freetts, AS_HELP_STRING([--with-freetts=PATH],
	[Set freetts class path (default /usr/share/java)]),
	[FREETTS=$withval], [FREETTS=/usr/share/java])

AC_SUBST(FREETTS)

AC_ARG_WITH(jfreechart, AS_HELP_STRING([--with-jfreechart=PATH],
	[Set jfreechart class path (default /usr/share/java)]),
	[JFREECHART=$withval], [JFREECHART=/usr/share/java])

AC_SUBST(JFREECHART)

AC_ARG_WITH(jcommon, AS_HELP_STRING([--with-jcommon=PATH],
	[Set jcommon class path (default /usr/share/java)]),
	[JCOMMON=$withval], [JCOMMON=/usr/share/java])

AC_SUBST(JCOMMON)

AC_ARG_WITH(jvm, AS_HELP_STRING([--with-jvm-include=PATH],
	[Set jvm include path for jni builds (default searches in /usr/lib/jvm)]),
	[JVM_INCLUDE=$withval], [JVM_INCLUDE=auto])

if test "x$JVM_INCLUDE" = "xauto"; then
	AC_MSG_CHECKING([JVM include files])
	for jvm in default-java java-6-openjdk java-6-sun; do
		if test "x$JVM_INCLUDE" = "xauto"; then
			INCLUDE="/usr/lib/jvm/$jvm/include"
			if test -f "$INCLUDE"/jni.h; then
				JVM_INCLUDE="$INCLUDE"
			fi
		fi
	done
	if test "x$JVM_INCLUDE" = "xauto"; then
		AC_MSG_ERROR([no JVM include files found])
	fi
	AC_MSG_RESULT([$JVM_INCLUDE])
fi

AC_SUBST(JVM_INCLUDE)

AC_ARG_WITH(fat-dir, AS_HELP_STRING([--with-fat-dir=PATH],
	    [Set the directory to install the 'fat' distribution files to (defaults to not installing)]),
	    [FATDIR=$withval], [FATDIR=none])

AM_CONDITIONAL(FATINSTALL, [test "x$FATDIR" != "xnone"])

AC_SUBST(FATDIR)

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

CFLAGS="-g"
WARN_CFLAGS=""
if test "x$GCC" = "xyes"; then
	WARN_CFLAGS="-Wall -Wpointer-arith -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations \
	-Wnested-externs -fno-strict-aliasing"
	AC_DEFINE_UNQUOTED(HAVE_WARNING_CPP_DIRECTIVE,1,
	[Can use #warning in C files])
fi
AC_SUBST(WARN_CFLAGS)

AC_CHECK_HEADERS(flite/flite.h,HAVE_FLITE_H=yes,HAVE_FLITE_H=no)
AC_CHECK_LIB(flite, flite_init,HAVE_LIBFLITE=yes,HAVE_LIBFLITE=no,-lasound -lm)

if test "x$HAVE_FLITE_H" = "xyes" -a "x$HAVE_LIBFLITE" = "xyes"; then
	AC_DEFINE(HAVE_FLITE,1,[Define if the flite library is usable])
	AC_SUBST(FLITE_LIBS,"-lflite_cmu_us_kal16 -lflite_usenglish -lflite_cmulex -lflite -lasound -lm")
	AC_SUBST(FLITE_INCS,-Iflite)
	save_LIBS="$LIBS"
	LIBS="$LIBS $FLITE_LIBS"
	AC_CHECK_FUNCS([register_cmu_us_kal16 register_cmu_us_kal],break)
	LIBS="$save_LIBS"
fi
AM_CONDITIONAL(USE_FLITE,test "x$HAVE_FLITE_H" = "xyes" -a "x$HAVE_LIBFLITE" = "xyes")

AC_CHECK_PROG([HAVE_SDCC], [sdcc], yes, no)
if test "x$HAVE_SDCC" = "xno"; then
	AC_MSG_ERROR([Please install sdcc to build AltOs])
fi

AC_CHECK_PROG([HAVE_NICKLE], [nickle], yes, no)
if test "x$HAVE_NICKLE" = "xno"; then
	AC_MSG_ERROR([Please install nickle to build AltOs])
fi

AC_CHECK_LIB(readline, readline)

PKG_CHECK_MODULES([GNOME], [gtk+-2.0 libglade-2.0 gconf-2.0])

PKG_CHECK_MODULES([LIBUSB], [libusb-1.0])

PKG_CHECK_MODULES([ALSA], [alsa])

PKG_CHECK_MODULES([PLPLOT], [plplotd])

PKG_CHECK_MODULES([SNDFILE], [sndfile])

AC_OUTPUT([
Makefile
altosui/Makefile
altosui/libaltos/Makefile
ao-tools/Makefile
ao-tools/lib/Makefile
ao-tools/ao-rawload/Makefile
ao-tools/ao-dbg/Makefile
ao-tools/ao-dumplog/Makefile
ao-tools/ao-bitbang/Makefile
ao-tools/ao-eeprom/Makefile
ao-tools/ao-list/Makefile
ao-tools/ao-load/Makefile
ao-tools/ao-postflight/Makefile
ao-tools/ao-view/Makefile
ao-utils/Makefile
src/Version
])
