# https://github.com/lammertb/libcrc
LIBCRC.DIR = $(HOME)/projects/3rd/libcrc
LIBCRC = libcrc.a

.SUFFIXES: .a .so

CPPFLAGS = -D_BSD_SOURCE -I. -I$(LIBCRC.DIR)/include -DMALLOC_IS_MMALLOC
CFLAGS = -g -std=c11
LDFLAGS = -L$(LIBCRC.DIR)/lib -lcrc

all: t/try libmmalloc.a libmmalloc-override.a sysconf sqlite/config.o

libmmalloc.a: alloc.o
	$(AR) r $@ $^

libmmalloc-override.a: alloc.o override.o
	$(AR) r $@ $^

override.o: override.c
	$(CC) -c -o$@ $(CFLAGS) -D_BSD_SOURCE -I. -I$(LIBCRC.DIR)/include $^

t/try: t/try.o libmmalloc.a
	$(CC) -o $@ $(CPPFLAGS) $(CFLAGS) $< -L. -lmmalloc $(LDFLAGS) 

t/try.o: mmalloc.h

TAGS: *.[ch] */*.[ch]
	etags $^
