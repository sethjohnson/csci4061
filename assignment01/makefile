# login: joh08230
# date:  09/30/12
# name:  Seth Johnson, Michael Walton
# id:    4273042, 4192221


OBJS = main.o
CC = gcc
CFLAGS = -Wall -c 
LFLAGS = -Wall
SOURCES = main.c makeargv.c freemakeargv.c
HEADERS = 
EXECNAME = graphexec

all: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXECNAME)

main.o : main.c
	$(CC) $(CFLAGS) main.c

clean: 
	\rm -f $(OBJS) $(EXECNAME)

tar:
	tar cfv assignment01.tar.gz makefile README $(SOURCES) $(HEADERS)
