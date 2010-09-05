JAVAC=javac
AM_CFLAGS="-I$(JVM_INCLUDE)"
AM_JAVACFLAGS=-encoding UTF-8

lib_LTLIBRARIES=libaltos.la

libaltos_la_SOURCES=\
	libaltos.c

HFILES=libaltos.h

SWIG_FILE=libaltos.swig

CLASSDIR=libaltosJNI

$(SWIG_FILE): libaltos.i0 $(HFILES)
	sed 's;//%;%;' libaltos.i0 $(HFILES) > $(SWIG_FILE)

all-local: classlibaltos.stamp

classlibaltos.stamp: $(SWIG_FILE)
	swig -java -package libaltosJNI $(SWIG_FILE)
	mkdir -p libaltosJNI
	$(JAVAC) -d . $(AM_JAVACFLAGS) $(JAVACFLAGS) *.java && \
	touch classlibaltos.stamp

clean-local:
	-rm -rf libaltosJNI *.class *.java classlibaltos.stamp $(SWIG_FILE)