CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = gitt

all: $(PROGS)

gitt: main.c gitt.c hash.c list.c sha1.c

clean:
	rm -rf *~ $(PROGS) .gitt