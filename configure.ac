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

AC_INIT(COPYING)

AM_INIT_AUTOMAKE(altos, 0.1)
AM_MAINTAINER_MODE

dnl ==========================================================================

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
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
AC_CHECK_LIB(flite, flite_init,HAVE_LIBFLITE=yes,HAVE_LIBFLITE=no,-lm)

if test "x$HAVE_FLITE_H" = "xyes" -a "x$HAVE_LIBFLITE" = "xyes"; then
	AC_DEFINE(HAVE_FLITE,1,[Define if the flite library is usable])
	AC_SUBST(FLITE_LIBS,"-lflite_cmu_us_kal16 -lflite_usenglish -lflite_cmulex -lflite -lm")
	AC_SUBST(FLITE_INCS,-Iflite)
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

AC_OUTPUT([
Makefile
ao-view/Makefile
ao-tools/Makefile
ao-tools/lib/Makefile
ao-tools/ao-rawload/Makefile
ao-tools/ao-dbg/Makefile
ao-tools/ao-bitbang/Makefile
ao-tools/ao-eeprom/Makefile
ao-tools/ao-load/Makefile
ao-utils/Makefile
])
