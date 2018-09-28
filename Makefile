# https://github.com/lammertb/libcrc
LIBCRC.DIR = $(HOME)/projects/3rd/libcrc
LIBCRC = libcrc.a

CPPFLAGS = -D_BSD_SOURCE -I. -I$(LIBCRC.DIR)/include -DMALLOC_IS_MMALLOC
CFLAGS = -g -std=c11
LDFLAGS = -L$(LIBCRC.DIR)/lib -lcrc

all: t/try alloc.o sysconf

t/try: t/try.o alloc.o
	$(CC) -o $@ $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS)

t/try.o: mmalloc.h

