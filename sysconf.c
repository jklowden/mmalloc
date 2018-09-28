#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void /* Print 'msg' plus sysconf() value for 'name' */
sysconfPrint(const char *msg, int name)
{
  errno = 0;
  long lim;

  if( (lim = sysconf(name)) == -1 ) {
    err(EXIT_FAILURE, "sysconf %s", msg);
  }

  if( lim == 0 ) { /* Call succeeded, limit indeterminate */
    printf("%s (indeterminate)\n", msg);
  } else { /* Call succeeded, limit determinate */
    printf("%-18s %'9ld\n", msg, lim); 
  }
}

int
main(int argc, char *argv[])
{
  sysconfPrint("_SC_ARG_MAX", _SC_ARG_MAX);
  sysconfPrint("_SC_LOGIN_NAME_MAX", _SC_LOGIN_NAME_MAX);
  sysconfPrint("_SC_OPEN_MAX", _SC_OPEN_MAX);
  sysconfPrint("_SC_NGROUPS_MAX", _SC_NGROUPS_MAX);
  sysconfPrint("_SC_PAGESIZE", _SC_PAGESIZE);
  sysconfPrint("_SC_RTSIG_MAX", _SC_RTSIG_MAX);
  exit(EXIT_SUCCESS);
}


