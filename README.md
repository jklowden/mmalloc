* mmalloc - A malloc(3) implementation based on mmap

mmalloc is a C library that replaces the malloc(3) dynamic memory functions defined in the C standard library.  It is useful for testing and debugging because it isolates every allocation (by using operating system resources).  It is not fast or efficient, however, and is probably not suitable for production use.  

To use it, 

- install libcrc from https://github.com/lammertb/libcrc
- build libmmalloc.a using the Makefile
- compile your C program including mmalloc.h 
- link your C program to libmmalloc.a and libcrc.a

Your program source is unchanged.  The header file mmalloc.h redefines malloc et. al. using a preprocessor macro.  

mmalloc calls mmap(2) for every call to malloc, calloc, or realloc.  It creates an anonymous, private map of at least one page (typically 4096 bytes).  Because each allocation is separately mapped, it is protected by the operating system.  Attempts to write outside the mapped area result in a SIGSEV signal.  By default, SIGSEV results in the process being terminated by the application.  

The pointer returned by mmalloc is near the end of the page allocated by mmap: the last by requested of malloc is the last byte allocated by mmap.  This has two consequences: 

1.  Attempts to write beyond the end of the requested space result in SIGSEGV. 

2.  The returned pointer may not be on a "paragraph" boundary. The address may be odd i.e., may have its 1 bit set.  This may break assumptions about how malloc works, but Posix is silent on the addresses returned by malloc, except to say that each returned pointer must be distinct.  

For example, on a system with 4K pages, `char *p = malloc(7)` will return an address `p` such that `p % 4096 == 4089`.  

In the unused space "before" the pointer, mmalloc maintains control information: 

-  the pointer returned by mmap, and the requested size. 
-  a crc32 checksum of the unused space (zeros, except for the pointer and size)

Although mmalloc cannot prevent writes to the unused space and control information, writes to that area cannot create any problems for mmalloc, and are reported when the user calls free: 

-  The saved pointer is checked for sanity.  The user's pointer must be on the first page returned by mmap. 

-  The saved length is checked for sanity.  The user's pointer must be in the allocated space, as stored in the control information. 

-  If those two tests pass, mfree computes a CRC on the unused space and saved pointer and length.  The saved CRC must match the computed one.  

If mfree detects any problem with the unused region, it exits with a message. 

