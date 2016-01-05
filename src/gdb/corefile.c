/* Core dump and executable file functions above target vector, for GDB.

   Copyright 1986, 1987, 1989, 1991, 1992, 1993, 1994, 1996, 1997,
   1998, 1999, 2000, 2001, 2003 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "defs.h"
#include "gdb_string.h"
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include "inferior.h"
#include "symtab.h"
#include "command.h"
#include "gdbcmd.h"
#include "bfd.h"
#include "target.h"
#include "gdbcore.h"
#include "dis-asm.h"
#include "gdb_stat.h"
#include "completer.h"
/* APPLE LOCAL symfile.h */
#include "symfile.h"
/* APPLE LOCAL objfiles.h */
#include "objfiles.h"
/* APPLE LOCAL: For objfile_changed function: */
#include "objc-lang.h"
#include "exceptions.h"


#ifndef __4_12_OR_LATER__
/* FIXME: these two defines are strings; we should have a numerical
 * equivalent: */
# if defined(PACKAGE_VERSION) && defined(VERSION)
#  define __4_12_OR_LATER__ 1
# endif /* PACKAGE_VERSION && VERSION */
#endif /* !__4_12_OR_LATER__ */
#ifndef IT_IS_TESTED
# define IT_IS_TESTED 1
#endif /* !IT_IS_TESTED */
#if 0
# ifndef FIXMES_ARE_FIXED
#  define FIXMES_ARE_FIXED 1
# endif /* !FIXMES_ARE_FIXED */
#endif /* 0 */

/* Local function declarations: */

extern void _initialize_core(void);
static void call_extra_exec_file_hooks(char *filename);

/* You can have any number of hooks for `exec_file_command' command to
   call.  If there is only one hook, then it is set in exec_file_display
   hook.  If there are two or more hooks, they are set in
   exec_file_extra_hooks[], and deprecated_exec_file_display_hook is
   set to a function that calls all of them.  This extra complexity is
   needed to preserve compatibility with old code that assumed that
   only one hook could be set, and which called
   deprecated_exec_file_display_hook directly.  */

typedef void (*hook_type) (char *);

hook_type deprecated_exec_file_display_hook;	/* the original hook */
static hook_type *exec_file_extra_hooks;   /* array of additional hooks */
static int exec_file_hook_count = 0;	/* size of array */

/* Binary file diddling handle for the core file.  */

bfd *core_bfd = NULL;


/* Backward compatability with old way of specifying core files.  */

void
/* APPLE LOCAL arguments to corefile */
core_file_command(const char *args, int from_tty)
{
  /* APPLE LOCAL begin refactor corefile */
  struct cleanup *cleanups;
  char *filename;
  char **argv;

  dont_repeat();

  if (args == NULL)
    {
      cleanups = NULL;
      filename = NULL;
    }
  else
    {
      argv = buildargv (args);
      if (argv == NULL)
	nomem (0);
      cleanups = make_cleanup_freeargv (argv);

      if (argv[0] == NULL)
	internal_error (__FILE__, __LINE__, "buildargv returned empty vector");
      else if (argv[1] != NULL)
	error ("more than one core filename was specified");
      else
	filename = argv[0];
    }

  core_file_attach (filename, from_tty);

  if (cleanups != NULL)
    do_cleanups (cleanups);
}

void
core_file_attach (char *filename, int from_tty)
{
  /* APPLE LOCAL end refactor corefile */
  struct target_ops *t;

  t = find_core_target ();
  if (t == NULL)
    error (_("GDB can't read core files on this machine."));

  if (!filename)
    (t->to_detach) (filename, from_tty);
  else
    (t->to_open) (filename, from_tty);
}


/* If there are two or more functions that wish to hook into
   exec_file_command, this function will call all of the hook
   functions.  */

static void
call_extra_exec_file_hooks (char *filename)
{
  int i;

  for (i = 0; i < exec_file_hook_count; i++)
    (*exec_file_extra_hooks[i]) (filename);
}

/* Call this to specify the hook for exec_file_command to call back.
   This is called from the x-window display code.  */

