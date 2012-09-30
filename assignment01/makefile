OBJS = main.o
CC = gcc
CFLAGS = -Wall -c 
LFLAGS = -Wall
SOURCES = main.c
HEADERS = 
EXECNAME = graphexec

all: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(EXECNAME)

main.o : main.c
	$(CC) $(CFLAGS) main.c

clean: 
	\rm -f $(OBJS) $(EXECNAME)

tar:
	tar cfv assignment01.tar makefile $(SOURCES) $(HEADERS)
