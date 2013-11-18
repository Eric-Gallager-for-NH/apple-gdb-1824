/* main.c - main() and some logging routines for expect

Written by: Don Libes, NIST, 2/6/90

Design and implementation of this program was paid for by U.S. tax
dollars. Therefore it is public domain. However, the author and NIST
would appreciate credit if this program or parts of it are used.
*/

#include "expect_cf.h"
#include <stdio.h>
#include <unistd.h>
#ifdef NO_STDLIB_H
# include "../compat/stdlib.h"
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# else
#  ifdef HAVE_MALLOC_H
#   include <malloc.h>
#  else
#   ifdef HAVE_MALLOC_MALLOC_H
#    include <malloc/malloc.h>
#   else
#    warning exp_main_exp.c expects a header that provides malloc() to be included.
#   endif /* HAVE_MALLOC_MALLOC_H */
#  endif /* HAVE_MALLOC_H */
# endif /* HAVE_STDLIB_H */
#endif /*NO_STDLIB_H*/
#include "tcl.h"
#ifdef USE_ITCL
# include "itcl.h"
#endif /* USE_ITCL */
#include "expect_tcl.h"

int
main(argc, argv)
int argc;
char *argv[];
{
	int rc = 0;
	Tcl_Interp *interp = Tcl_CreateInterp();

	/* need this for [info nameofexecutable] to work */
	Tcl_FindExecutable (argv[0]);

	if (Tcl_Init(interp) == TCL_ERROR) {
		fprintf(stderr,"Tcl_Init failed: %s\n",interp->result);
		exit(1);
	}
	if (Expect_Init(interp) == TCL_ERROR) {
		fprintf(stderr,"Expect_Init failed: %s\n",interp->result);
		exit(1);
	}

#ifdef USE_ITCL
       if (Itcl_Init(interp) == TCL_ERROR) {
               fprintf(stderr,"Itcl_Init failed: %s\n",interp->result);
               exit(1);
       }
#endif /* USE_ITCL */
	exp_parse_argv(interp,argc,argv);

	/* become interactive if requested or "nothing to do" */
	if (exp_interactive)
		(void) exp_interpreter(interp);
	else if (exp_cmdfile)
		rc = exp_interpret_cmdfile(interp,exp_cmdfile);
	else if (exp_cmdfilename)
		rc = exp_interpret_cmdfilename(interp,exp_cmdfilename);

	/* assert(exp_cmdlinecmds != 0) */

	exp_exit(interp,rc);
	/*NOTREACHED*/
	return 0;		/* Needed only to prevent compiler warning. */
}

/* EOF */

