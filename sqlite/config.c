#include <mmalloc.h>

#include <err.h>

size_t mmsize( void *ptr );
size_t mmrequired( size_t size );

/*
 * The sqlite3_config() interface may only be invoked prior to library
 * initialization using sqlite3_initialize() or after shutdown by
 * sqlite3_shutdown().
 * https://www.sqlite.org/c3ref/config.html
 */

#if false
typedef struct sqlite3_mem_methods sqlite3_mem_methods;
struct sqlite3_mem_methods {
  void *(*xMalloc)(int);         /* Memory allocation function */
  void (*xFree)(void*);          /* Free a prior allocation */
  void *(*xRealloc)(void*,int);  /* Resize an allocation */
  int (*xSize)(void*);           /* Return the size of an allocation */
  int (*xRoundup)(int);          /* Round up request size to allocation size */
  int (*xInit)(void*);           /* Initialize the memory allocator */
  void (*xShutdown)(void*);      /* Deinitialize the memory allocator */
  void *pAppData;                /* Argument to xInit() and xShutdown() */
};
#endif

static void
*xMalloc( int size ) {
  mmalloc(size);
}

static void
xFree( void *p ) {
  mfree(p);
}

static void
*xRealloc( void *ptr, int size ) {
  mrealloc(ptr, size);
}

static int
xSize( void *ptr ) {
  return (int) mmsize(ptr);
}

static int
xRoundup( int size ) {
  return (int) mmrequired(size);
}

static int
xInit( void *p ) {
  return SQLITE_OK;
}

static void
xShutdown( void *p ) {
  ;
}


sqlite3_mem_methods methods = {
  xMalloc,  xFree, xRealloc, xSize, xRoundup, xInit, xShutdown,
  NULL
};

int
use_libmmalloc() {
  int erc;

  /*
   * SQLITE_CONFIG_MALLOC The SQLITE_CONFIG_MALLOC option takes a
   * single argument which is a pointer to an instance of the
   * sqlite3_mem_methods structure. The argument specifies
   * alternative low-level memory allocation routines to be used in
   * place of the memory allocation routines built into
   * SQLite. SQLite makes its own private copy of the content of the
   * sqlite3_mem_methods structure before the sqlite3_config() call
   * returns.
   */

  if( (erc = sqlite3_config(SQLITE_CONFIG_MALLOC, &methods)) != SQLITE_OK ) {
    errx(EXIT_FAILURE, "%s", sqlite3_errstr(erc));
  }

  return erc;
}
	 


     
