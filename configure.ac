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
AC_INIT([altos], 1.2)
AC_CONFIG_SRCDIR([src/core/ao.h])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

VERSION_DASH=`echo $VERSION | sed 's/\./-/g'`
AC_SUBST(VERSION_DASH)


dnl ==========================================================================
dnl Java library versions

ALTOSUILIB_VERSION=1
ALTOSLIB_VERSION=1

AC_SUBST(ALTOSLIB_VERSION)
AC_DEFINE(ALTOSLIB_VERSION,$ALTOSLIB_VERSION,[Version of the AltosLib package])
AC_SUBST(ALTOSUILIB_VERSION)
AC_DEFINE(ALTOSUILIB_VERSION,$ALTOSUILIB_VERSION,[Version of the AltosUILib package])

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

AC_ARG_WITH(android, AS_HELP_STRING([--with-android=PATH],
	[Set android SDK path (default searches in a variety of places)]),
	[ANDROID_SDK=$withval], [ANDROID_SDK=${ANDROID_SDK:-auto}])

if test "x$ANDROID_SDK" = "xauto"; then
	AC_MSG_CHECKING([Android SDK])
	for sdk in ../android/android-sdk-linux ../android/android-sdk ../android-sdk ../android-sdk-linux $HOME/android; do
		if test "x$ANDROID_SDK" = "xauto"; then
			SDK="$sdk"
			if test -f "$SDK/SDK Readme.txt"; then
				ANDROID_SDK=`readlink -m "$SDK"`
			fi
		fi
	done
	if test "x$ANDROID_SDK" = "xauto"; then
		AC_MSG_NOTICE([no Android SDK found])
		ANDROID_SDK=no
	fi
	AC_MSG_RESULT([$ANDROID_SDK])
fi
if test "x$ANDROID_SDK" != "xno"; then
	AC_MSG_CHECKING([Android SDK version])
	SDK_VERSION=`grep -i '^Pkg.Revision=' $ANDROID_SDK/tools/source.properties | cut -f2- -d= | sed s/^r//`
	SDK_MAJOR_VERSION="`echo $SDK_VERSION | sed 's/[[^0-9]].*//'`"
	if test "$SDK_MAJOR_VERSION" -lt 17 ; then
		AC_MSG_NOTICE([SDK version $SDK_VERSION is too old. Need >= r17])
		ANDROID_SDK=no
	else
		AC_MSG_RESULT([$SDK_VERSION])
	fi
fi
if test "x$ANDROID_SDK" != "xno"; then
	AC_MSG_CHECKING([Android SDK API Level])
	if test -f "$ANDROID_SDK/platforms/android-10/source.properties"; then
		API_LEVEL=`sed -ne 's/^AndroidVersion.ApiLevel=//p' $ANDROID_SDK/platforms/android-10/source.properties`
		if test "$API_LEVEL" = "10"; then
			AC_MSG_RESULT([$API_LEVEL])
		else
			AC_MSG_NOTICE([API Level 10 not installed correctly.])
			ANDROID_SDK=no
		fi
	else
		AC_MSG_NOTICE([API Level 10 not installed, but is required.])
		ANDROID_SDK=no
	fi
fi
if test "x$ANDROID_SDK" != "xno"; then
	AC_MSG_CHECKING([Android SDK required extras])
	if test -f "$ANDROID_SDK/extras/android/support/source.properties"; then
		ANDROID_SUPPORT_LIB_NAME=`sed -ne 's/^Extra.NameDisplay=//p' $ANDROID_SDK/extras/android/support/source.properties`
		if test -f "$ANDROID_SDK/extras/google/google_play_services/source.properties"; then
			GOOGLE_PLAY_SERVICES_NAME=`sed -ne 's/^Extra.NameDisplay=//p' $ANDROID_SDK/extras/google/google_play_services/source.properties`
			AC_MSG_RESULT([$ANDROID_SUPPORT_LIB_NAME, $GOOGLE_PLAY_SERVICES_NAME])
		else
			AC_MSG_NOTICE([Google Play Services library not installed.])
			ANDROID_SDK=no
		fi
	else
		AC_MSG_NOTICE([Android Support Library not installed.])
		ANDROID_SDK=no
	fi
fi

AM_CONDITIONAL([ANDROID], [test x$ANDROID_SDK != xno])

AC_SUBST(ANDROID_SDK)

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

AC_CHECK_PROG([HAVE_SDCC], [sdcc], yes, no)
if test "x$HAVE_SDCC" = "xno"; then
	AC_MSG_ERROR([Please install sdcc to build AltOs])
fi

AC_CHECK_PROG([HAVE_NICKLE], [nickle], yes, no)
if test "x$HAVE_NICKLE" = "xno"; then
	AC_MSG_ERROR([Please install nickle to build AltOs])
fi

AC_CHECK_LIB(readline, readline)

PKG_CHECK_MODULES([LIBUSB], [libusb-1.0])

AC_CHECK_HEADERS(libelf.h libelf/libelf.h, [break])
AC_CHECK_HEADERS(gelf.h libelf/gelf.h, [break])

PKG_CHECK_MODULES([LIBSTLINK], [stlink], [HAVE_STLINK=yes], [HAVE_STLINK=no])

AM_CONDITIONAL([LIBSTLINK], [test x$HAVE_STLINK != xno])

AC_OUTPUT([
Makefile
altoslib/Makefile
altosuilib/Makefile
altosuilib/AltosUIVersion.java
altosui/Makefile
altosui/Info.plist
altosui/altos-windows.nsi
libaltos/Makefile
micropeak/Makefile
micropeak/Info.plist
micropeak/micropeak-windows.nsi
altosdroid/Makefile
altosdroid/local.properties
ao-tools/Makefile
ao-tools/lib/Makefile
ao-tools/ao-rawload/Makefile
ao-tools/ao-dbg/Makefile
ao-tools/ao-bitbang/Makefile
ao-tools/ao-eeprom/Makefile
ao-tools/ao-list/Makefile
ao-tools/ao-load/Makefile
ao-tools/ao-telem/Makefile
ao-tools/ao-stmload/Makefile
ao-tools/ao-send-telem/Makefile
ao-tools/ao-sky-flash/Makefile
ao-tools/ao-dumpflash/Makefile
ao-tools/ao-edit-telem/Makefile
ao-tools/ao-dump-up/Makefile
ao-utils/Makefile
src/Version
])
