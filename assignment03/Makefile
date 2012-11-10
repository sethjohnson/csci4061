CC=gcc
CFLAGS=-g -Wall
LIB32=/usr/lib
LIB64=/usr/lib

SOURCES=linked_list.c packet_public.c mm_public.c main_malloc.c main_mm.c
HEADERS=linked_list.h packet_public.h mm_public.h


LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
   LIB=${LIB64}
else
   LIB=${LIB32}
endif

OBJS=mm_public.o linked_list.o
all:  tests packet

tests : main_malloc main_mm

packet : packet_public.c packet_public.h  mm_public.o
	$(CC)  $(CFLAGS) packet_public.c $(OBJS) -o packet
	

main_malloc: main_malloc.c mm_public.o linked_list.o
	$(CC)  $(CFLAGS)  main_malloc.c $(OBJS) -o main_malloc 

main_mm: main_mm.c mm_public.o linked_list.o
	$(CC)  $(CFLAGS)  main_mm.c $(OBJS) -o main_mm 



mm_public.o: mm_public.c mm_public.h linked_list.o
	$(CC)  $(CFLAGS)  -c mm_public.c 

linked_list.o: linked_list.c linked_list.h
	$(CC)  $(CFLAGS) -c  linked_list.c 

clean:
	rm -rf main_mm main_malloc packet *.o

tar:
	tar cfv assignment03.tar.gz Makefile README $(SOURCES) $(HEADERS)



