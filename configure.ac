dnl 
dnl  Copyright © 2008 Keith Packard <keithp@keithp.com>
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

AM_INIT_AUTOMAKE(cctools, 0.1)
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

dnl ==========================================================================

AM_CONDITIONAL(CROSS_COMPILING, test $cross_compiling = yes)

dnl ==========================================================================

# Setup for compiling build tools (fc-glyphname, etc)
AC_MSG_CHECKING([for a C compiler for build tools])
if test $cross_compiling = yes; then
  AC_CHECK_PROGS(CC_FOR_BUILD, gcc cc)
else
  CC_FOR_BUILD=$CC
fi
AC_MSG_RESULT([$CC_FOR_BUILD])
AC_SUBST(CC_FOR_BUILD)

PKG_CHECK_MODULES([LIBUSB], [libusb-1.0])

AC_MSG_CHECKING([for suffix of executable build tools])
if test $cross_compiling = yes; then
  cat >conftest.c <<\_______EOF
int
main ()
{
  exit (0);
}
_______EOF
  for i in .exe ""; do
    compile="$CC_FOR_BUILD conftest.c -o conftest$i"
    if AC_TRY_EVAL(compile); then
      if (./conftest) 2>&AC_FD_CC; then
	EXEEXT_FOR_BUILD=$i
	break
      fi
    fi
  done
  rm -f conftest*
  if test "${EXEEXT_FOR_BUILD+set}" != set; then
    AC_MSG_ERROR([Cannot determine suffix of executable build tools])
  fi
else
  EXEEXT_FOR_BUILD=$EXEEXT
fi
AC_MSG_RESULT([$EXEEXT_FOR_BUILD])
AC_SUBST(EXEEXT_FOR_BUILD)

USB_LIBS="-lusb"
AC_SUBST(USB_LIBS)

AC_OUTPUT([
Makefile 
lib/Makefile
ccload/Makefile
s51/Makefile
])
