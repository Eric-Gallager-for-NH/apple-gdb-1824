/*
 * exp_regexp.h
 */

/* access to regexp internals */
#define regbol		exp_regbol
#define regtry		exp_regtry
#define regexec		exp_regexec
#define regerror	TclRegError
extern char *regbol;
int regtry();

/* EOF */