void
specify_exec_file_hook (void (*hook) (char *))
{
  hook_type *new_array;

  if (deprecated_exec_file_display_hook != NULL)
    {
      /* There's already a hook installed.  Arrange to have both it
       * and the subsequent hooks called. */
      if (exec_file_hook_count == 0)
	{
	  /* If this is the first extra hook, initialize the hook array.  */
	  exec_file_extra_hooks = (hook_type *) xmalloc (sizeof (hook_type));
	  exec_file_extra_hooks[0] = deprecated_exec_file_display_hook;
	  deprecated_exec_file_display_hook = call_extra_exec_file_hooks;
	  exec_file_hook_count = 1;
	}

      /* Grow the hook array by one and add the new hook to the end.
         Yes, it's inefficient to grow it by one each time but since
         this is hardly ever called it's not a big deal.  */
      exec_file_hook_count++;
      new_array =
	(hook_type *) xrealloc (exec_file_extra_hooks,
				exec_file_hook_count * sizeof (hook_type));
      exec_file_extra_hooks = new_array;
      exec_file_extra_hooks[exec_file_hook_count - 1] = hook;
    }
  else
    deprecated_exec_file_display_hook = hook;
}

/* The exec file must be closed before running an inferior.
   If it is needed again after the inferior dies, it must
   be reopened.  */

void
close_exec_file(void)
{
#ifdef HAVE_BFD_TEMPCLOSE				/* FIXME */
  if (exec_bfd)
    bfd_tempclose(exec_bfd);
#else
# ifdef __GNUC__
  asm("");
# endif /* __GNUC__ */
#endif /* HAVE_BFD_TEMPCLOSE */
}

void
reopen_exec_file(void)
{
#if 0				/* FIXME */
  if (exec_bfd)
    bfd_reopen(exec_bfd);
#else
  char *filename;
  int res;
  struct stat st;
  long mtime;

  /* APPLE LOCAL begin gdb_quitting */
  /* Do NOT do any of this if we are quitting.  */
  if (gdb_quitting)
    return;
  /* APPLE LOCAL end gdb_quitting */

  /* Do NOT do anything if the current target is NOT exec. */
  if (exec_bfd == NULL || (strcmp(target_shortname, "exec") != 0))
    return;

  /* If the timestamp of the exec file has changed, reopen it. */
  /* APPLE LOCAL comment */
  /* The whole world may have changed, so just unset all breakpoints: */
  filename = xstrdup(bfd_get_filename(exec_bfd));
  make_cleanup(xfree, filename);
  mtime = bfd_get_mtime(exec_bfd);
  res = stat(filename, &st);

  if (res == -1) {
    warning("stat failed with errno %d (i.e. \"%s\").\n", errno,
            safe_strerror(errno));
  }

  if (mtime && (mtime != st.st_mtime))
    {
      exec_open(filename, 0);
      /* APPLE LOCAL begin hooks */
      tell_breakpoints_objfile_changed(NULL);
      /* APPLE LOOCAL cache lookup values for improved performance  */
      symtab_clear_cached_lookup_values();
      tell_objc_msgsend_cacher_objfile_changed(NULL);
      /* APPLE LOCAL end hooks */
    }
#endif /* 0 */
}

/* If we have both a core file and an exec file,
   print a warning if they don't go together.  */

void
validate_files(void)
{
  if (exec_bfd && core_bfd)
    {
      if (!core_file_matches_executable_p(core_bfd, exec_bfd))
	warning(_("core file may not match specified executable file."));
      else if (bfd_get_mtime(exec_bfd) > bfd_get_mtime(core_bfd))
	warning(_("exec file is newer than core file."));
    }
}

/* Return the name of the executable file as a string.
   ERR nonzero means get error if there is none specified;
   otherwise return 0 in that case.  */

char *
get_exec_file(int err)
{
  if (exec_bfd)
    return bfd_get_filename(exec_bfd);
  if (!err)
    return NULL;

  error(_("No executable file specified.\n\
Use the \"file\" or \"exec-file\" command."));
  return NULL;
}


/* Report a memory error with error(): */
void ATTR_NORETURN
memory_error (int status, CORE_ADDR memaddr)
{
  struct ui_file *tmp_stream = mem_fileopen();
  make_cleanup_ui_file_delete(tmp_stream);

  if (status == EIO)
    {
      /* Actually, address between memaddr and memaddr + len
         was out of bounds. */
      fprintf_unfiltered (tmp_stream, "Cannot access memory at address 0x%s",
                          phex_nz (memaddr, sizeof (CORE_ADDR)));
    }
  else
    {
      fprintf_filtered (tmp_stream, "Error accessing memory address 0x%s",
                          phex_nz (memaddr, sizeof (CORE_ADDR)));
      fprintf_filtered (tmp_stream, ": %s.",
		       safe_strerror (status));
    }

  error_stream (tmp_stream);
}

