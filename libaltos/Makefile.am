JAVAC=javac
AM_CFLAGS=-DLINUX -DPOSIX_TTY -I$(JVM_INCLUDE)
AM_JAVACFLAGS=-target 1.6 -encoding UTF-8 -Xlint:deprecation -source 6

altoslibdir=$(libdir)/altos

altoslib_LTLIBRARIES=libaltos.la
libaltos_la_LDFLAGS=-version-info 1:0:1 -Wl,-znoexecstack

libaltos_la_SOURCES=\
	libaltos.c \
	libaltos_wrap.c

noinst_PROGRAMS=cjnitest

cjnitest_SOURCES=cjnitest.c
cjnitest_LDADD=libaltos.la

if MULTI_ARCH
altoslib_LTLIBRARIES+=libaltos32.la libaltos64.la

libaltos32_la_LDFLAGS=$(libaltos_la_LDFLAGS)
libaltos64_la_LDFLAGS=$(libaltos_la_LDFLAGS)

libaltos32_la_CFLAGS=-m32 $(AM_CFLAGS)
libaltos64_la_CFLAGS=-m64 $(AM_CFLAGS)

libaltos32_la_SOURCES=$(libaltos_la_SOURCES)
libaltos64_la_SOURCES=$(libaltos_la_SOURCES)

noinst_PROGRAMS+=cjnitest32 cjnitest64

cjnitest32_CFLAGS=-m32
cjnitest64_CFLAGS=-m64

cjnitest32_SOURCES=$(cjnitest_SOURCES)
cjnitest64_SOURCES=$(cjnitest_SOURCES)

cjnitest32_LDADD=libaltos32.la
cjnitest64_LDADD=libaltos64.la

endif


LIBS=-lbluetooth

HFILES=libaltos.h

SWIG_FILE=libaltos.swig

CLASSDIR=libaltosJNI

$(SWIG_FILE): libaltos.i0 $(HFILES)
	sed 's;//%;%;' libaltos.i0 $(HFILES) > $(SWIG_FILE)

all-local: classlibaltos.stamp

libaltos_wrap.c: classlibaltos.stamp

classlibaltos.stamp: $(SWIG_FILE)
	swig -java -package libaltosJNI $(SWIG_FILE)
	mkdir -p libaltosJNI
	$(JAVAC) -d . $(AM_JAVACFLAGS) $(JAVACFLAGS) *.java && \
	touch classlibaltos.stamp

MINGCC32=i686-w64-mingw32-gcc
MINGCC64=x86_64-w64-mingw32-gcc
MINGFLAGS=-Wall -DWINDOWS -DBUILD_DLL -I$(JVM_INCLUDE)
MINGLIBS=-lsetupapi

fat: all altos.dll altos64.dll

altos.dll: $(libaltos_la_SOURCES)
	$(MINGCC32) -o $@ $(MINGFLAGS) -shared $(libaltos_la_SOURCES) $(MINGLIBS)

altos64.dll: $(libaltos_la_SOURCES)
	$(MINGCC64) -o $@ $(MINGFLAGS) -shared $(libaltos_la_SOURCES) $(MINGLIBS)

clean-local:
	-rm -rf libaltosJNI *.class *.java classlibaltos.stamp $(SWIG_FILE) libaltos_wrap.c altos.dll altos64.dll
