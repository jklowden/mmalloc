#include <assert.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <mmalloc.h>

#include <setjmp.h>
#include <signal.h>

static sigjmp_buf sigjmp_buffer;

void
mark_segfault(int sig) {
  assert(sig == SIGSEGV);
  siglongjmp( sigjmp_buffer, sig );
}

int
main(int argc, char *argv[])
{
  int *p = calloc(2, sizeof(int));
  p[0] = 11;
  p[1] = 12;

  printf( "%d, %d\n", p[0], p[1]);
  fflush(stdout);

  if( SIG_ERR == signal(SIGSEGV, mark_segfault) ) {
    err(EXIT_FAILURE, "signal");
  }
  
  int faulted = sigsetjmp(sigjmp_buffer, 1);

  if( faulted == 0 ) {
    p[2] = 13;
    errx( EXIT_FAILURE, "%s:%d: succeeded in writing '%d' past the end ",
	  __func__, __LINE__, p[2] );
  }
  // If we got this far, the signal was handled. 
  printf("attempt write to one past the end was %s\n",
	 faulted != 0? "trapped" : "not trapped");
  faulted = 0;

  p[-2] = 'A';
  
  free(p);
  
  return EXIT_SUCCESS;
}
