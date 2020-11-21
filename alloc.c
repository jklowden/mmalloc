#include <assert.h>
#include <err.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <checksum.h>

#define COUNT_OF(x) sizeof(x)/sizeof((x)[0])

static size_t page_size;
static const char *log_dir, *log_backtrace;

struct mgmt_t {
  char *base;
  size_t len;
  uint32_t crc;
};

enum { mgmt_size = sizeof(struct mgmt_t) };

static void
Warnx(const char *format, ...) {

if( NULL == getenv("MMALLOC_VERBOSE") ) return;

  va_list argp;

  va_start(argp, format);

  vwarnx(format, argp);
}
static char log_filename[PATH_MAX];

void
mlog( const char func[], int line, const void *ptr ) {
  static size_t nalloc = 0;
  const struct mgmt_t *mgmt = (void*)((const char*)ptr - mgmt_size);
  char *s = log_filename; // re-use filename buffer 
  int fd;
  
  if( ! log_dir ) return;

  sprintf(log_filename, "%s/%p", log_dir, mgmt->base);

  const int flags = O_WRONLY | O_CREAT | O_APPEND;
  const mode_t mode = S_IRUSR | S_IWUSR;
  
  if( (fd = open(log_filename, flags, mode)) == -1 ) {
    warn("%s", log_filename);
    return;
  }

  /*
   * If called with a function and line, report it to the log file. 
   * If not, write the backtrace instead. 
   */
  ++nalloc;
  if( func ) {
    int len = sprintf(s, "%s:%d [%zu]: %zu bytes allocated at %p, %zu total\n",
		      func, line, nalloc,
		      (mgmt->base + mgmt->len) - (char*)ptr,
		      ptr,
		      mgmt->len);

    if( -1 == write(fd, s, len) ) {
      err(EXIT_FAILURE, "%s", func);
    }
  } else { // print backtrace
    static char symbols[1024 * 24];
    void *bt[24];

    int len = sprintf(s, "#%zu: %zu bytes allocated at %p, %zu total\n",
		      nalloc,
		      (mgmt->base + mgmt->len) - (char*)ptr,
		      ptr,
		      mgmt->len);

    if( -1 == write(fd, s, len) ) {
      err(EXIT_FAILURE, "%s", func);
    }

    int nbt = backtrace(bt, COUNT_OF(bt));

    backtrace_symbols_fd(bt, nbt, fd);
  }

  if( -1 == close(fd) ) {
    err(EXIT_FAILURE, "%s", func);
  }
}

static void
munlog( const void *ptr ) {
  if( ! log_dir ) return;
  if( log_backtrace ) return;
  
  const struct mgmt_t *mgmt = (void*)((const char*)ptr - mgmt_size);
  sprintf(log_filename, "%s/%p", log_dir, mgmt->base);
  
  if( -1 == unlink(log_filename) ) {
    warn(__func__);
  } else {
    Warnx("removed %s", log_filename);
  }
}
  
static inline size_t
allocation_size( size_t size ) {
  return (1 + (mgmt_size + size) / page_size) * page_size;
}

// compute the pointer returned by malloc for page(s) described by mgmt_t. 
static inline void *
malloc_at( const struct mgmt_t mgmt, size_t size ) {
  return mgmt.base + mgmt.len - size;
}

/*
 * Allocate 1 or more pages of memory.  Put allocation description
 * "behind" the returned pointer.  Return pointer to memory, just as
 * malloc(3).
 */
