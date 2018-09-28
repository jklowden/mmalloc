# https://github.com/lammertb/libcrc
LIBCRC.DIR = $(HOME)/projects/3rd/libcrc
LIBCRC = libcrc.a

.SUFFIXES: .a .so

CPPFLAGS = -D_BSD_SOURCE -I. -I$(LIBCRC.DIR)/include -DMALLOC_IS_MMALLOC
CFLAGS = -g -std=c11
LDFLAGS = -L$(LIBCRC.DIR)/lib -lcrc

all: t/try libmmalloc.a sysconf

libmmalloc.a: alloc.o
	$(AR) r $@ $^

t/try: t/try.o libmmalloc.a
	$(CC) -o $@ $(CPPFLAGS) $(CFLAGS) $< -L. -lmmalloc $(LDFLAGS) 

t/try.o: mmalloc.h

