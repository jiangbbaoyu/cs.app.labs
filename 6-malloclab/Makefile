CC = gcc
CFLAGS = -g -Wall
LDLIBS = -lpthread

all: mmaptest
mmaptest: 914_mmap.c csapp.h csapp.c
	$(CC) $(CFLAGS) -o mmaptest 914_mmap.c csapp.c $(LDLIBS)
clean:
	rm -rf mmaptest