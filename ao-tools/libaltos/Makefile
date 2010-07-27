OS:=$(shell uname)

ifeq ($(OS),Linux)

JAVA_CFLAGS=-I/usr/lib/jvm/java-6-openjdk/include -I.

OS_CFLAGS=-DLINUX -DPOSIX_TTY $(JAVA_CFLAGS)

LIBEXT=so

endif

ifeq ($(OS),Darwin)

DARWIN_CFLAGS=\
	-I/System/Library/Frameworks/JavaVM.framework/Headers \
	-I/System/Library/Frameworks/IOKit.framework/Headers \
	-I/System/Library/Frameworks/CoreFoundation.framework/Headers
DARWIN_LIBS=\
	-framework IOKit -framework CoreFoundation

OS_CFLAGS = $(DARWIN_CFLAGS) -DDARWIN -DPOSIX_TTY
LIBEXT=dylib

endif

.SUFFIXES: .java .class

CLASSPATH=".:jnitest/*:libaltosJNI:/usr/share/java/*"

SWIG_DIR=swig_bindings/java
SWIG_FILE=$(SWIG_DIR)/libaltos.swig
SWIG_WRAP=$(SWIG_DIR)/libaltos_wrap.c

JNI_DIR=libaltosJNI
JNI_FILE=$(JNI_DIR)/libaltosJNI.java 
JNI_SRCS=$(JNI_FILE) \
	$(JNI_DIR)/SWIGTYPE_p_altos_file.java \
	$(JNI_DIR)/SWIGTYPE_p_altos_list.java \
	$(JNI_DIR)/altos_device.java \
	$(JNI_DIR)/libaltos.java

JAVAFILES=\
	$(JNI_SRCS)

CLASSFILES = $(JAVAFILES:%.java=%.class)

JAVAFLAGS=-Xlint:unchecked

all: libaltos.$(LIBEXT) cjnitest $(CLASSFILES)

.java.class:
	javac -cp "$(CLASSPATH)" $(JAVAFLAGS) $*.java

CFLAGS=$(OS_CFLAGS) -O0 -g

HEADERS=libaltos.h
SRCS = libaltos.c $(SWIG_WRAP)
OBJS = $(SRCS:%.c=%.o)
LIBS = $(DARWIN_LIBS)

cjnitest: cjnitest.o $(OBJS)
	cc -o $@ $(CFLAGS) cjnitest.o $(OBJS) $(LIBS)

libaltos.$(LIBEXT): $(OBJS)
	gcc -shared -o $@ $(CFLAGS) $(OBJS) $(LIBS)

clean:
	rm -f $(CLASSFILES) $(OBJS) libaltos.$(LIBEXT) cjnitest

$(JNI_FILE): libaltos.i0 $(HEADERS)
	mkdir -p $(SWIG_DIR)
	mkdir -p libaltosJNI
	sed 's;//%;%;' libaltos.i0 $(HEADERS) > $(SWIG_FILE)
	swig -java -package libaltosJNI $(SWIG_FILE)
	cp swig_bindings/java/*.java libaltosJNI

$(SWIG_WRAP): $(JNI_FILE)
