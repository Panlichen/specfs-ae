CC=gcc
CCPLUS=g++
CFLAGS=-Wall -g 

all: largefile smallfile prealloc_test rbtree_test

largefile: largefile.c stats.c
	$(CC) $(CFLAGS) -o largefile largefile.c stats.c

smallfile: smallfile.c stats.c
	$(CC) $(CFLAGS) -o smallfile smallfile.c stats.c

prealloc_test: prealloc_test.cpp
	$(CCPLUS) $(CFLAGS) -o prealloc_test prealloc_test.cpp

rbtree_test: rbtree_test.cpp
	$(CCPLUS) $(CFLAGS) -o rbtree_test rbtree_test.cpp

clean:
	rm -f largefile smallfile prealloc_test rbtree_test