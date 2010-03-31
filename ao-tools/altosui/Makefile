.SUFFIXES: .java .class

CLASSPATH=..
CLASSFILES=AltosSerialMonitor.class AltosSerial.class AltosUI.class
JAVAFLAGS=-Xlint:unchecked

all: $(CLASSFILES)

.java.class:
	javac -cp $(CLASSPATH) $(JAVAFLAGS) $*.java

clean:
	rm -f *.class