/* Same as target_read_memory, but report an error if can't read.  */
void
read_memory (CORE_ADDR memaddr, gdb_byte *myaddr, int len)
{
  int status;
  status = target_read_memory (memaddr, myaddr, len);
  if (status != 0)
    memory_error (status, memaddr);
}

/* Argument / return result struct for use with
   do_captured_read_memory_integer().  MEMADDR and LEN are filled in
   by gdb_read_memory_integer().  RESULT is the contents that were
   successfully read from MEMADDR of length LEN.  */

struct captured_read_memory_integer_arguments
{
  CORE_ADDR memaddr;
  int len;
  /* APPLE LOCAL begin unsigned */
  int signedp;
  union {
    LONGEST sresult;
    ULONGEST uresult;
  } result;
  /* APPLE LOCAL end unsigned */
};

/* Helper function for gdb_read_memory_integer().  DATA must be a
   pointer to a captured_read_memory_integer_arguments struct.
   Return 1 if successful.  Note that the catch_errors() interface
   will return 0 if an error occurred while reading memory.  This
   choice of return code is so that we can distinguish between
   success and failure.  */

static int
do_captured_read_memory_integer (void *data)
{
  struct captured_read_memory_integer_arguments *args;
  CORE_ADDR memaddr;
  int len;

  args = (struct captured_read_memory_integer_arguments*)data;
  memaddr = args->memaddr;
  len = args->len;

  /* APPLE LOCAL begin unsigned */
  if (args->signedp)
    args->result.sresult = read_memory_integer(memaddr, len);
  else
    args->result.uresult = read_memory_unsigned_integer(memaddr, len);
  /* APPLE LOCAL end unsigned */

  return 1;
}

/* Read memory at MEMADDR of length LEN and put the contents in
   RETURN_VALUE.  Return 0 if MEMADDR couldn't be read and non-zero
   if successful.  */

int
safe_read_memory_integer(CORE_ADDR memaddr, int len, LONGEST *return_value)
{
  int status;
  struct captured_read_memory_integer_arguments args;
  args.memaddr = memaddr;
  args.len = len;
  args.signedp = 1;

  status = catch_errors(do_captured_read_memory_integer, &args,
                        "", RETURN_MASK_ALL);
  if (status)
    *return_value = args.result.sresult;

  return status;
}

/* APPLE LOCAL begin unsigned */
int
safe_read_memory_unsigned_integer(CORE_ADDR memaddr, int len,
                                  ULONGEST *return_value)
{
  int status;
  struct captured_read_memory_integer_arguments args;
  args.memaddr = memaddr;
  args.len = len;
  args.signedp = 0;

  status = catch_errors(do_captured_read_memory_integer, &args,
                        "", RETURN_MASK_ALL);
  if (status)
    *return_value = args.result.uresult;

  return status;
}
/* APPLE LOCAL end unsigned */

LONGEST
read_memory_integer(CORE_ADDR memaddr, int len)
{
  char buf[sizeof(LONGEST)];

  read_memory(memaddr, (gdb_byte *)buf, len);
  return extract_signed_integer((const gdb_byte *)buf, len);
}

ULONGEST
read_memory_unsigned_integer(CORE_ADDR memaddr, int len)
{
  char buf[sizeof(ULONGEST)];

  read_memory(memaddr, (gdb_byte *)buf, len);
  return extract_unsigned_integer((const gdb_byte *)buf, len);
}

void
read_memory_string(CORE_ADDR memaddr, char *buffer, int max_len)
{
  char *cp;
  int i;
  int cnt;

  cp = buffer;
  while (1)
    {
      if ((cp - buffer) >= max_len)
	{
	  buffer[max_len - 1] = '\0';
	  break;
	}
      cnt = (max_len - (cp - buffer));
      if (cnt > 8)
	cnt = 8;
      read_memory(memaddr + (int)(cp - buffer), (gdb_byte *)cp, cnt);
      for (i = 0; (i < cnt) && *cp; i++, cp++)
	;			/* null body */

      if ((i < cnt) && !*cp)
	break;
    }
}

CORE_ADDR
read_memory_typed_address(CORE_ADDR addr, struct type *type)
{
  char *buf = (char *)alloca(TYPE_LENGTH(type));
  read_memory(addr, (gdb_byte *)buf, TYPE_LENGTH(type));
  return extract_typed_address((const gdb_byte *)buf, type);
}

