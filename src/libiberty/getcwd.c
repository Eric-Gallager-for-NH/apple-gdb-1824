/* getcwd.c
 * Emulate getcwd using getwd.
 * This function is in the public domain. */

/*

@deftypefn Supplemental char* getcwd (char *@var{pathname}, int @var{len})

Copy the absolute pathname for the current working directory into
@var{pathname}, which is assumed to point to a buffer of at least
@var{len} bytes, and return a pointer to the buffer. If the current
directory's path does NOT fit in @var{len} characters, then the result is
@code{NULL} and @code{errno} is set. If @var{pathname} is a null pointer,
then @code{getcwd} will obtain @var{len} bytes of space using
@code{malloc}.

@end deftypefn

*/

/* includes: */
#include "config.h"

#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#else
# warning "getcwd.c expects <sys/param.h> to be included."
#endif /* HAVE_SYS_PARAM_H */
#include <errno.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
# warning "getcwd.c expects <string.h> to be included."
#endif /* HAVE_STRING_H */
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#else
# warning "getcwd.c expects <stdlib.h> to be included."
#endif /* HAVE_STDLIB_H */

/* prototypes: */
extern char *getwd(char *buf, size_t len);
extern int errno;

/* defines */
#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif /* !MAXPATHLEN */

/* actual function: */
char *getcwd(char *buf, size_t len)
{
  char ourbuf[MAXPATHLEN];
  char *result;

  result = getwd (ourbuf);
  if (result) {
    if (strlen (ourbuf) >= len) {
      errno = ERANGE;
      return 0;
    }
    if (!buf) {
       buf = (char*)malloc(len);
       if (!buf) {
           errno = ENOMEM;
	   return 0;
       }
    }
    strcpy (buf, ourbuf);
  }
  return buf;
}

/* EOF */
