CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = gitt

all: $(PROGS)

gitt: main.c gitt.c hash.c list.c sha1.c blob_tree.c

clean:
	rm -rf *~ $(PROGS) .gitt