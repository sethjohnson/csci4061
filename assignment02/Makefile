CC=gcc
CFLAGS=-g -Wall `pkg-config --cflags gtk+-2.0 webkit-1.0`
LDFLAGS+=`pkg-config --libs gtk+-2.0 webkit-1.0`
LIB32=/usr/lib
LIB64=/usr/lib
#LIB32=/soft/webkit/1.1.15.4/jaunty32/lib
#LIB64=/soft/webkit/1.1.15.4/jaunty64/lib
SOURCES=wrapper.c wrapper.h browser.c
OBJ=browser

LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
   LIB=${LIB64}
else
   LIB=${LIB32}
endif

all:  $(SOURCES) $(OBJ)

$(OBJ): $(SOURCES)
	$(CC) -L ${LIB} $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(OBJ) 

clean:
	rm -rf $(OBJ)

tar:
	tar cfv $(OBJ).tar.gz Makefile README $(SOURCES) $(HEADERS)
