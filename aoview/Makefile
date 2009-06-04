VERSION=$(shell git describe)
MODULES=gtk+-2.0 libglade-2.0 gconf-2.0
INCLUDES=$(shell pkg-config --cflags $(MODULES)) -I..
WARN= -Wall -Wpointer-arith -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -fno-strict-aliasing
CFLAGS=$(INCLUDES) -O0 -g $(WARN) -DAOVIEW_VERSION='"$(VERSION)"'
LIBS=$(shell pkg-config --libs $(MODULES)) -lm

BIN=/usr/local/bin

SRC = \
	aoview_main.c \
	aoview_dev.c \
	aoview_dev_dialog.c \
	aoview_serial.c \
	aoview_monitor.c \
	aoview_state.c \
	aoview_convert.c \
	aoview_log.c \
	aoview_table.c \
	aoview_util.c \
	aoview_file.c \
	aoview_eeprom.c

INC = \
	aoview.h

OBJ = \
	$(SRC:.c=.o)

PROG = aoview

$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBS)

$(OBJ): $(INC)

aoview_main.o: aoview_glade.h

clean:
	rm -f $(OBJ) $(PROG)

install: $(BIN)/aoview

$(BIN)/aoview: aoview
	install aoview $(BIN)

aoview_glade.h: aoview.glade
	sed -e 's/"/\\"/g' -e 's/^/"/' -e 's/$$/"/' $< > $@