/* Same as target_write_memory, but report an error if cannot write: */
void
write_memory(CORE_ADDR memaddr, const bfd_byte *myaddr, int len)
{
  int status;
  bfd_byte *bytes = (bfd_byte *)alloca(len);

  memcpy(bytes, myaddr, len);
  status = target_write_memory(memaddr, bytes, len);
  if (status != 0)
    memory_error(status, memaddr);
}

/* Store VALUE at ADDR in the inferior as a LEN-byte unsigned integer: */
void
write_memory_unsigned_integer(CORE_ADDR addr, int len, ULONGEST value)
{
  char *buf = (char *)alloca(len);
  store_unsigned_integer((gdb_byte *)buf, len, value);
  write_memory(addr, (const bfd_byte *)buf, len);
}

/* Store VALUE at ADDR in the inferior as a LEN-byte signed integer: */
void
write_memory_signed_integer(CORE_ADDR addr, int len, LONGEST value)
{
  char *buf = (char *)alloca(len);
  store_signed_integer((gdb_byte *)buf, len, value);
  write_memory(addr, (const bfd_byte *)buf, len);
}


#if defined(__4_12_OR_LATER__) && defined(IT_IS_TESTED)
/* Enable after 4.12.  It is not tested.  */

/* Search code.  Targets can just make this their search function, or
   if the protocol has a less general search function, they can call this
   in the cases it can't handle.  */
void
generic_search(int len, char *data, char *mask, CORE_ADDR startaddr,
               int increment, CORE_ADDR lorange, CORE_ADDR hirange,
               CORE_ADDR *addr_found, char *data_found)
{
  int i;
  CORE_ADDR curaddr = startaddr;

  while ((curaddr >= lorange) && (curaddr < hirange))
    {
      read_memory(curaddr, (gdb_byte *)data_found, len);
      for (i = 0; i < len; ++i)
	if ((data_found[i] & mask[i]) != data[i])
	  goto try_again;
      /* It matches: */
      *addr_found = curaddr;
      return;

    try_again:
      curaddr += increment;
    }
  *addr_found = (CORE_ADDR)0UL;
  return;
}
#endif /* __4_12_OR_LATER__ && IT_IS_TESTED */

/* The current default bfd target.  Points to storage allocated for
   gnutarget_string.  */
char *gnutarget;

/* Same thing, except it is "auto" not NULL for the default case: */
static char *gnutarget_string;
static void
show_gnutarget_string(struct ui_file *file, int from_tty,
		      struct cmd_list_element *c, const char *value)
{
  fprintf_filtered(file, _("The current BFD target is \"%s\".\n"), value);
}

static void set_gnutarget_command(const char *, int, struct cmd_list_element *);

/* */
static void
set_gnutarget_command(const char *ignore ATTRIBUTE_UNUSED, int from_tty,
		      struct cmd_list_element *c)
{
  if (strcmp(gnutarget_string, "auto") == 0)
    gnutarget = NULL;
  else
    gnutarget = gnutarget_string;
}

/* Set the gnutarget: */
void
set_gnutarget(const char *newtarget)
{
  if (gnutarget_string != NULL)
    xfree(gnutarget_string);
  gnutarget_string = savestring(newtarget, strlen(newtarget));
  set_gnutarget_command(NULL, 0, NULL);
}

void
_initialize_core(void)
{
  struct cmd_list_element *c;
  c = add_cmd("core-file", class_files, core_file_command, _("\
Use FILE as core dump for examining memory and registers.\n\
No arg means have no core file.  This command has been superseded by the\n\
`target core' and `detach' commands."), &cmdlist);
  set_cmd_completer(c, filename_completer);
#ifdef FIXMES_ARE_FIXED
  c->completer_word_break_characters = gdb_completer_filename_word_break_characters; /* FIXME */
#endif /* FIXMES_ARE_FIXED */

  add_setshow_string_noescape_cmd("gnutarget", class_files,
				  &gnutarget_string, _("\
Set the current BFD target."), _("\
Show the current BFD target."), _("\
Use `set gnutarget auto' to specify automatic detection."),
				  set_gnutarget_command,
				  show_gnutarget_string,
				  &setlist, &showlist);

  if (getenv("GNUTARGET"))
    set_gnutarget(getenv("GNUTARGET"));
  else
    set_gnutarget("auto");
}

/* EOF */