static char *
map_anon( size_t size ) {
  enum { offset = 0,
	 prot = PROT_READ | PROT_WRITE,
	 flags = MAP_PRIVATE | MAP_ANONYMOUS };
  
  if( page_size == 0 ) {
    if( (page_size = sysconf(_SC_PAGESIZE)) == -1 ) {
      err(EXIT_FAILURE, "sysconf _SC_PAGESIZE"); 
    }
    Warnx("_SC_PAGESIZE: %ld bytes, mgmt: %zu bytes", page_size, mgmt_size);

    if( (log_dir = getenv("MMALLOC_LOGDIR")) != NULL ) {
      size_t len;
      if( (len = strlen(log_dir)) > PATH_MAX - 32 ) {
	errx(EXIT_FAILURE, "MMALLOC_LOGDIR name '%s' is %zu characters long, "
	     "cannot be more than  %d = PATH_MAX - 32",
	     log_dir, len, PATH_MAX - 32);
      }
    }
    log_backtrace = getenv("MMALLOC_BACKTRACE");
  }
  assert(page_size > 0);

  struct mgmt_t mgmt = { .len = allocation_size(size) };

  assert(mgmt.len >= mgmt_size + size);  

  mgmt.base = mmap(NULL, mgmt.len, prot, flags, -1, 0);
  if( mgmt.base == NULL ) {
    err(EXIT_FAILURE, "%s:%d: mmap", __func__, __LINE__ );
  }

  char *p = malloc_at(mgmt, size) - mgmt_size;

  if( true ) {
    char *pend = mgmt.base + mgmt.len;
    Warnx("returning %p, %zu bytes back from %p",
	  p + mgmt_size, pend - (p + mgmt_size), pend);
  }

  memcpy(p, &mgmt, mgmt_size);

  char *pcrc = p + ( (char*)&mgmt.crc - (char*)&mgmt );
  size_t len = pcrc - mgmt.base; // allocated space up to the CRC
  mgmt.crc = crc_32(mgmt.base, len);
  memcpy(pcrc, &mgmt.crc, sizeof(mgmt.crc)); // pcrc might not be aligned

#if 0
  warnx( "protecting %zu bytes (%p-%p) + CRC (%zu bytes)\n"
	 "for size %zu at %p, ending at %p, total block %zu",
	 len, mgmt.base, pcrc, sizeof(mgmt.crc),
	 size, p + mgmt_size, p + mgmt_size + size,
	 (p + mgmt_size + size) - mgmt.base );
#endif
  
  if( log_backtrace ) {
    mlog(NULL, 0, p + mgmt_size);
  }

  return p + mgmt_size;
}

void *
mmalloc(size_t size) {
  return map_anon(size);
}

void
mfree(void *ptr) {
  if( ptr == NULL ) {
    return;
  }

  struct mgmt_t mgmt;
  memcpy(&mgmt, (char*)ptr - mgmt_size, mgmt_size);

  // verify ptr is between base and base+len
  if( !( mgmt.base < (char*)ptr && (char*)ptr < mgmt.base + mgmt.len) ) {
    errx(EXIT_FAILURE, "%s:%d: allocation corrupted before '%x' at %p",
	 __func__, __LINE__, *(char*)ptr, ptr );
  }

  // verify ptr is on same page as mgmt.base
  if( (char*)ptr - mgmt.base > page_size ) {
    errx(EXIT_FAILURE, "%s:%d: internal pointer frobbed: "
	 "%p is %zu from %p, should be <= %zu ",
	 __func__, __LINE__,
	 mgmt.base, (char*)ptr - mgmt.base, ptr, page_size );
  }

  // verify pointer, length, and unused portion are unmodified
  struct mgmt_t *p = (void*)((char*)ptr - mgmt_size);
  size_t len = (char*)&p->crc - mgmt.base;
  mgmt.crc = crc_32(mgmt.base, len);
  if( 0 != memcmp(&p->crc, &mgmt.crc, sizeof(mgmt.crc)) ) {
    errx(EXIT_FAILURE, "%s:%d: internal page frobbed: "
	 "0x%x != 0x%x for %p, len %zu ",
	 __func__, __LINE__,
	 mgmt.crc, p->crc, p, len );
  }

  for( char *p = mgmt.base; p < ((char*)ptr - mgmt_size); p++ ) {
    if( *p != '\0' ) {
      errx(EXIT_FAILURE, "%s:%d: nonzero memory "
	   "'%x' at %p, %zu bytes before %p", __func__, __LINE__,
	   *p, p, (char*)ptr - p, ptr );
    }
  }
  if( -1 ==  munmap(mgmt.base, mgmt.len) ) {
    err(EXIT_FAILURE, "%s:%d: munmap", __func__, __LINE__);
  }

  munlog(ptr);
}

void *
mcalloc(size_t nelem, size_t size) {
  return map_anon(nelem * size);
}

static inline size_t
min( size_t a, size_t b) {
  return a < b? a : b;
}

void *
mrealloc(void *ptr, size_t size) {
  if( NULL == ptr ) return map_anon(size);

  struct mgmt_t old;
  memcpy(&old, (char*)ptr - mgmt_size, mgmt_size);

  void *p = map_anon(size);

  memcpy(p, old.base, min(size, old.len));

  mfree(ptr);

  return p;
}

size_t
mmsize( void *ptr ) {
  const struct mgmt_t *pmgmt = (struct mgmt_t*)ptr - 1;
  return pmgmt->len;
}

size_t
mmrequired( size_t size ) {
  size += mgmt_size;

  if( size < page_size ) return page_size;

  return page_size + page_size * (size / page_size);
}
