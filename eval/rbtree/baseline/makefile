SOURCES = $(wildcard *.c)

CC = gcc
CFLAGS = $(shell pkg-config fuse --cflags) -pthread
LDFLAGS = $(shell pkg-config fuse --libs)
CFLAGS += -Wall
#CFLAGS += -DDEBUG
#CFLAGS += -O3

atomfs-fuse : $(SOURCES:.c=.o)
	$(CC) -o $@ $^ $(LDFLAGS)

atomfs-fuse-debug: $(SOURCES:.c=-debug.o)

debug: atomfs-fuse-debug
	-mkdir -p /tmp/fs > /dev/null 2>&1
	-sudo umount /tmp/fs >/dev/null 2>&1
	echo "r -n /dev/pmem0 /tmp/fs" > /tmp/parms
	exec gdb -x /tmp/parms ./atomfs-fuse-debug

run: atomfs-fuse 
	-mkdir -p ~/atomfs > /dev/null 2>&1
	./atomfs-fuse -n /dev/pmem0 ~/atomfs

%-debug.o: CFLAGS += -g -Wall -Wno-pointer-sign -DDEBUG
%-debug.o %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean :
	rm -f *.o
	rm -f atomfs-fuse 
