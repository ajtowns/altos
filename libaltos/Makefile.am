JAVAC=javac
AM_CFLAGS=-DLINUX -DPOSIX_TTY -I$(JVM_INCLUDE)
AM_JAVACFLAGS=-encoding UTF-8

altoslibdir=$(libdir)/altos

altoslib_LTLIBRARIES=libaltos.la

libaltos_la_LDFLAGS = -version-info 1:0:1

libaltos_la_SOURCES=\
	libaltos.c \
	libaltos_wrap.c

noinst_PROGRAMS=cjnitest

cjnitest_LDADD=libaltos.la

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
