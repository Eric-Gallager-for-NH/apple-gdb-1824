/* <proc_service.h> replacement for systems that do NOT have it.
   Copyright (C) 2000-2013 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef GDB_PROC_SERVICE_H
#define GDB_PROC_SERVICE_H

#include <sys/types.h>

#ifdef HAVE_PROC_SERVICE_H
# include <proc_service.h>
#else

#ifdef HAVE_SYS_PROCFS_H
# include <sys/procfs.h>
#else
# ifdef __GNUC__
#  warning gdb_proc_service.h expects <sys/procfs.h> to be included.
# endif /* __GNUC__ */
#endif /* HAVE_SYS_PROCFS_H */

/* Not all platforms bring in <linux/elf.h> via <sys/procfs.h>. If
   <sys/procfs.h> was NOT enough to find elf_fpregset_t, try the kernel
   headers also (but do NOT do so if we do NOT need to).  */
#ifndef HAVE_ELF_FPREGSET_T
# ifdef HAVE_LINUX_ELF_H
#  include <linux/elf.h>
# endif /* HAVE_LINUX_ELF_H */
#endif /* !HAVE_ELF_FPREGSET_T */

typedef enum
{
  PS_OK,			/* Success.  */
  PS_ERR,			/* Generic error.  */
  PS_BADPID,			/* Bad process handle.  */
  PS_BADLID,			/* Bad LWP id.  */
  PS_BADADDR,			/* Bad address.  */
  PS_NOSYM,			/* Symbol not found.  */
  PS_NOFREGS			/* FPU register set not available.  */
} ps_err_e;

#ifndef HAVE_LWPID_T
typedef unsigned int lwpid_t;
#endif /* !HAVE_LWPID_T */

#ifndef HAVE_PSADDR_T
typedef void *psaddr_t;
#endif /* !HAVE_PSADDR_T */

#ifndef HAVE_PRGREGSET_T
typedef elf_gregset_t prgregset_t;
#endif /* !HAVE_PRGREGSET_T */

#endif /* HAVE_PROC_SERVICE_H */

/* Structure that identifies the target process.  */
struct ps_prochandle
{
  /* We do NOT need to track anything. All context is served from the
     current inferior.  */
};

#endif /* gdb_proc_service.h */

/* EOF */
