/* readelf.c -- display contents of an ELF format file
   Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
   Free Software Foundation, Inc.

   Originally developed by Eric Youngdale <eric@andante.jic.com>
   Modifications by Nick Clifton <nickc@redhat.com>

   This file is part of GNU Binutils.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* The difference between readelf and objdump:

  Both programs are capable of displaying the contents of ELF format files,
  so why does the binutils project have two file dumpers ?

  The reason is that objdump sees an ELF file through a BFD filter of the
  world; if BFD has a bug where, say, it disagrees about a machine constant
  in e_flags, then the odds are good that it will remain internally
  consistent.  The linker sees it the BFD way, objdump sees it the BFD way,
  GAS sees it the BFD way.  There was need for a tool to go find out what
  the file actually says.

  This is why the readelf program does not link against the BFD library - it
  exists as an independent program to help verify the correct working of BFD.

  There is also the case that readelf can provide more information about an
  ELF file than is provided by objdump.  In particular it can display DWARF
  debugging information which (at the moment) objdump cannot.  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects config.h to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_CONFIG_H */

#if HAVE_LOCALE_H
# include <locale.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <locale.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_LOCALE_H */

#ifdef HAVE_ASSERT_H
# include <assert.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <assert.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_ASSERT_H */
#ifdef HAVE_LIBINTL_H
# include <libintl.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <libintl.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_LIBINTL_H */
#ifdef HAVE_LIMITS_H
# include <limits.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <limits.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_LIMITS_H */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <sys/types.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <sys/stat.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_STDIO_H
# include <stdio.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <stdio.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_STDIO_H */
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <stdlib.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_TIME_H
# include <time.h>
#else
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "readelf.c expects <time.h> to be included."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* HAVE_TIME_H */

#ifndef INFINITY
# if defined(HAVE_MATH_H) || (defined(STDC_HEADERS) && defined(__STDC__))
#  include <math.h>
# else
#  ifdef HUGE_VALF
#   define INFINITY HUGE_VALF
#  else
#   define INFINITY 1e50f
#  endif /* HUGE_VALF */
# endif /* HAVE_MATH_H || (STDC_HEADERS && __STDC__) */
#endif /* !INFINITY */

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif /* HAVE_STDINT_H */

#if defined(__GNUC__) && (__GNUC__ >= 2)
/* Define BFD64 here, even if our default architecture is 32 bit ELF
 * as this will allow us to read in and parse 64bit and 32bit ELF files.
 * Only do this if we believe that the compiler can support a 64 bit
 * data type. For now we only rely on GCC being able to do this.  */
# define BFD64
#endif /* __GNUC__ >= 2 */

#include "bfd.h"

#include "elf/common.h"
#include "elf/external.h"
#include "elf/internal.h"
#include "elf/dwarf2.h"

/* The following headers use the elf/reloc-macros.h file to
 * automatically generate relocation recognition functions
 * such as elf_mips_reloc_type()  */

#define RELOC_MACROS_GEN_FUNC

#include "elf/alpha.h"
#include "elf/arc.h"
#include "elf/arm.h"
#include "elf/avr.h"
#include "elf/cris.h"
#include "elf/d10v.h"
#include "elf/d30v.h"
#include "elf/dlx.h"
#include "elf/fr30.h"
#include "elf/frv.h"
#include "elf/h8.h"
#include "elf/hppa.h"
#include "elf/i386.h"
#include "elf/i370.h"
#include "elf/i860.h"
#include "elf/i960.h"
#include "elf/ia64.h"
#include "elf/ip2k.h"
#include "elf/m32c.h"
#include "elf/m32r.h"
#include "elf/m68k.h"
#include "elf/m68hc11.h"
#include "elf/mcore.h"
#include "elf/mips.h"
#include "elf/mmix.h"
#include "elf/mn10200.h"
#include "elf/mn10300.h"
#include "elf/ms1.h"
#include "elf/msp430.h"
#include "elf/or32.h"
#include "elf/pj.h"
#include "elf/ppc.h"
#include "elf/ppc64.h"
#include "elf/s390.h"
#include "elf/sh.h"
#include "elf/sparc.h"
#include "elf/v850.h"
#include "elf/vax.h"
#include "elf/x86-64.h"
#include "elf/xstormy16.h"
#include "elf/crx.h"
#include "elf/iq2000.h"
#include "elf/xtensa.h"

#include "aout/ar.h"

#include "bucomm.h"
#include "getopt.h"
#include "libiberty.h"

#include "sysdep.h"

const char *program_name = "readelf";
static long archive_file_offset;
static unsigned long archive_file_size;
static unsigned long dynamic_addr;
static bfd_size_type dynamic_size;
static unsigned int dynamic_nent;
static char *dynamic_strings;
static unsigned long dynamic_strings_length;
static char *string_table;
static unsigned long string_table_length;
static unsigned long num_dynamic_syms;
static Elf_Internal_Sym *dynamic_symbols;
static Elf_Internal_Syminfo *dynamic_syminfo;
static unsigned long dynamic_syminfo_offset;
static unsigned int dynamic_syminfo_nent;
static char program_interpreter[64];
static bfd_vma dynamic_info[DT_JMPREL + 1];
static bfd_vma version_info[16];
static Elf_Internal_Ehdr elf_header;
static Elf_Internal_Shdr *section_headers;
static Elf_Internal_Phdr *program_headers;
static Elf_Internal_Dyn *dynamic_section;
static Elf_Internal_Shdr *symtab_shndx_hdr;
static int show_name;
static int do_dynamic;
static int do_syms;
static int do_reloc;
static int do_sections;
static int do_section_groups;
static int do_section_details;
static int do_segments;
static int do_unwind;
static int do_using_dynamic;
static int do_header;
static int do_dump;
static int do_version;
static int do_wide;
static int do_histogram;
static int do_debugging;
static int do_debug_info;
static int do_debug_abbrevs;
static int do_debug_lines;
static int do_debug_pubnames;
static int do_debug_aranges;
static int do_debug_ranges;
static int do_debug_frames;
static int do_debug_frames_interp;
static int do_debug_macinfo;
static int do_debug_str;
static int do_debug_loc;
static int do_arch;
static int do_notes;
static int is_32bit_elf;
static int have_frame_base;
static int need_base_address;
static bfd_vma eh_addr_size;

struct group_list
{
  struct group_list *next;
  unsigned int section_index;
};

struct group
{
  struct group_list *root;
  unsigned int group_index;
};

static size_t group_count;
static struct group *section_groups;
static struct group **section_headers_groups;

/* A dynamic array of flags indicating for which sections a hex dump
 * has been requested (via the -x switch) and/or a disassembly dump
 * (via the -i switch).  */
#ifdef __clang__
extern char *cmdline_dump_sects;
extern unsigned num_cmdline_dump_sects;
#endif /* __clang__ */
char *cmdline_dump_sects = NULL;
unsigned num_cmdline_dump_sects = 0;

/* A dynamic array of flags indicating for which sections a dump of some
 * kind has been requested. It is reset on a per-object file basis
 * and then initialised from the cmdline_dump_sects array and the results
 * of interpreting the -w switch: */
#ifdef __clang__
extern char *dump_sects;
extern unsigned int num_dump_sects;
#endif /* __clang__ */
char *dump_sects = NULL;
unsigned int num_dump_sects = 0;

#define HEX_DUMP	(1 << 0)
#define DISASS_DUMP	(1 << 1)
#define DEBUG_DUMP	(1 << 2)

/* How to print a vma value: */
typedef enum print_mode
{
  HEX,
  DEC,
  DEC_5,
  UNSIGNED,
  PREFIX_HEX,
  FULL_HEX,
  LONG_HEX
}
print_mode;

static bfd_vma (*byte_get) (unsigned char *, int);
static void (*byte_put) (unsigned char *, bfd_vma, int);

#define UNKNOWN -1

#define SECTION_NAME(X)	((X) == NULL ? "<none>" : \
			 ((X)->sh_name >= string_table_length \
			  ? "<corrupt>" : string_table + (X)->sh_name))

/* Given st_shndx I, map to section_headers index.  */
#define SECTION_HEADER_INDEX(I)				\
  ((I) < SHN_LORESERVE					\
   ? (I)						\
   : ((I) <= SHN_HIRESERVE				\
      ? 0						\
      : (I) - (SHN_HIRESERVE + 1 - SHN_LORESERVE)))

/* Reverse of the above.  */
#define SECTION_HEADER_NUM(N)				\
  ((N) < SHN_LORESERVE					\
   ? (N)						\
   : (N) + (SHN_HIRESERVE + 1 - SHN_LORESERVE))

#define SECTION_HEADER(I) (section_headers + SECTION_HEADER_INDEX (I))

#define DT_VERSIONTAGIDX(tag)	(DT_VERNEEDNUM - (tag))	/* Reverse order!  */

#define BYTE_GET(field)	byte_get (field, sizeof (field))

#define NUM_ELEM(array) 	(sizeof (array) / sizeof ((array)[0]))

#define GET_ELF_SYMBOLS(file, section)			\
  (is_32bit_elf ? get_32bit_elf_symbols (file, section)	\
   : get_64bit_elf_symbols (file, section))

#define VALID_DYNAMIC_NAME(offset)	((dynamic_strings != NULL) && (offset < dynamic_strings_length))
/* GET_DYNAMIC_NAME asssumes that VALID_DYNAMIC_NAME has
   already been called and verified that the string exists.  */
#define GET_DYNAMIC_NAME(offset)	(dynamic_strings + offset)

/* This is just a bit of syntatic sugar.  */
#define streq(a,b)	(strcmp ((a), (b)) == 0)
#define strneq(a,b,n)	(strncmp ((a), (b), (n)) == 0)

static void ATTRIBUTE_PRINTF_1
error (const char *message, ...)
{
  va_list args;

  va_start (args, message);
  fprintf (stderr, _("%s: Error: "), program_name);
  vfprintf (stderr, message, args);
  va_end (args);
}

static void ATTRIBUTE_PRINTF_1
warn (const char *message, ...)
{
  va_list args;

  va_start (args, message);
  fprintf (stderr, _("%s: Warning: "), program_name);
  vfprintf (stderr, message, args);
  va_end (args);
}

static void *
cmalloc (size_t nmemb, size_t size)
{
  /* Check for overflow.  */
  if (nmemb >= ~(size_t) 0 / size)
    return NULL;
  else
    return malloc (nmemb * size);
}

static void *
xcmalloc (size_t nmemb, size_t size)
{
  /* Check for overflow.  */
  if (nmemb >= ~(size_t) 0 / size)
    return NULL;
  else
    return xmalloc (nmemb * size);
}

static void *
xcrealloc (void *ptr, size_t nmemb, size_t size)
{
  /* Check for overflow.  */
  if (nmemb >= ~(size_t) 0 / size)
    return NULL;
  else
    return xrealloc (ptr, nmemb * size);
}

static void *
get_data (void *var, FILE *file, long offset, size_t size, size_t nmemb,
	  const char *reason)
{
  void *mvar;

  if (size == 0 || nmemb == 0)
    return NULL;

  if (fseek (file, archive_file_offset + offset, SEEK_SET))
    {
      error (_("Unable to seek to 0x%lx for %s\n"),
	     archive_file_offset + offset, reason);
      return NULL;
    }

  mvar = var;
  if (mvar == NULL)
    {
      /* Check for overflow.  */
      if (nmemb < (~(size_t) 0 - 1) / size)
	/* + 1 so that we can '\0' terminate invalid string table sections.  */
	mvar = malloc (size * nmemb + 1);

      if (mvar == NULL)
	{
	  error (_("Out of memory allocating 0x%lx bytes for %s\n"),
		 (unsigned long)(size * nmemb), reason);
	  return NULL;
	}

      ((char *) mvar)[size * nmemb] = '\0';
    }

  if (fread (mvar, size, nmemb, file) != nmemb)
    {
      error (_("Unable to read in 0x%lx bytes of %s\n"),
	     (unsigned long)(size * nmemb), reason);
      if (mvar != var)
	free (mvar);
      return NULL;
    }

  return mvar;
}

static bfd_vma
byte_get_little_endian (unsigned char *field, int size)
{
  switch (size)
    {
    case 1:
      return *field;

    case 2:
      return (((unsigned int)(field[0]))
              | (((unsigned int)(field[1])) << 8));

#ifndef BFD64
    case 8:
      /* We want to extract data from an 8 byte wide field and place it
       * into a 4 byte wide field. Since this is a little endian source,
       * we can just use the 4 byte extraction code.  */
      /* Fall through.  */
#endif /* !BFD64 */
    case 4:
      return (((unsigned long) (field[0]))
              | (((unsigned long) (field[1])) << 8)
              | (((unsigned long) (field[2])) << 16)
              | (((unsigned long) (field[3])) << 24));

#ifdef BFD64
    case 8:
      return (((bfd_vma) (field[0]))
              | (((bfd_vma) (field[1])) << 8)
              | (((bfd_vma) (field[2])) << 16)
              | (((bfd_vma) (field[3])) << 24)
              | (((bfd_vma) (field[4])) << 32)
              | (((bfd_vma) (field[5])) << 40)
              | (((bfd_vma) (field[6])) << 48)
              | (((bfd_vma) (field[7])) << 56));
#endif /* BFD64 */
    default:
      error(_("Unhandled data length: %d\n"), size);
      abort();
    }
}

static bfd_vma
byte_get_signed(unsigned char *field, int size)
{
  bfd_vma x = byte_get(field, size);

  switch (size)
    {
    case 1: return ((x ^ 0x80) - 0x80);
    case 2: return ((x ^ 0x8000) - 0x8000);
    case 4: return ((x ^ 0x80000000) - 0x80000000);
    case 8: return x;
    default: abort();
    }
}

static void
byte_put_little_endian(unsigned char *field, bfd_vma value, int size)
{
  switch (size)
    {
    case 8:
      field[7] = (unsigned char)((((value >> 24) >> 24) >> 8) & 0xff);
      field[6] = (((value >> 24) >> 24) & 0xff);
      field[5] = (((value >> 24) >> 16) & 0xff);
      field[4] = (((value >> 24) >> 8) & 0xff);
      /* Fall through.  */
      ATTRIBUTE_FALLTHROUGH;
    case 4:
      field[3] = ((value >> 24) & 0xff);
      field[2] = ((value >> 16) & 0xff);
      /* Fall through.  */
      ATTRIBUTE_FALLTHROUGH;
    case 2:
      field[1] = ((value >> 8) & 0xff);
      /* Fall through.  */
      ATTRIBUTE_FALLTHROUGH;
    case 1:
      field[0] = (value & 0xff);
      break;

    default:
      error(_("Unhandled data length: %d\n"), size);
      abort();
    }
}

#if defined BFD64 && !BFD_HOST_64BIT_LONG
static int
print_dec_vma(bfd_vma vma, int is_signed)
{
  char buf[40];
  char *bufp = buf;
  int nc = 0;

  if (is_signed && ((bfd_signed_vma)vma < 0L))
    {
      vma = -vma;
      putchar('-');
      nc = 1;
    }

  do {
    *bufp++ = (char)('0' + (char)(vma % 10UL));
    vma /= 10;
  } while (vma != 0);
  nc += (bufp - buf);

  while (bufp > buf)
    putchar(*--bufp);
  return nc;
}

static int
print_hex_vma(bfd_vma vma)
{
  char buf[32];
  char *bufp = buf;
  int nc;

  do {
    char digit = (char)('0' + (char)(vma & 0x0f));
    if (digit > '9')
      digit = (char)(digit + (char)('a' - '0' - (char)10));
    *bufp++ = digit;
    vma >>= 4;
  } while (vma != 0);
  nc = (bufp - buf);

  while (bufp > buf)
    putchar(*--bufp);
  return nc;
}
#endif /* BFD64 && !BFD_HOST_64BIT_LONG */

/* Print a VMA value: */
static int
print_vma(bfd_vma vma, print_mode mode)
{
#ifdef BFD64
  if (is_32bit_elf)
#endif /* BFD64 */
    {
      switch (mode)
	{
	case FULL_HEX: return printf("0x%8.8lx", (unsigned long)vma);
	case LONG_HEX: return printf("%8.8lx", (unsigned long)vma);
	case DEC_5:
	  if (vma <= 99999)
	    return printf("%5ld", (long)vma);
	  /* Drop through: */
	  ATTRIBUTE_FALLTHROUGH;
	case PREFIX_HEX: return printf("0x%lx", (unsigned long)vma);
	case HEX: return printf("%lx", (unsigned long)vma);
	case DEC: return printf("%ld", (unsigned long)vma);
	case UNSIGNED: return printf("%lu", (unsigned long)vma);
        default:;
	}
    }
#ifdef BFD64
  else
    {
      int nc = 0;

      switch (mode)
	{
	case FULL_HEX:
	  nc = printf("0x");
	  /* Drop through: */
	  ATTRIBUTE_FALLTHROUGH;
	case LONG_HEX:
	  printf_vma(vma);
	  return (nc + 16);

	case PREFIX_HEX:
	  nc = printf("0x");
	  /* Drop through: */
	  ATTRIBUTE_FALLTHROUGH;
	case HEX:
# if BFD_HOST_64BIT_LONG
	  return (nc + printf("%lx", vma));
# else
	  return (nc + print_hex_vma(vma));
# endif /* BFD_HOST_64BIT_LONG */

	case DEC:
# if BFD_HOST_64BIT_LONG
	  return printf("%ld", vma);
# else
	  return print_dec_vma(vma, 1);
# endif /* BFD_HOST_64BIT_LONG */

	case DEC_5:
# if BFD_HOST_64BIT_LONG
	  if (vma <= 99999)
	    return printf("%5ld", vma);
	  else
	    return printf("%#lx", vma);
# else
	  if (vma <= 99999)
	    return printf("%5ld", _bfd_int64_low(vma));
	  else
	    return print_hex_vma(vma);
# endif /* BFD_HOST_64BIT_LONG */

	case UNSIGNED:
# if BFD_HOST_64BIT_LONG
	  return printf("%lu", vma);
# else
	  return print_dec_vma(vma, 0);
# endif /* BFD_HOST_64BIT_LONG */

        default:;
	}
    }
#endif /* BFD64 */
  return 0;
}

/* Display a symbol on stdout. If do_wide is not true then
   format the symbol to be at most WIDTH characters,
   truncating as necessary. If WIDTH is negative then
   format the string to be exactly - WIDTH characters,
   truncating or padding as necessary.  */

static void
print_symbol (int width, const char *symbol)
{
  if (do_wide)
    printf ("%s", symbol);
  else if (width < 0)
    printf ("%-*.*s", width, width, symbol);
  else
    printf ("%-.*s", width, symbol);
}

static bfd_vma
byte_get_big_endian(unsigned char *field, int size)
{
  switch (size)
    {
    case 1:
      return *field;

    case 2:
      return (bfd_vma)(((unsigned int)(field[1]))
                       | (unsigned int)(((int)(field[0])) << 8));

#ifndef BFD64
    case 8:
      /* Although we are extracing data from an 8 byte wide field,
	 we are returning only 4 bytes of data.  */
      field += 4;
      /* Fall thru */
#endif /* !BFD64 */
    case 4:
      return ((unsigned long) (field[3]))
	|   (((unsigned long) (field[2])) << 8)
	|   (((unsigned long) (field[1])) << 16)
	|   (((unsigned long) (field[0])) << 24);

#ifdef BFD64
    case 8:
      return ((bfd_vma) (field[7]))
	|   (((bfd_vma) (field[6])) << 8)
	|   (((bfd_vma) (field[5])) << 16)
	|   (((bfd_vma) (field[4])) << 24)
	|   (((bfd_vma) (field[3])) << 32)
	|   (((bfd_vma) (field[2])) << 40)
	|   (((bfd_vma) (field[1])) << 48)
	|   (((bfd_vma) (field[0])) << 56);
#endif /* BFD64 */

    default:
      error (_("Unhandled data length: %d\n"), size);
      abort ();
    }
}

static void
byte_put_big_endian (unsigned char *field, bfd_vma value, int size)
{
  switch (size)
    {
    case 8:
      field[7] = value & 0xff;
      field[6] = (value >> 8) & 0xff;
      field[5] = (value >> 16) & 0xff;
      field[4] = (value >> 24) & 0xff;
      value >>= 16;
      value >>= 16;
      /* Fall through.  */
      ATTRIBUTE_FALLTHROUGH;
    case 4:
      field[3] = value & 0xff;
      field[2] = (value >> 8) & 0xff;
      value >>= 16;
      /* Fall through.  */
      ATTRIBUTE_FALLTHROUGH;
    case 2:
      field[1] = value & 0xff;
      value >>= 8;
      /* Fall through.  */
      ATTRIBUTE_FALLTHROUGH;
    case 1:
      field[0] = value & 0xff;
      break;

    default:
      error (_("Unhandled data length: %d\n"), size);
      abort ();
    }
}

/* Return a pointer to section NAME, or NULL if no such section exists.  */

static Elf_Internal_Shdr *
find_section (const char *name)
{
  unsigned int i;

  for (i = 0; i < elf_header.e_shnum; i++)
    if (streq (SECTION_NAME (section_headers + i), name))
      return section_headers + i;

  return NULL;
}

/* Guess the relocation size commonly used by the specific machines: */
static int
guess_is_rela(unsigned long e_machine)
{
  switch (e_machine)
    {
      /* Targets that use REL relocations.  */
    case EM_ARM:
    case EM_386:
    case EM_486:
    case EM_960:
    case EM_DLX:
    case EM_OPENRISC:
    case EM_OR32:
    case EM_CYGNUS_M32R:
    case EM_D10V:
    case EM_CYGNUS_D10V:
    case EM_MIPS:
    case EM_MIPS_RS3_LE:
      return FALSE;

      /* Targets that use RELA relocations.  */
    case EM_68K:
    case EM_H8_300:
    case EM_H8_300H:
    case EM_H8S:
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
    case EM_SPARC:
    case EM_PPC:
    case EM_PPC64:
    case EM_V850:
    case EM_CYGNUS_V850:
    case EM_D30V:
    case EM_CYGNUS_D30V:
    case EM_MN10200:
    case EM_CYGNUS_MN10200:
    case EM_MN10300:
    case EM_CYGNUS_MN10300:
    case EM_FR30:
    case EM_CYGNUS_FR30:
    case EM_CYGNUS_FRV:
    case EM_SH:
    case EM_ALPHA:
    case EM_MCORE:
    case EM_IA_64:
    case EM_AVR:
    case EM_AVR_OLD:
    case EM_CRIS:
    case EM_860:
    case EM_X86_64:
    case EM_S390:
    case EM_S390_OLD:
    case EM_MMIX:
    case EM_MSP430:
    case EM_MSP430_OLD:
    case EM_XSTORMY16:
    case EM_CRX:
    case EM_VAX:
    case EM_IP2K:
    case EM_IP2K_OLD:
    case EM_IQ2000:
    case EM_XTENSA:
    case EM_XTENSA_OLD:
    case EM_M32R:
    case EM_M32C:
    case EM_MS1:
      return TRUE;

    case EM_MMA:
    case EM_PCP:
    case EM_NCPU:
    case EM_NDR1:
    case EM_STARCORE:
    case EM_ME16:
    case EM_ST100:
    case EM_TINYJ:
    case EM_FX66:
    case EM_ST9PLUS:
    case EM_ST7:
    case EM_68HC16:
    case EM_68HC11:
    case EM_68HC08:
    case EM_68HC05:
    case EM_SVX:
    case EM_ST19:
    default:
      warn(_("Do not know about relocations on this machine architecture\n"));
      return FALSE;
    }
}

static int
slurp_rela_relocs(FILE *file, unsigned long rel_offset,
                  unsigned long rel_size, Elf_Internal_Rela **relasp,
                  unsigned long *nrelasp)
{
  Elf_Internal_Rela *relas;
  unsigned long nrelas;
  unsigned int i;

  if (is_32bit_elf)
    {
      Elf32_External_Rela *erelas;

      erelas = (Elf32_External_Rela *)get_data(NULL, file, rel_offset, 1,
                                               rel_size, _("relocs"));
      if (!erelas) {
	return 0;
      }

      nrelas = (rel_size / sizeof(Elf32_External_Rela));

      relas = (Elf_Internal_Rela *)cmalloc(nrelas,
                                           sizeof(Elf_Internal_Rela));

      if (relas == NULL)
	{
	  free(erelas);
	  error(_("out of memory parsing relocs"));
	  return 0;
	}

      for (i = 0; i < nrelas; i++)
	{
	  relas[i].r_offset = BYTE_GET(erelas[i].r_offset);
	  relas[i].r_info = BYTE_GET(erelas[i].r_info);
	  relas[i].r_addend = BYTE_GET(erelas[i].r_addend);
	}

      free(erelas);
    }
  else
    {
      Elf64_External_Rela *erelas;

      erelas = (Elf64_External_Rela *)get_data(NULL, file, rel_offset, 1,
                                               rel_size, _("relocs"));
      if (!erelas) {
	return 0;
      }

      nrelas = (rel_size / sizeof(Elf64_External_Rela));

      relas = (Elf_Internal_Rela *)cmalloc(nrelas, sizeof(Elf_Internal_Rela));

      if (relas == NULL)
	{
	  free(erelas);
	  error(_("out of memory parsing relocs"));
	  return 0;
	}

      for (i = 0; i < nrelas; i++)
	{
	  relas[i].r_offset = BYTE_GET(erelas[i].r_offset);
	  relas[i].r_info = BYTE_GET(erelas[i].r_info);
	  relas[i].r_addend = BYTE_GET(erelas[i].r_addend);
	}

      free(erelas);
    }
  *relasp = relas;
  *nrelasp = nrelas;
  return 1;
}

static int
slurp_rel_relocs(FILE *file, unsigned long rel_offset,
                 unsigned long rel_size, Elf_Internal_Rela **relsp,
                 unsigned long *nrelsp)
{
  Elf_Internal_Rela *rels;
  unsigned long nrels;
  unsigned int i;

  if (is_32bit_elf)
    {
      Elf32_External_Rel *erels;

      erels = (Elf32_External_Rel *)get_data(NULL, file, rel_offset, 1,
                                             rel_size, _("relocs"));
      if (!erels) {
	return 0;
      }

      nrels = (rel_size / sizeof(Elf32_External_Rel));

      rels = (Elf_Internal_Rela *)cmalloc(nrels, sizeof(Elf_Internal_Rela));

      if (rels == NULL)
	{
	  free(erels);
	  error(_("out of memory parsing relocs"));
	  return 0;
	}

      for (i = 0; i < nrels; i++)
	{
	  rels[i].r_offset = BYTE_GET(erels[i].r_offset);
	  rels[i].r_info = BYTE_GET(erels[i].r_info);
	  rels[i].r_addend = 0;
	}

      free(erels);
    }
  else
    {
      Elf64_External_Rel *erels;

      erels = (Elf64_External_Rel *)get_data(NULL, file, rel_offset, 1,
                                             rel_size, _("relocs"));
      if (!erels) {
	return 0;
      }

      nrels = (rel_size / sizeof(Elf64_External_Rel));

      rels = (Elf_Internal_Rela *)cmalloc(nrels,
                                          sizeof(Elf_Internal_Rela));

      if (rels == NULL)
	{
	  free(erels);
	  error(_("out of memory parsing relocs"));
	  return 0;
	}

      for (i = 0; i < nrels; i++)
	{
	  rels[i].r_offset = BYTE_GET(erels[i].r_offset);
	  rels[i].r_info = BYTE_GET(erels[i].r_info);
	  rels[i].r_addend = 0;
	}

      free(erels);
    }
  *relsp = rels;
  *nrelsp = nrels;
  return 1;
}

/* Display contents of relocation data found at the specified offset: */
static int
dump_relocations(FILE *file, unsigned long rel_offset,
                 unsigned long rel_size, Elf_Internal_Sym *symtab,
                 unsigned long nsyms, char *strtab,
                 unsigned long strtablen, int is_rela)
{
  unsigned int i;
  Elf_Internal_Rela *rels;

  if (is_rela == UNKNOWN) {
    is_rela = guess_is_rela(elf_header.e_machine);
  }

  if (is_rela)
    {
      if (!slurp_rela_relocs(file, rel_offset, rel_size, &rels, &rel_size))
	return 0;
    }
  else
    {
      if (!slurp_rel_relocs(file, rel_offset, rel_size, &rels, &rel_size))
	return 0;
    }

  if (is_32bit_elf)
    {
      if (is_rela)
	{
	  if (do_wide)
	    printf (_(" Offset     Info    Type                Sym. Value  Symbol's Name + Addend\n"));
	  else
	    printf (_(" Offset     Info    Type            Sym.Value  Sym. Name + Addend\n"));
	}
      else
	{
	  if (do_wide)
	    printf (_(" Offset     Info    Type                Sym. Value  Symbol's Name\n"));
	  else
	    printf (_(" Offset     Info    Type            Sym.Value  Sym. Name\n"));
	}
    }
  else
    {
      if (is_rela)
	{
	  if (do_wide)
	    printf (_("    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend\n"));
	  else
	    printf (_("  Offset          Info           Type           Sym. Value    Sym. Name + Addend\n"));
	}
      else
	{
	  if (do_wide)
	    printf (_("    Offset             Info             Type               Symbol's Value  Symbol's Name\n"));
	  else
	    printf (_("  Offset          Info           Type           Sym. Value    Sym. Name\n"));
	}
    }

  for (i = 0; i < rel_size; i++)
    {
      const char *rtype;
      const char *rtype2 = NULL;
      const char *rtype3 = NULL;
      bfd_vma offset;
      bfd_vma info;
      bfd_vma symtab_index;
      bfd_vma type;
      bfd_vma type2 = 0;
      bfd_vma type3 = 0;

      offset = rels[i].r_offset;
      info   = rels[i].r_info;

      if (is_32bit_elf)
	{
	  type         = ELF32_R_TYPE (info);
	  symtab_index = ELF32_R_SYM  (info);
	}
      else
	{
	  /* The #ifdef BFD64 below is to prevent a compile time warning.
	     We know that if we do not have a 64 bit data type that we
	     will never execute this code anyway.  */
#ifdef BFD64
	  if (elf_header.e_machine == EM_MIPS)
	    {
	      /* In little-endian objects, r_info isn't really a 64-bit
		 little-endian value: it has a 32-bit little-endian
		 symbol index followed by four individual byte fields.
		 Reorder INFO accordingly.  */
	      if (elf_header.e_ident[EI_DATA] != ELFDATA2MSB)
		info = (((info & 0xffffffff) << 32)
			| ((info >> 56) & 0xff)
			| ((info >> 40) & 0xff00)
			| ((info >> 24) & 0xff0000)
			| ((info >> 8) & 0xff000000));
	      type  = ELF64_MIPS_R_TYPE (info);
	      type2 = ELF64_MIPS_R_TYPE2 (info);
	      type3 = ELF64_MIPS_R_TYPE3 (info);
	    }
	  else if (elf_header.e_machine == EM_SPARCV9)
	    type = ELF64_R_TYPE_ID (info);
	  else
	    type = ELF64_R_TYPE (info);

	  symtab_index = ELF64_R_SYM  (info);
#endif /* BFD64 */
	}

      if (is_32bit_elf)
	{
#ifdef _bfd_int64_low
	  printf ("%8.8lx  %8.8lx ", _bfd_int64_low (offset), _bfd_int64_low (info));
#else
	  printf ("%8.8lx  %8.8lx ", offset, info);
#endif /* _bfd_int64_low */
	}
      else
	{
#ifdef _bfd_int64_low
	  printf (do_wide
		  ? "%8.8lx%8.8lx  %8.8lx%8.8lx "
		  : "%4.4lx%8.8lx  %4.4lx%8.8lx ",
		  _bfd_int64_high (offset),
		  _bfd_int64_low (offset),
		  _bfd_int64_high (info),
		  _bfd_int64_low (info));
#else
	  printf (do_wide
		  ? "%16.16lx  %16.16lx "
		  : "%12.12lx  %12.12lx ",
		  offset, info);
#endif /* _bfd_int64_low */
	}

      switch (elf_header.e_machine)
	{
	default:
	  rtype = NULL;
	  break;
	case EM_M32R:
	case EM_CYGNUS_M32R:
	  rtype = elf_m32r_reloc_type(type);
	  break;
	case EM_386:
	case EM_486:
	  rtype = elf_i386_reloc_type(type);
	  break;
	case EM_68HC11:
	case EM_68HC12:
	  rtype = elf_m68hc11_reloc_type(type);
	  break;
	case EM_68K:
	  rtype = elf_m68k_reloc_type(type);
	  break;
	case EM_960:
	  rtype = elf_i960_reloc_type(type);
	  break;
	case EM_AVR:
	case EM_AVR_OLD:
	  rtype = elf_avr_reloc_type(type);
	  break;
	case EM_OLD_SPARCV9:
	case EM_SPARC32PLUS:
	case EM_SPARCV9:
	case EM_SPARC:
	  rtype = elf_sparc_reloc_type (type);
	  break;
	case EM_V850:
	case EM_CYGNUS_V850:
	  rtype = v850_reloc_type(type);
	  break;
	case EM_D10V:
	case EM_CYGNUS_D10V:
	  rtype = elf_d10v_reloc_type(type);
	  break;
	case EM_D30V:
	case EM_CYGNUS_D30V:
	  rtype = elf_d30v_reloc_type(type);
	  break;
	case EM_DLX:
	  rtype = elf_dlx_reloc_type(type);
	  break;
	case EM_SH:
	  rtype = elf_sh_reloc_type(type);
	  break;
	case EM_MN10300:
	case EM_CYGNUS_MN10300:
	  rtype = elf_mn10300_reloc_type(type);
	  break;
	case EM_MN10200:
	case EM_CYGNUS_MN10200:
	  rtype = elf_mn10200_reloc_type(type);
	  break;
	case EM_FR30:
	case EM_CYGNUS_FR30:
	  rtype = elf_fr30_reloc_type(type);
	  break;
	case EM_CYGNUS_FRV:
	  rtype = elf_frv_reloc_type(type);
	  break;
	case EM_MCORE:
	  rtype = elf_mcore_reloc_type(type);
	  break;
	case EM_MMIX:
	  rtype = elf_mmix_reloc_type(type);
	  break;
	case EM_MSP430:
	case EM_MSP430_OLD:
	  rtype = elf_msp430_reloc_type(type);
	  break;
	case EM_PPC:
	  rtype = elf_ppc_reloc_type(type);
	  break;
	case EM_PPC64:
	  rtype = elf_ppc64_reloc_type(type);
	  break;
	case EM_MIPS:
	case EM_MIPS_RS3_LE:
	  rtype = elf_mips_reloc_type(type);
	  if (!is_32bit_elf)
	    {
	      rtype2 = elf_mips_reloc_type(type2);
	      rtype3 = elf_mips_reloc_type(type3);
	    }
	  break;
	case EM_ALPHA:
	  rtype = elf_alpha_reloc_type(type);
	  break;
	case EM_ARM:
	  rtype = elf_arm_reloc_type(type);
	  break;
	case EM_ARC:
	  rtype = elf_arc_reloc_type(type);
	  break;
	case EM_PARISC:
	  rtype = elf_hppa_reloc_type(type);
	  break;
	case EM_H8_300:
	case EM_H8_300H:
	case EM_H8S:
	  rtype = elf_h8_reloc_type(type);
	  break;
	case EM_OPENRISC:
	case EM_OR32:
	  rtype = elf_or32_reloc_type(type);
	  break;
	case EM_PJ:
	case EM_PJ_OLD:
	  rtype = elf_pj_reloc_type(type);
	  break;
	case EM_IA_64:
	  rtype = elf_ia64_reloc_type(type);
	  break;
	case EM_CRIS:
	  rtype = elf_cris_reloc_type(type);
	  break;
	case EM_860:
	  rtype = elf_i860_reloc_type(type);
	  break;
	case EM_X86_64:
	  rtype = elf_x86_64_reloc_type(type);
	  break;
	case EM_S370:
	  rtype = i370_reloc_type(type);
	  break;
	case EM_S390_OLD:
	case EM_S390:
	  rtype = elf_s390_reloc_type(type);
	  break;
	case EM_XSTORMY16:
	  rtype = elf_xstormy16_reloc_type(type);
	  break;
	case EM_CRX:
	  rtype = elf_crx_reloc_type(type);
	  break;
	case EM_VAX:
	  rtype = elf_vax_reloc_type(type);
	  break;
	case EM_IP2K:
	case EM_IP2K_OLD:
	  rtype = elf_ip2k_reloc_type(type);
	  break;
	case EM_IQ2000:
	  rtype = elf_iq2000_reloc_type(type);
	  break;
	case EM_XTENSA_OLD:
	case EM_XTENSA:
	  rtype = elf_xtensa_reloc_type(type);
	  break;
	case EM_M32C:
	  rtype = elf_m32c_reloc_type(type);
	  break;
	case EM_MS1:
	  rtype = elf_ms1_reloc_type(type);
	  break;
	}

      if (rtype == NULL)
#ifdef _bfd_int64_low
	printf (_("unrecognized: %-7lx"), _bfd_int64_low (type));
#else
	printf (_("unrecognized: %-7lx"), type);
#endif /* _bfd_int64_low */
      else
	printf (do_wide ? "%-22.22s" : "%-17.17s", rtype);

      if (elf_header.e_machine == EM_ALPHA
	  && streq (rtype, "R_ALPHA_LITUSE")
	  && is_rela)
	{
	  switch (rels[i].r_addend)
	    {
	    case LITUSE_ALPHA_ADDR:   rtype = "ADDR";   break;
	    case LITUSE_ALPHA_BASE:   rtype = "BASE";   break;
	    case LITUSE_ALPHA_BYTOFF: rtype = "BYTOFF"; break;
	    case LITUSE_ALPHA_JSR:    rtype = "JSR";    break;
	    case LITUSE_ALPHA_TLSGD:  rtype = "TLSGD";  break;
	    case LITUSE_ALPHA_TLSLDM: rtype = "TLSLDM"; break;
	    case LITUSE_ALPHA_JSRDIRECT: rtype = "JSRDIRECT"; break;
	    default: rtype = NULL;
	    }
	  if (rtype)
	    printf (" (%s)", rtype);
	  else
	    {
	      putchar (' ');
	      printf (_("<unknown addend: %lx>"),
		      (unsigned long) rels[i].r_addend);
	    }
	}
      else if (symtab_index)
	{
	  if (symtab == NULL || symtab_index >= nsyms)
	    printf (" bad symbol index: %08lx", (unsigned long) symtab_index);
	  else
	    {
	      Elf_Internal_Sym *psym;

	      psym = symtab + symtab_index;

	      printf (" ");
	      print_vma (psym->st_value, LONG_HEX);
	      printf (is_32bit_elf ? "   " : " ");

	      if (psym->st_name == 0)
		{
		  const char *sec_name = "<null>";
		  char name_buf[40];

		  if (ELF_ST_TYPE (psym->st_info) == STT_SECTION)
		    {
		      bfd_vma sec_index = (bfd_vma) -1;

		      if (psym->st_shndx < SHN_LORESERVE)
			sec_index = psym->st_shndx;
		      else if (psym->st_shndx > SHN_HIRESERVE)
			sec_index = psym->st_shndx - (SHN_HIRESERVE + 1
						      - SHN_LORESERVE);

		      if (sec_index != (bfd_vma) -1)
			sec_name = SECTION_NAME (section_headers + sec_index);
		      else if (psym->st_shndx == SHN_ABS)
			sec_name = "ABS";
		      else if (psym->st_shndx == SHN_COMMON)
			sec_name = "COMMON";
		      else if (elf_header.e_machine == EM_X86_64
			       && psym->st_shndx == SHN_X86_64_LCOMMON)
			sec_name = "LARGE_COMMON";
		      else if (elf_header.e_machine == EM_IA_64
			       && elf_header.e_ident[EI_OSABI] == ELFOSABI_HPUX
			       && psym->st_shndx == SHN_IA_64_ANSI_COMMON)
			sec_name = "ANSI_COM";
		      else
			{
			  sprintf (name_buf, "<section 0x%x>",
				   (unsigned int) psym->st_shndx);
			  sec_name = name_buf;
			}
		    }
		  print_symbol (22, sec_name);
		}
	      else if (strtab == NULL)
		printf (_("<string table index: %3ld>"), psym->st_name);
	      else if (psym->st_name >= strtablen)
		printf (_("<corrupt string table index: %3ld>"), psym->st_name);
	      else
		print_symbol (22, strtab + psym->st_name);

	      if (is_rela)
		printf (" + %lx", (unsigned long) rels[i].r_addend);
	    }
	}
      else if (is_rela)
	{
	  printf ("%*c", is_32bit_elf ?
		  (do_wide ? 34 : 28) : (do_wide ? 26 : 20), ' ');
	  print_vma (rels[i].r_addend, LONG_HEX);
	}

      if ((elf_header.e_machine == EM_SPARCV9) && (rtype != NULL)
      	  && streq(rtype, "R_SPARC_OLO10"))
	printf(" + %lx", (unsigned long)ELF64_R_TYPE_DATA(info));

      putchar('\n');

      if (! is_32bit_elf && elf_header.e_machine == EM_MIPS)
	{
	  printf ("                    Type2: ");

	  if (rtype2 == NULL)
#ifdef _bfd_int64_low
	    printf (_("unrecognized: %-7lx"), _bfd_int64_low (type2));
#else
	    printf (_("unrecognized: %-7lx"), type2);
#endif /* _bfd_int64_low */
	  else
	    printf ("%-17.17s", rtype2);

	  printf ("\n                    Type3: ");

	  if (rtype3 == NULL)
#ifdef _bfd_int64_low
	    printf (_("unrecognized: %-7lx"), _bfd_int64_low (type3));
#else
	    printf (_("unrecognized: %-7lx"), type3);
#endif /* _bfd_int64_low */
	  else
	    printf ("%-17.17s", rtype3);

	  putchar ('\n');
	}
    }

  free (rels);

  return 1;
}

static const char *
get_mips_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_MIPS_RLD_VERSION: return "MIPS_RLD_VERSION";
    case DT_MIPS_TIME_STAMP: return "MIPS_TIME_STAMP";
    case DT_MIPS_ICHECKSUM: return "MIPS_ICHECKSUM";
    case DT_MIPS_IVERSION: return "MIPS_IVERSION";
    case DT_MIPS_FLAGS: return "MIPS_FLAGS";
    case DT_MIPS_BASE_ADDRESS: return "MIPS_BASE_ADDRESS";
    case DT_MIPS_MSYM: return "MIPS_MSYM";
    case DT_MIPS_CONFLICT: return "MIPS_CONFLICT";
    case DT_MIPS_LIBLIST: return "MIPS_LIBLIST";
    case DT_MIPS_LOCAL_GOTNO: return "MIPS_LOCAL_GOTNO";
    case DT_MIPS_CONFLICTNO: return "MIPS_CONFLICTNO";
    case DT_MIPS_LIBLISTNO: return "MIPS_LIBLISTNO";
    case DT_MIPS_SYMTABNO: return "MIPS_SYMTABNO";
    case DT_MIPS_UNREFEXTNO: return "MIPS_UNREFEXTNO";
    case DT_MIPS_GOTSYM: return "MIPS_GOTSYM";
    case DT_MIPS_HIPAGENO: return "MIPS_HIPAGENO";
    case DT_MIPS_RLD_MAP: return "MIPS_RLD_MAP";
    case DT_MIPS_DELTA_CLASS: return "MIPS_DELTA_CLASS";
    case DT_MIPS_DELTA_CLASS_NO: return "MIPS_DELTA_CLASS_NO";
    case DT_MIPS_DELTA_INSTANCE: return "MIPS_DELTA_INSTANCE";
    case DT_MIPS_DELTA_INSTANCE_NO: return "MIPS_DELTA_INSTANCE_NO";
    case DT_MIPS_DELTA_RELOC: return "MIPS_DELTA_RELOC";
    case DT_MIPS_DELTA_RELOC_NO: return "MIPS_DELTA_RELOC_NO";
    case DT_MIPS_DELTA_SYM: return "MIPS_DELTA_SYM";
    case DT_MIPS_DELTA_SYM_NO: return "MIPS_DELTA_SYM_NO";
    case DT_MIPS_DELTA_CLASSSYM: return "MIPS_DELTA_CLASSSYM";
    case DT_MIPS_DELTA_CLASSSYM_NO: return "MIPS_DELTA_CLASSSYM_NO";
    case DT_MIPS_CXX_FLAGS: return "MIPS_CXX_FLAGS";
    case DT_MIPS_PIXIE_INIT: return "MIPS_PIXIE_INIT";
    case DT_MIPS_SYMBOL_LIB: return "MIPS_SYMBOL_LIB";
    case DT_MIPS_LOCALPAGE_GOTIDX: return "MIPS_LOCALPAGE_GOTIDX";
    case DT_MIPS_LOCAL_GOTIDX: return "MIPS_LOCAL_GOTIDX";
    case DT_MIPS_HIDDEN_GOTIDX: return "MIPS_HIDDEN_GOTIDX";
    case DT_MIPS_PROTECTED_GOTIDX: return "MIPS_PROTECTED_GOTIDX";
    case DT_MIPS_OPTIONS: return "MIPS_OPTIONS";
    case DT_MIPS_INTERFACE: return "MIPS_INTERFACE";
    case DT_MIPS_DYNSTR_ALIGN: return "MIPS_DYNSTR_ALIGN";
    case DT_MIPS_INTERFACE_SIZE: return "MIPS_INTERFACE_SIZE";
    case DT_MIPS_RLD_TEXT_RESOLVE_ADDR: return "MIPS_RLD_TEXT_RESOLVE_ADDR";
    case DT_MIPS_PERF_SUFFIX: return "MIPS_PERF_SUFFIX";
    case DT_MIPS_COMPACT_SIZE: return "MIPS_COMPACT_SIZE";
    case DT_MIPS_GP_VALUE: return "MIPS_GP_VALUE";
    case DT_MIPS_AUX_DYNAMIC: return "MIPS_AUX_DYNAMIC";
    default: return NULL;
    }
}

static const char *
get_sparc64_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_SPARC_REGISTER: return "SPARC_REGISTER";
    default: return NULL;
    }
}

static const char *
get_ppc_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_PPC_GOT: return "PPC_GOT";
    default: return NULL;
    }
}

static const char *
get_ppc64_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_PPC64_GLINK: return "PPC64_GLINK";
    case DT_PPC64_OPD:   return "PPC64_OPD";
    case DT_PPC64_OPDSZ: return "PPC64_OPDSZ";
    default: return NULL;
    }
}

static const char *
get_parisc_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_HP_LOAD_MAP:	return "HP_LOAD_MAP";
    case DT_HP_DLD_FLAGS:	return "HP_DLD_FLAGS";
    case DT_HP_DLD_HOOK:	return "HP_DLD_HOOK";
    case DT_HP_UX10_INIT:	return "HP_UX10_INIT";
    case DT_HP_UX10_INITSZ:	return "HP_UX10_INITSZ";
    case DT_HP_PREINIT:		return "HP_PREINIT";
    case DT_HP_PREINITSZ:	return "HP_PREINITSZ";
    case DT_HP_NEEDED:		return "HP_NEEDED";
    case DT_HP_TIME_STAMP:	return "HP_TIME_STAMP";
    case DT_HP_CHECKSUM:	return "HP_CHECKSUM";
    case DT_HP_GST_SIZE:	return "HP_GST_SIZE";
    case DT_HP_GST_VERSION:	return "HP_GST_VERSION";
    case DT_HP_GST_HASHVAL:	return "HP_GST_HASHVAL";
    case DT_HP_EPLTREL:		return "HP_GST_EPLTREL";
    case DT_HP_EPLTRELSZ:	return "HP_GST_EPLTRELSZ";
    case DT_HP_FILTERED:	return "HP_FILTERED";
    case DT_HP_FILTER_TLS:	return "HP_FILTER_TLS";
    case DT_HP_COMPAT_FILTERED:	return "HP_COMPAT_FILTERED";
    case DT_HP_LAZYLOAD:	return "HP_LAZYLOAD";
    case DT_HP_BIND_NOW_COUNT:	return "HP_BIND_NOW_COUNT";
    case DT_PLT:		return "PLT";
    case DT_PLT_SIZE:		return "PLT_SIZE";
    case DT_DLT:		return "DLT";
    case DT_DLT_SIZE:		return "DLT_SIZE";
    default: return NULL;
    }
}

static const char *
get_ia64_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_IA_64_PLT_RESERVE: return "IA_64_PLT_RESERVE";
    default: return NULL;
    }
}

static const char *
get_alpha_dynamic_type (unsigned long type)
{
  switch (type)
    {
    case DT_ALPHA_PLTRO: return "ALPHA_PLTRO";
    default: return NULL;
    }
}

static const char *
get_dynamic_type (unsigned long type)
{
  static char buff[64];

  switch (type)
    {
    case DT_NULL:	return "NULL";
    case DT_NEEDED:	return "NEEDED";
    case DT_PLTRELSZ:	return "PLTRELSZ";
    case DT_PLTGOT:	return "PLTGOT";
    case DT_HASH:	return "HASH";
    case DT_STRTAB:	return "STRTAB";
    case DT_SYMTAB:	return "SYMTAB";
    case DT_RELA:	return "RELA";
    case DT_RELASZ:	return "RELASZ";
    case DT_RELAENT:	return "RELAENT";
    case DT_STRSZ:	return "STRSZ";
    case DT_SYMENT:	return "SYMENT";
    case DT_INIT:	return "INIT";
    case DT_FINI:	return "FINI";
    case DT_SONAME:	return "SONAME";
    case DT_RPATH:	return "RPATH";
    case DT_SYMBOLIC:	return "SYMBOLIC";
    case DT_REL:	return "REL";
    case DT_RELSZ:	return "RELSZ";
    case DT_RELENT:	return "RELENT";
    case DT_PLTREL:	return "PLTREL";
    case DT_DEBUG:	return "DEBUG";
    case DT_TEXTREL:	return "TEXTREL";
    case DT_JMPREL:	return "JMPREL";
    case DT_BIND_NOW:   return "BIND_NOW";
    case DT_INIT_ARRAY: return "INIT_ARRAY";
    case DT_FINI_ARRAY: return "FINI_ARRAY";
    case DT_INIT_ARRAYSZ: return "INIT_ARRAYSZ";
    case DT_FINI_ARRAYSZ: return "FINI_ARRAYSZ";
    case DT_RUNPATH:    return "RUNPATH";
    case DT_FLAGS:      return "FLAGS";

    case DT_PREINIT_ARRAY: return "PREINIT_ARRAY";
    case DT_PREINIT_ARRAYSZ: return "PREINIT_ARRAYSZ";

    case DT_CHECKSUM:	return "CHECKSUM";
    case DT_PLTPADSZ:	return "PLTPADSZ";
    case DT_MOVEENT:	return "MOVEENT";
    case DT_MOVESZ:	return "MOVESZ";
    case DT_FEATURE:	return "FEATURE";
    case DT_POSFLAG_1:	return "POSFLAG_1";
    case DT_SYMINSZ:	return "SYMINSZ";
    case DT_SYMINENT:	return "SYMINENT"; /* aka VALRNGHI */

    case DT_ADDRRNGLO:  return "ADDRRNGLO";
    case DT_CONFIG:	return "CONFIG";
    case DT_DEPAUDIT:	return "DEPAUDIT";
    case DT_AUDIT:	return "AUDIT";
    case DT_PLTPAD:	return "PLTPAD";
    case DT_MOVETAB:	return "MOVETAB";
    case DT_SYMINFO:	return "SYMINFO"; /* aka ADDRRNGHI */

    case DT_VERSYM:	return "VERSYM";

    case DT_RELACOUNT:	return "RELACOUNT";
    case DT_RELCOUNT:	return "RELCOUNT";
    case DT_FLAGS_1:	return "FLAGS_1";
    case DT_VERDEF:	return "VERDEF";
    case DT_VERDEFNUM:	return "VERDEFNUM";
    case DT_VERNEED:	return "VERNEED";
    case DT_VERNEEDNUM:	return "VERNEEDNUM";

    case DT_AUXILIARY:	return "AUXILIARY";
    case DT_USED:	return "USED";
    case DT_FILTER:	return "FILTER";

    case DT_GNU_PRELINKED: return "GNU_PRELINKED";
    case DT_GNU_CONFLICT: return "GNU_CONFLICT";
    case DT_GNU_CONFLICTSZ: return "GNU_CONFLICTSZ";
    case DT_GNU_LIBLIST: return "GNU_LIBLIST";
    case DT_GNU_LIBLISTSZ: return "GNU_LIBLISTSZ";

    default:
      if ((type >= DT_LOPROC) && (type <= DT_HIPROC))
	{
	  const char *result;

	  switch (elf_header.e_machine)
	    {
	    case EM_MIPS:
	    case EM_MIPS_RS3_LE:
	      result = get_mips_dynamic_type(type); break;
	    case EM_SPARCV9:
	      result = get_sparc64_dynamic_type(type); break;
	    case EM_PPC:
	      result = get_ppc_dynamic_type(type); break;
	    case EM_PPC64:
	      result = get_ppc64_dynamic_type(type); break;
	    case EM_IA_64:
	      result = get_ia64_dynamic_type(type); break;
	    case EM_ALPHA:
	      result = get_alpha_dynamic_type(type); break;
	    default: result = NULL; break;
	    }

	  if (result != NULL)
	    return result;

	  snprintf (buff, sizeof (buff), _("Processor Specific: %lx"), type);
	}
      else if (((type >= DT_LOOS) && (type <= DT_HIOS))
	       || (elf_header.e_machine == EM_PARISC
		   && (type >= OLD_DT_LOOS) && (type <= OLD_DT_HIOS)))
	{
	  const char *result;

	  switch (elf_header.e_machine)
	    {
	    case EM_PARISC:
	      result = get_parisc_dynamic_type(type); break;
	    default: result = NULL; break;
	    }

	  if (result != NULL)
	    return result;

	  snprintf(buff, sizeof(buff), _("Operating System specific: %lx"),
		   type);
	}
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %lx"), type);

      return buff;
    }
}

static const  char *
get_file_type(unsigned e_type)
{
  static char buff[32];

  switch (e_type)
    {
    case ET_NONE:	return _("NONE (None)");
    case ET_REL:	return _("REL (Relocatable file)");
    case ET_EXEC:	return _("EXEC (Executable file)");
    case ET_DYN:	return _("DYN (Shared object file)");
    case ET_CORE:	return _("CORE (Core file)");

    default:
      if ((e_type >= ET_LOPROC) && (e_type <= ET_HIPROC))
	snprintf(buff, sizeof(buff), _("Processor Specific: (%x)"), e_type);
      else if ((e_type >= ET_LOOS) && (e_type <= ET_HIOS))
	snprintf(buff, sizeof(buff), _("OS Specific: (%x)"), e_type);
      else
	snprintf(buff, sizeof(buff), _("<unknown>: %x"), e_type);
      return buff;
    }
}

static const char *
get_machine_name(unsigned e_machine)
{
  static char buff[64]; /* XXX */

  switch (e_machine)
    {
    case EM_NONE:		return _("None");
    case EM_M32:		return "WE32100";
    case EM_SPARC:		return "Sparc";
    case EM_386:		return "Intel 80386";
    case EM_68K:		return "MC68000";
    case EM_88K:		return "MC88000";
    case EM_486:		return "Intel 80486";
    case EM_860:		return "Intel 80860";
    case EM_MIPS:		return "MIPS R3000";
    case EM_S370:		return "IBM System/370";
    case EM_MIPS_RS3_LE:	return "MIPS R4000 big-endian";
    case EM_OLD_SPARCV9:	return "Sparc v9 (old)";
    case EM_PARISC:		return "HPPA";
    case EM_PPC_OLD:		return "Power PC (old)";
    case EM_SPARC32PLUS:	return "Sparc v8+" ;
    case EM_960:		return "Intel 90860";
    case EM_PPC:		return "PowerPC";
    case EM_PPC64:		return "PowerPC64";
    case EM_V800:		return "NEC V800";
    case EM_FR20:		return "Fujitsu FR20";
    case EM_RH32:		return "TRW RH32";
    case EM_MCORE:		return "MCORE";
    case EM_ARM:		return "ARM";
    case EM_OLD_ALPHA:		return "Digital Alpha (old)";
    case EM_SH:			return "Renesas / SuperH SH";
    case EM_SPARCV9:		return "Sparc v9";
    case EM_TRICORE:		return "Siemens Tricore";
    case EM_ARC:		return "ARC";
    case EM_H8_300:		return "Renesas H8/300";
    case EM_H8_300H:		return "Renesas H8/300H";
    case EM_H8S:		return "Renesas H8S";
    case EM_H8_500:		return "Renesas H8/500";
    case EM_IA_64:		return "Intel IA-64";
    case EM_MIPS_X:		return "Stanford MIPS-X";
    case EM_COLDFIRE:		return "Motorola Coldfire";
    case EM_68HC12:		return "Motorola M68HC12";
    case EM_ALPHA:		return "Alpha";
    case EM_CYGNUS_D10V:
    case EM_D10V:		return "d10v";
    case EM_CYGNUS_D30V:
    case EM_D30V:		return "d30v";
    case EM_CYGNUS_M32R:
    case EM_M32R:		return "Renesas M32R (formerly Mitsubishi M32r)";
    case EM_CYGNUS_V850:
    case EM_V850:		return "NEC v850";
    case EM_CYGNUS_MN10300:
    case EM_MN10300:		return "mn10300";
    case EM_CYGNUS_MN10200:
    case EM_MN10200:		return "mn10200";
    case EM_CYGNUS_FR30:
    case EM_FR30:		return "Fujitsu FR30";
    case EM_CYGNUS_FRV:		return "Fujitsu FR-V";
    case EM_PJ_OLD:
    case EM_PJ:			return "picoJava";
    case EM_MMA:		return "Fujitsu Multimedia Accelerator";
    case EM_PCP:		return "Siemens PCP";
    case EM_NCPU:		return "Sony nCPU embedded RISC processor";
    case EM_NDR1:		return "Denso NDR1 microprocesspr";
    case EM_STARCORE:		return "Motorola Star*Core processor";
    case EM_ME16:		return "Toyota ME16 processor";
    case EM_ST100:		return "STMicroelectronics ST100 processor";
    case EM_TINYJ:		return "Advanced Logic Corp. TinyJ embedded processor";
    case EM_FX66:		return "Siemens FX66 microcontroller";
    case EM_ST9PLUS:		return "STMicroelectronics ST9+ 8/16 bit microcontroller";
    case EM_ST7:		return "STMicroelectronics ST7 8-bit microcontroller";
    case EM_68HC16:		return "Motorola MC68HC16 Microcontroller";
    case EM_68HC11:		return "Motorola MC68HC11 Microcontroller";
    case EM_68HC08:		return "Motorola MC68HC08 Microcontroller";
    case EM_68HC05:		return "Motorola MC68HC05 Microcontroller";
    case EM_SVX:		return "Silicon Graphics SVx";
    case EM_ST19:		return "STMicroelectronics ST19 8-bit microcontroller";
    case EM_VAX:		return "Digital VAX";
    case EM_AVR_OLD:
    case EM_AVR:		return "Atmel AVR 8-bit microcontroller";
    case EM_CRIS:		return "Axis Communications 32-bit embedded processor";
    case EM_JAVELIN:		return "Infineon Technologies 32-bit embedded cpu";
    case EM_FIREPATH:		return "Element 14 64-bit DSP processor";
    case EM_ZSP:		return "LSI Logic's 16-bit DSP processor";
    case EM_MMIX:		return "Donald Knuth's educational 64-bit processor";
    case EM_HUANY:		return "Harvard Universitys's machine-independent object format";
    case EM_PRISM:		return "Vitesse Prism";
    case EM_X86_64:		return "Advanced Micro Devices X86-64";
    case EM_S390_OLD:
    case EM_S390:		return "IBM S/390";
    case EM_XSTORMY16:		return "Sanyo Xstormy16 CPU core";
    case EM_OPENRISC:
    case EM_OR32:		return "OpenRISC";
    case EM_CRX:		return "National Semiconductor CRX microprocessor";
    case EM_DLX:		return "OpenDLX";
    case EM_IP2K_OLD:
    case EM_IP2K:		return "Ubicom IP2xxx 8-bit microcontrollers";
    case EM_IQ2000:       	return "Vitesse IQ2000";
    case EM_XTENSA_OLD:
    case EM_XTENSA:		return "Tensilica Xtensa Processor";
    case EM_M32C:	        return "Renesas M32c";
    case EM_MS1:                return "Morpho Techologies MS1 processor";
    default:
      snprintf(buff, sizeof(buff), _("<unknown>: %x"), e_machine);
      return buff;
    }
}

static void
decode_ARM_machine_flags(unsigned e_flags, char buf[])
{
  unsigned eabi;
  int unknown = 0;

  eabi = EF_ARM_EABI_VERSION (e_flags);
  e_flags &= ~ EF_ARM_EABIMASK;

  /* Handle "generic" ARM flags.  */
  if (e_flags & EF_ARM_RELEXEC)
    {
      strcat (buf, ", relocatable executable");
      e_flags &= ~ EF_ARM_RELEXEC;
    }

  if (e_flags & EF_ARM_HASENTRY)
    {
      strcat (buf, ", has entry point");
      e_flags &= ~ EF_ARM_HASENTRY;
    }

  /* Now handle EABI specific flags.  */
  switch (eabi)
    {
    default:
      strcat(buf, ", <unrecognized EABI>");
      if (e_flags)
	unknown = 1;
      break;
    case EF_ARM_EABI_VER1:
      strcat(buf, ", Version1 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_SYMSARESORTED: /* Conflicts with EF_ARM_INTERWORK.  */
	      strcat (buf, ", sorted symbol tables");
	      break;

	    default:
	      unknown = 1;
	      break;
	    }
	}
      break;
    case EF_ARM_EABI_VER2:
      strcat(buf, ", Version2 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_SYMSARESORTED: /* Conflicts with EF_ARM_INTERWORK.  */
	      strcat (buf, ", sorted symbol tables");
	      break;

	    case EF_ARM_DYNSYMSUSESEGIDX:
	      strcat (buf, ", dynamic symbols use segment index");
	      break;

	    case EF_ARM_MAPSYMSFIRST:
	      strcat (buf, ", mapping symbols precede others");
	      break;

	    default:
	      unknown = 1;
	      break;
	    }
	}
      break;
    case EF_ARM_EABI_VER3:
      strcat(buf, ", Version3 EABI");
      break;
    case EF_ARM_EABI_VER4:
      strcat(buf, ", Version4 EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_BE8:
	      strcat (buf, ", BE8");
	      break;

	    case EF_ARM_LE8:
	      strcat (buf, ", LE8");
	      break;

	    default:
	      unknown = 1;
	      break;
	    }
	}
      break;
    case EF_ARM_EABI_UNKNOWN:
      strcat(buf, ", GNU EABI");
      while (e_flags)
	{
	  unsigned flag;

	  /* Process flags one bit at a time.  */
	  flag = e_flags & - e_flags;
	  e_flags &= ~ flag;

	  switch (flag)
	    {
	    case EF_ARM_INTERWORK:
	      strcat (buf, ", interworking enabled");
	      break;

	    case EF_ARM_APCS_26:
	      strcat (buf, ", uses APCS/26");
	      break;

	    case EF_ARM_APCS_FLOAT:
	      strcat (buf, ", uses APCS/float");
	      break;

	    case EF_ARM_PIC:
	      strcat (buf, ", position independent");
	      break;

	    case EF_ARM_ALIGN8:
	      strcat (buf, ", 8 bit structure alignment");
	      break;

	    case EF_ARM_NEW_ABI:
	      strcat (buf, ", uses new ABI");
	      break;

	    case EF_ARM_OLD_ABI:
	      strcat (buf, ", uses old ABI");
	      break;

	    case EF_ARM_SOFT_FLOAT:
	      strcat (buf, ", software FP");
	      break;

	    case EF_ARM_VFP_FLOAT:
	      strcat (buf, ", VFP");
	      break;

	    case EF_ARM_MAVERICK_FLOAT:
	      strcat (buf, ", Maverick FP");
	      break;

	    default:
	      unknown = 1;
	      break;
	    }
	}
    }

  if (unknown)
    strcat(buf,", <unknown>");
}

static char *
get_machine_flags (unsigned e_flags, unsigned e_machine)
{
  static char buf[1024];

  buf[0] = '\0';

  if (e_flags)
    {
      switch (e_machine)
	{
	default: break;
	case EM_ARM: decode_ARM_machine_flags(e_flags, buf); break;
	case EM_CYGNUS_FRV:
	  switch (e_flags & EF_FRV_CPU_MASK)
	    {
	    case EF_FRV_CPU_GENERIC: break;
	    default: strcat(buf, ", fr???"); break;
	    case EF_FRV_CPU_FR300: strcat(buf, ", fr300"); break;
	    case EF_FRV_CPU_FR400: strcat(buf, ", fr400"); break;
	    case EF_FRV_CPU_FR405: strcat(buf, ", fr405"); break;
	    case EF_FRV_CPU_FR450: strcat(buf, ", fr450"); break;
	    case EF_FRV_CPU_FR500: strcat(buf, ", fr500"); break;
	    case EF_FRV_CPU_FR550: strcat(buf, ", fr550"); break;
	    case EF_FRV_CPU_SIMPLE: strcat(buf, ", simple"); break;
	    case EF_FRV_CPU_TOMCAT: strcat(buf, ", tomcat"); break;
	    }
	  break;

	case EM_68K:
	  if (e_flags & EF_CPU32)
	    strcat (buf, ", cpu32");
	  if (e_flags & EF_M68000)
	    strcat (buf, ", m68000");
	  break;

	case EM_PPC:
	  if (e_flags & EF_PPC_EMB)
	    strcat (buf, ", emb");

	  if (e_flags & EF_PPC_RELOCATABLE)
	    strcat (buf, ", relocatable");

	  if (e_flags & EF_PPC_RELOCATABLE_LIB)
	    strcat (buf, ", relocatable-lib");
	  break;

	case EM_V850:
	case EM_CYGNUS_V850:
	  switch (e_flags & EF_V850_ARCH)
	    {
	    case E_V850E1_ARCH: strcat(buf, ", v850e1"); break;
	    case E_V850E_ARCH: strcat(buf, ", v850e"); break;
	    case E_V850_ARCH: strcat(buf, ", v850"); break;
	    default: strcat(buf, ", unknown v850 architecture variant");
	      break;
	    }
	  break;

	case EM_M32R:
	case EM_CYGNUS_M32R:
	  if ((e_flags & EF_M32R_ARCH) == E_M32R_ARCH)
	    strcat (buf, ", m32r");

	  break;

	case EM_MIPS:
	case EM_MIPS_RS3_LE:
	  if (e_flags & EF_MIPS_NOREORDER)
	    strcat (buf, ", noreorder");

	  if (e_flags & EF_MIPS_PIC)
	    strcat (buf, ", pic");

	  if (e_flags & EF_MIPS_CPIC)
	    strcat (buf, ", cpic");

	  if (e_flags & EF_MIPS_UCODE)
	    strcat (buf, ", ugen_reserved");

	  if (e_flags & EF_MIPS_ABI2)
	    strcat (buf, ", abi2");

	  if (e_flags & EF_MIPS_OPTIONS_FIRST)
	    strcat (buf, ", odk first");

	  if (e_flags & EF_MIPS_32BITMODE)
	    strcat (buf, ", 32bitmode");

	  switch ((e_flags & EF_MIPS_MACH))
	    {
	    case E_MIPS_MACH_3900: strcat (buf, ", 3900"); break;
	    case E_MIPS_MACH_4010: strcat (buf, ", 4010"); break;
	    case E_MIPS_MACH_4100: strcat (buf, ", 4100"); break;
	    case E_MIPS_MACH_4111: strcat (buf, ", 4111"); break;
	    case E_MIPS_MACH_4120: strcat (buf, ", 4120"); break;
	    case E_MIPS_MACH_4650: strcat (buf, ", 4650"); break;
	    case E_MIPS_MACH_5400: strcat (buf, ", 5400"); break;
	    case E_MIPS_MACH_5500: strcat (buf, ", 5500"); break;
	    case E_MIPS_MACH_SB1:  strcat (buf, ", sb1");  break;
	    case E_MIPS_MACH_9000: strcat (buf, ", 9000"); break;
	    case 0:
	    /* We simply ignore the field in this case to avoid confusion:
	       MIPS ELF does not specify EF_MIPS_MACH, it is a GNU
	       extension.  */
	      break;
	    default: strcat (buf, ", unknown CPU"); break;
	    }

	  switch ((e_flags & EF_MIPS_ABI))
	    {
	    case E_MIPS_ABI_O32: strcat (buf, ", o32"); break;
	    case E_MIPS_ABI_O64: strcat (buf, ", o64"); break;
	    case E_MIPS_ABI_EABI32: strcat (buf, ", eabi32"); break;
	    case E_MIPS_ABI_EABI64: strcat (buf, ", eabi64"); break;
	    case 0:
	    /* We simply ignore the field in this case to avoid confusion:
	       MIPS ELF does not specify EF_MIPS_ABI, it is a GNU extension.
	       This means it is likely to be an o32 file, but not for
	       sure.  */
	      break;
	    default: strcat (buf, ", unknown ABI"); break;
	    }

	  if (e_flags & EF_MIPS_ARCH_ASE_MDMX)
	    strcat (buf, ", mdmx");

	  if (e_flags & EF_MIPS_ARCH_ASE_M16)
	    strcat (buf, ", mips16");

	  switch ((e_flags & EF_MIPS_ARCH))
	    {
	    case E_MIPS_ARCH_1: strcat (buf, ", mips1"); break;
	    case E_MIPS_ARCH_2: strcat (buf, ", mips2"); break;
	    case E_MIPS_ARCH_3: strcat (buf, ", mips3"); break;
	    case E_MIPS_ARCH_4: strcat (buf, ", mips4"); break;
	    case E_MIPS_ARCH_5: strcat (buf, ", mips5"); break;
	    case E_MIPS_ARCH_32: strcat (buf, ", mips32"); break;
	    case E_MIPS_ARCH_32R2: strcat (buf, ", mips32r2"); break;
	    case E_MIPS_ARCH_64: strcat (buf, ", mips64"); break;
	    case E_MIPS_ARCH_64R2: strcat (buf, ", mips64r2"); break;
	    default: strcat (buf, ", unknown ISA"); break;
	    }

	  break;

	case EM_SH:
	  switch ((e_flags & EF_SH_MACH_MASK))
	    {
	    case EF_SH1: strcat (buf, ", sh1"); break;
	    case EF_SH2: strcat (buf, ", sh2"); break;
	    case EF_SH3: strcat (buf, ", sh3"); break;
	    case EF_SH_DSP: strcat (buf, ", sh-dsp"); break;
	    case EF_SH3_DSP: strcat (buf, ", sh3-dsp"); break;
	    case EF_SH4AL_DSP: strcat (buf, ", sh4al-dsp"); break;
	    case EF_SH3E: strcat (buf, ", sh3e"); break;
	    case EF_SH4: strcat (buf, ", sh4"); break;
	    case EF_SH5: strcat (buf, ", sh5"); break;
	    case EF_SH2E: strcat (buf, ", sh2e"); break;
	    case EF_SH4A: strcat (buf, ", sh4a"); break;
	    case EF_SH2A: strcat (buf, ", sh2a"); break;
	    case EF_SH4_NOFPU: strcat (buf, ", sh4-nofpu"); break;
	    case EF_SH4A_NOFPU: strcat (buf, ", sh4a-nofpu"); break;
	    case EF_SH2A_NOFPU: strcat (buf, ", sh2a-nofpu"); break;
	    default: strcat (buf, ", unknown ISA"); break;
	    }

	  break;

	case EM_SPARCV9:
	  if (e_flags & EF_SPARC_32PLUS)
	    strcat (buf, ", v8+");

	  if (e_flags & EF_SPARC_SUN_US1)
	    strcat (buf, ", ultrasparcI");

	  if (e_flags & EF_SPARC_SUN_US3)
	    strcat (buf, ", ultrasparcIII");

	  if (e_flags & EF_SPARC_HAL_R1)
	    strcat (buf, ", halr1");

	  if (e_flags & EF_SPARC_LEDATA)
	    strcat (buf, ", ledata");

	  if ((e_flags & EF_SPARCV9_MM) == EF_SPARCV9_TSO)
	    strcat (buf, ", tso");

	  if ((e_flags & EF_SPARCV9_MM) == EF_SPARCV9_PSO)
	    strcat (buf, ", pso");

	  if ((e_flags & EF_SPARCV9_MM) == EF_SPARCV9_RMO)
	    strcat (buf, ", rmo");
	  break;

	case EM_PARISC:
	  switch (e_flags & EF_PARISC_ARCH)
	    {
	    case EFA_PARISC_1_0: strcpy(buf, ", PA-RISC 1.0"); break;
	    case EFA_PARISC_1_1: strcpy(buf, ", PA-RISC 1.1"); break;
	    case EFA_PARISC_2_0: strcpy(buf, ", PA-RISC 2.0"); break;
	    default: break;
	    }
	  if (e_flags & EF_PARISC_TRAPNIL)
	    strcat (buf, ", trapnil");
	  if (e_flags & EF_PARISC_EXT)
	    strcat (buf, ", ext");
	  if (e_flags & EF_PARISC_LSB)
	    strcat (buf, ", lsb");
	  if (e_flags & EF_PARISC_WIDE)
	    strcat (buf, ", wide");
	  if (e_flags & EF_PARISC_NO_KABP)
	    strcat (buf, ", no kabp");
	  if (e_flags & EF_PARISC_LAZYSWAP)
	    strcat (buf, ", lazyswap");
	  break;

	case EM_PJ:
	case EM_PJ_OLD:
	  if ((e_flags & EF_PICOJAVA_NEWCALLS) == EF_PICOJAVA_NEWCALLS)
	    strcat (buf, ", new calling convention");

	  if ((e_flags & EF_PICOJAVA_GNUCALLS) == EF_PICOJAVA_GNUCALLS)
	    strcat (buf, ", gnu calling convention");
	  break;

	case EM_IA_64:
	  if ((e_flags & EF_IA_64_ABI64))
	    strcat (buf, ", 64-bit");
	  else
	    strcat (buf, ", 32-bit");
	  if ((e_flags & EF_IA_64_REDUCEDFP))
	    strcat (buf, ", reduced fp model");
	  if ((e_flags & EF_IA_64_NOFUNCDESC_CONS_GP))
	    strcat (buf, ", no function descriptors, constant gp");
	  else if ((e_flags & EF_IA_64_CONS_GP))
	    strcat (buf, ", constant gp");
	  if ((e_flags & EF_IA_64_ABSOLUTE))
	    strcat (buf, ", absolute");
	  break;

	case EM_VAX:
	  if ((e_flags & EF_VAX_NONPIC))
	    strcat (buf, ", non-PIC");
	  if ((e_flags & EF_VAX_DFLOAT))
	    strcat (buf, ", D-Float");
	  if ((e_flags & EF_VAX_GFLOAT))
	    strcat (buf, ", G-Float");
	  break;
	}
    }

  return buf;
}

static const char *
get_osabi_name (unsigned int osabi)
{
  static char buff[32];

  switch (osabi)
    {
    case ELFOSABI_NONE:		return "UNIX - System V";
    case ELFOSABI_HPUX:		return "UNIX - HP-UX";
    case ELFOSABI_NETBSD:	return "UNIX - NetBSD";
    case ELFOSABI_LINUX:	return "UNIX - Linux";
    case ELFOSABI_HURD:		return "GNU/Hurd";
    case ELFOSABI_SOLARIS:	return "UNIX - Solaris";
    case ELFOSABI_AIX:		return "UNIX - AIX";
    case ELFOSABI_IRIX:		return "UNIX - IRIX";
    case ELFOSABI_FREEBSD:	return "UNIX - FreeBSD";
    case ELFOSABI_TRU64:	return "UNIX - TRU64";
    case ELFOSABI_MODESTO:	return "Novell - Modesto";
    case ELFOSABI_OPENBSD:	return "UNIX - OpenBSD";
    case ELFOSABI_OPENVMS:	return "VMS - OpenVMS";
    case ELFOSABI_NSK:		return "HP - Non-Stop Kernel";
    case ELFOSABI_AROS:		return "Amiga Research OS";
    case ELFOSABI_STANDALONE:	return _("Standalone App");
    case ELFOSABI_ARM:		return "ARM";
    default:
      snprintf (buff, sizeof (buff), _("<unknown: %x>"), osabi);
      return buff;
    }
}

static const char *
get_arm_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_ARM_EXIDX: return "EXIDX";
    default: break;
    }

  return NULL;
}

static const char *
get_mips_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_MIPS_REGINFO: return "REGINFO";
    case PT_MIPS_RTPROC: return "RTPROC";
    case PT_MIPS_OPTIONS: return "OPTIONS";
    default: break;
    }

  return NULL;
}

static const char *
get_parisc_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_HP_TLS:		return "HP_TLS";
    case PT_HP_CORE_NONE:	return "HP_CORE_NONE";
    case PT_HP_CORE_VERSION:	return "HP_CORE_VERSION";
    case PT_HP_CORE_KERNEL:	return "HP_CORE_KERNEL";
    case PT_HP_CORE_COMM:	return "HP_CORE_COMM";
    case PT_HP_CORE_PROC:	return "HP_CORE_PROC";
    case PT_HP_CORE_LOADABLE:	return "HP_CORE_LOADABLE";
    case PT_HP_CORE_STACK:	return "HP_CORE_STACK";
    case PT_HP_CORE_SHM:	return "HP_CORE_SHM";
    case PT_HP_CORE_MMF:	return "HP_CORE_MMF";
    case PT_HP_PARALLEL:	return "HP_PARALLEL";
    case PT_HP_FASTBIND:	return "HP_FASTBIND";
    case PT_HP_OPT_ANNOT:	return "HP_OPT_ANNOT";
    case PT_HP_HSL_ANNOT:	return "HP_HSL_ANNOT";
    case PT_HP_STACK:		return "HP_STACK";
    case PT_HP_CORE_UTSNAME:	return "HP_CORE_UTSNAME";
    case PT_PARISC_ARCHEXT:	return "PARISC_ARCHEXT";
    case PT_PARISC_UNWIND:	return "PARISC_UNWIND";
    case PT_PARISC_WEAKORDER:	return "PARISC_WEAKORDER";
    default: break;
    }

  return NULL;
}

static const char *
get_ia64_segment_type (unsigned long type)
{
  switch (type)
    {
    case PT_IA_64_ARCHEXT:	return "IA_64_ARCHEXT";
    case PT_IA_64_UNWIND:	return "IA_64_UNWIND";
    case PT_HP_TLS:		return "HP_TLS";
    case PT_IA_64_HP_OPT_ANOT:	return "HP_OPT_ANNOT";
    case PT_IA_64_HP_HSL_ANOT:	return "HP_HSL_ANNOT";
    case PT_IA_64_HP_STACK:	return "HP_STACK";
    default: break;
    }

  return NULL;
}

static const char *
get_segment_type (unsigned long p_type)
{
  static char buff[32];

  switch (p_type)
    {
    case PT_NULL:	return "NULL";
    case PT_LOAD:	return "LOAD";
    case PT_DYNAMIC:	return "DYNAMIC";
    case PT_INTERP:	return "INTERP";
    case PT_NOTE:	return "NOTE";
    case PT_SHLIB:	return "SHLIB";
    case PT_PHDR:	return "PHDR";
    case PT_TLS:	return "TLS";

    case PT_GNU_EH_FRAME:
			return "GNU_EH_FRAME";
    case PT_GNU_STACK:	return "GNU_STACK";
    case PT_GNU_RELRO:  return "GNU_RELRO";

    default:
      if ((p_type >= PT_LOPROC) && (p_type <= PT_HIPROC))
	{
	  const char *result;

	  switch (elf_header.e_machine)
	    {
	    case EM_ARM:
	      result = get_arm_segment_type (p_type);
	      break;
	    case EM_MIPS:
	    case EM_MIPS_RS3_LE:
	      result = get_mips_segment_type (p_type);
	      break;
	    case EM_PARISC:
	      result = get_parisc_segment_type (p_type);
	      break;
	    case EM_IA_64:
	      result = get_ia64_segment_type (p_type);
	      break;
	    default:
	      result = NULL;
	      break;
	    }

	  if (result != NULL)
	    return result;

	  sprintf (buff, "LOPROC+%lx", p_type - PT_LOPROC);
	}
      else if ((p_type >= PT_LOOS) && (p_type <= PT_HIOS))
	{
	  const char *result;

	  switch (elf_header.e_machine)
	    {
	    case EM_PARISC:
	      result = get_parisc_segment_type (p_type);
	      break;
	    case EM_IA_64:
	      result = get_ia64_segment_type (p_type);
	      break;
	    default:
	      result = NULL;
	      break;
	    }

	  if (result != NULL)
	    return result;

	  sprintf (buff, "LOOS+%lx", p_type - PT_LOOS);
	}
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %lx"), p_type);

      return buff;
    }
}

static const char *
get_mips_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_MIPS_LIBLIST:	 return "MIPS_LIBLIST";
    case SHT_MIPS_MSYM:		 return "MIPS_MSYM";
    case SHT_MIPS_CONFLICT:	 return "MIPS_CONFLICT";
    case SHT_MIPS_GPTAB:	 return "MIPS_GPTAB";
    case SHT_MIPS_UCODE:	 return "MIPS_UCODE";
    case SHT_MIPS_DEBUG:	 return "MIPS_DEBUG";
    case SHT_MIPS_REGINFO:	 return "MIPS_REGINFO";
    case SHT_MIPS_PACKAGE:	 return "MIPS_PACKAGE";
    case SHT_MIPS_PACKSYM:	 return "MIPS_PACKSYM";
    case SHT_MIPS_RELD:		 return "MIPS_RELD";
    case SHT_MIPS_IFACE:	 return "MIPS_IFACE";
    case SHT_MIPS_CONTENT:	 return "MIPS_CONTENT";
    case SHT_MIPS_OPTIONS:	 return "MIPS_OPTIONS";
    case SHT_MIPS_SHDR:		 return "MIPS_SHDR";
    case SHT_MIPS_FDESC:	 return "MIPS_FDESC";
    case SHT_MIPS_EXTSYM:	 return "MIPS_EXTSYM";
    case SHT_MIPS_DENSE:	 return "MIPS_DENSE";
    case SHT_MIPS_PDESC:	 return "MIPS_PDESC";
    case SHT_MIPS_LOCSYM:	 return "MIPS_LOCSYM";
    case SHT_MIPS_AUXSYM:	 return "MIPS_AUXSYM";
    case SHT_MIPS_OPTSYM:	 return "MIPS_OPTSYM";
    case SHT_MIPS_LOCSTR:	 return "MIPS_LOCSTR";
    case SHT_MIPS_LINE:		 return "MIPS_LINE";
    case SHT_MIPS_RFDESC:	 return "MIPS_RFDESC";
    case SHT_MIPS_DELTASYM:	 return "MIPS_DELTASYM";
    case SHT_MIPS_DELTAINST:	 return "MIPS_DELTAINST";
    case SHT_MIPS_DELTACLASS:	 return "MIPS_DELTACLASS";
    case SHT_MIPS_DWARF:	 return "MIPS_DWARF";
    case SHT_MIPS_DELTADECL:	 return "MIPS_DELTADECL";
    case SHT_MIPS_SYMBOL_LIB:	 return "MIPS_SYMBOL_LIB";
    case SHT_MIPS_EVENTS:	 return "MIPS_EVENTS";
    case SHT_MIPS_TRANSLATE:	 return "MIPS_TRANSLATE";
    case SHT_MIPS_PIXIE:	 return "MIPS_PIXIE";
    case SHT_MIPS_XLATE:	 return "MIPS_XLATE";
    case SHT_MIPS_XLATE_DEBUG:	 return "MIPS_XLATE_DEBUG";
    case SHT_MIPS_WHIRL:	 return "MIPS_WHIRL";
    case SHT_MIPS_EH_REGION:	 return "MIPS_EH_REGION";
    case SHT_MIPS_XLATE_OLD:	 return "MIPS_XLATE_OLD";
    case SHT_MIPS_PDR_EXCEPTION: return "MIPS_PDR_EXCEPTION";
    default: break;
    }
  return NULL;
}

static const char *
get_parisc_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_PARISC_EXT:	return "PARISC_EXT";
    case SHT_PARISC_UNWIND:	return "PARISC_UNWIND";
    case SHT_PARISC_DOC:	return "PARISC_DOC";
    case SHT_PARISC_ANNOT:	return "PARISC_ANNOT";
    case SHT_PARISC_SYMEXTN:	return "PARISC_SYMEXTN";
    case SHT_PARISC_STUBS:	return "PARISC_STUBS";
    case SHT_PARISC_DLKM:	return "PARISC_DLKM";
    default: break;
    }
  return NULL;
}

static const char *
get_ia64_section_type_name (unsigned int sh_type)
{
  /* If the top 8 bits are 0x78 the next 8 are the os/abi ID.  */
  if ((sh_type & 0xFF000000) == SHT_IA_64_LOPSREG)
    return get_osabi_name ((sh_type & 0x00FF0000) >> 16);

  switch (sh_type)
    {
    case SHT_IA_64_EXT:		  return "IA_64_EXT";
    case SHT_IA_64_UNWIND:	  return "IA_64_UNWIND";
    case SHT_IA_64_PRIORITY_INIT: return "IA_64_PRIORITY_INIT";
    default: break;
    }
  return NULL;
}

static const char *
get_x86_64_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_X86_64_UNWIND:	return "X86_64_UNWIND";
    default: break;
    }
  return NULL;
}

static const char *
get_arm_section_type_name (unsigned int sh_type)
{
  switch (sh_type)
    {
    case SHT_ARM_EXIDX: return "ARM_EXIDX";
    default: break;
    }
  return NULL;
}

static const char *
get_section_type_name (unsigned int sh_type)
{
  static char buff[32];

  switch (sh_type)
    {
    case SHT_NULL:		return "NULL";
    case SHT_PROGBITS:		return "PROGBITS";
    case SHT_SYMTAB:		return "SYMTAB";
    case SHT_STRTAB:		return "STRTAB";
    case SHT_RELA:		return "RELA";
    case SHT_HASH:		return "HASH";
    case SHT_DYNAMIC:		return "DYNAMIC";
    case SHT_NOTE:		return "NOTE";
    case SHT_NOBITS:		return "NOBITS";
    case SHT_REL:		return "REL";
    case SHT_SHLIB:		return "SHLIB";
    case SHT_DYNSYM:		return "DYNSYM";
    case SHT_INIT_ARRAY:	return "INIT_ARRAY";
    case SHT_FINI_ARRAY:	return "FINI_ARRAY";
    case SHT_PREINIT_ARRAY:	return "PREINIT_ARRAY";
    case SHT_GROUP:		return "GROUP";
    case SHT_SYMTAB_SHNDX:	return "SYMTAB SECTION INDICIES";
    case SHT_GNU_verdef:	return "VERDEF";
    case SHT_GNU_verneed:	return "VERNEED";
    case SHT_GNU_versym:	return "VERSYM";
    case 0x6ffffff0:		return "VERSYM";
    case 0x6ffffffc:		return "VERDEF";
    case 0x7ffffffd:		return "AUXILIARY";
    case 0x7fffffff:		return "FILTER";
    case SHT_GNU_LIBLIST:	return "GNU_LIBLIST";

    default:
      if ((sh_type >= SHT_LOPROC) && (sh_type <= SHT_HIPROC))
	{
	  const char *result;

	  switch (elf_header.e_machine)
	    {
	    case EM_MIPS:
	    case EM_MIPS_RS3_LE:
	      result = get_mips_section_type_name(sh_type); break;
	    case EM_PARISC:
	      result = get_parisc_section_type_name(sh_type); break;
	    case EM_IA_64:
	      result = get_ia64_section_type_name(sh_type); break;
	    case EM_X86_64:
	      result = get_x86_64_section_type_name(sh_type); break;
	    case EM_ARM:
	      result = get_arm_section_type_name(sh_type); break;
	    default:
	      result = NULL; break;
	    }

	  if (result != NULL)
	    return result;

	  sprintf (buff, "LOPROC+%x", sh_type - SHT_LOPROC);
	}
      else if ((sh_type >= SHT_LOOS) && (sh_type <= SHT_HIOS))
	sprintf (buff, "LOOS+%x", sh_type - SHT_LOOS);
      else if ((sh_type >= SHT_LOUSER) && (sh_type <= SHT_HIUSER))
	sprintf (buff, "LOUSER+%x", sh_type - SHT_LOUSER);
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %x"), sh_type);

      return buff;
    }
}

#define OPTION_DEBUG_DUMP	512

static struct option options[] =
{
  {"all",	       no_argument, 0, 'a'},
  {"file-header",      no_argument, 0, 'h'},
  {"program-headers",  no_argument, 0, 'l'},
  {"headers",	       no_argument, 0, 'e'},
  {"histogram",	       no_argument, 0, 'I'},
  {"segments",	       no_argument, 0, 'l'},
  {"sections",	       no_argument, 0, 'S'},
  {"section-headers",  no_argument, 0, 'S'},
  {"section-groups",   no_argument, 0, 'g'},
  {"section-details",  no_argument, 0, 't'},
  {"full-section-name",no_argument, 0, 'N'},
  {"symbols",	       no_argument, 0, 's'},
  {"syms",	       no_argument, 0, 's'},
  {"relocs",	       no_argument, 0, 'r'},
  {"notes",	       no_argument, 0, 'n'},
  {"dynamic",	       no_argument, 0, 'd'},
  {"arch-specific",    no_argument, 0, 'A'},
  {"version-info",     no_argument, 0, 'V'},
  {"use-dynamic",      no_argument, 0, 'D'},
  {"hex-dump",	       required_argument, 0, 'x'},
  {"debug-dump",       optional_argument, 0, OPTION_DEBUG_DUMP},
  {"unwind",	       no_argument, 0, 'u'},
#ifdef SUPPORT_DISASSEMBLY
  {"instruction-dump", required_argument, 0, 'i'},
#endif /* SUPPORT_DISASSEMBLY */

  {"version",	       no_argument, 0, 'v'},
  {"wide",	       no_argument, 0, 'W'},
  {"help",	       no_argument, 0, 'H'},
  {0,		       no_argument, 0, 0}
};

/* */
static void ATTRIBUTE_NORETURN
usage(void)
{
  fprintf(stdout, _("Usage: readelf <option(s)> elf-file(s)\n"));
  fprintf(stdout, _(" Display information about the contents of ELF format files\n"));
  fprintf(stdout, _(" Options are:\n\
  -a --all               Equivalent to: -h -l -S -s -r -d -V -A -I\n\
  -h --file-header       Display the ELF file header\n\
  -l --program-headers   Display the program headers\n\
     --segments          An alias for --program-headers\n\
  -S --section-headers   Display the sections' header\n\
     --sections          An alias for --section-headers\n\
  -g --section-groups    Display the section groups\n\
  -t --section-details   Display the section details\n\
  -e --headers           Equivalent to: -h -l -S\n\
  -s --syms              Display the symbol table\n\
      --symbols          An alias for --syms\n\
  -n --notes             Display the core notes (if present)\n\
  -r --relocs            Display the relocations (if present)\n\
  -u --unwind            Display the unwind info (if present)\n\
  -d --dynamic           Display the dynamic section (if present)\n\
  -V --version-info      Display the version sections (if present)\n\
  -A --arch-specific     Display architecture specific information (if any).\n\
  -D --use-dynamic       Use the dynamic section info when displaying symbols\n\
  -x --hex-dump=<number> Dump the contents of section <number>\n\
  -w[liaprmfFsoR] or\n\
  --debug-dump[=line,=info,=abbrev,=pubnames,=aranges,=macro,=frames,=str,=loc,=Ranges]\n\
                         Display the contents of DWARF2 debug sections\n"));
#ifdef SUPPORT_DISASSEMBLY
  fprintf(stdout, _("\
  -i --instruction-dump=<number>\n\
                         Disassemble the contents of section <number>\n"));
#endif /* SUPPORT_DISASSEMBLY */
  fprintf(stdout, _("\
  -I --histogram         Display histogram of bucket list lengths\n\
  -W --wide              Allow output width to exceed 80 characters\n\
  -H --help              Display this information\n\
  -v --version           Display the version number of readelf\n"));
  fprintf(stdout, _("Report bugs to %s\n"), REPORT_BUGS_TO);

  xexit(0);
}

/* Record the fact that the user wants the contents of section number
   SECTION to be displayed using the method(s) encoded as flags bits
   in TYPE.  Note, TYPE can be zero if we are creating the array for
   the first time.  */

static void
request_dump (unsigned int section, int type)
{
  if (section >= num_dump_sects)
    {
      char *new_dump_sects;

      new_dump_sects = (char *)calloc(section + 1, 1);

      if (new_dump_sects == NULL)
	error (_("Out of memory allocating dump request table."));
      else
	{
	  /* Copy current flag settings.  */
	  memcpy (new_dump_sects, dump_sects, num_dump_sects);

	  free (dump_sects);

	  dump_sects = new_dump_sects;
	  num_dump_sects = section + 1;
	}
    }

  if (dump_sects)
    dump_sects[section] |= type;

  return;
}

/* */
static void
parse_args (int argc, char **argv)
{
  int c;

  if (argc < 2)
    usage ();

  while ((c = getopt_long
	  (argc, argv, "ersuahnldSDAINtgw::x:i:vVWH", options, NULL)) != EOF)
    {
      char *cp;
      int section;

      switch (c)
	{
	case 0: /* Long options.  */ break;
	case 'H': usage(); break;
	case 'a': do_syms++; do_reloc++; do_unwind++; do_dynamic++;
	  do_header++; do_sections++; do_section_groups++; do_segments++;
	  do_version++; do_histogram++; do_arch++; do_notes++; break;
	case 'g': do_section_groups++; break;
	case 't': /* fall through to: */
	case 'N': do_sections++; do_section_details++; break;
	case 'e': do_header++; do_sections++; do_segments++; break;
	case 'A': do_arch++; break;
	case 'D': do_using_dynamic++; break;
	case 'r': do_reloc++; break;
	case 'u': do_unwind++; break;
	case 'h': do_header++; break;
	case 'l': do_segments++; break;
	case 's': do_syms++; break;
	case 'S': do_sections++; break;
	case 'd': do_dynamic++; break;
	case 'I': do_histogram++; break;
	case 'n': do_notes++; break;
	case 'x': do_dump++;
	  section = strtoul(optarg, & cp, 0);
	  if (! *cp && section >= 0)
	    {
	      request_dump(section, HEX_DUMP);
	      break;
	    }
	  goto oops;
	case 'w': do_dump++;
	  if (optarg == 0)
	    do_debugging = 1;
	  else
	    {
	      unsigned int u_index = 0U;

	      do_debugging = 0;

	      while (optarg[u_index])
		switch (optarg[u_index++])
		  {
		  case 'i': /* fall through to: */
		  case 'I': do_debug_info = 1; break;
		  case 'a': /* fall through to: */
		  case 'A': do_debug_abbrevs = 1; break;
		  case 'l': /* fall through to: */
		  case 'L': do_debug_lines = 1; break;
		  case 'p': /* fall through to: */
		  case 'P': do_debug_pubnames = 1;  break;
		  case 'r': do_debug_aranges = 1; break;
		  case 'R': do_debug_ranges = 1; break;
		  case 'F': do_debug_frames_interp = 1; break; /* ...right? */
		  case 'f': do_debug_frames = 1; break;
		  case 'm': /* fall through to: */
		  case 'M': do_debug_macinfo = 1; break;
		  case 's': /* fall through to: */
		  case 'S': do_debug_str = 1; break;
		  case 'o': /* fall through to: */
		  case 'O': do_debug_loc = 1; break;
		  default:
		    warn(_("Unrecognized debug option '%s'\n"), optarg);
		    break;
		  }
	    }
	  break;
	case OPTION_DEBUG_DUMP: do_dump++;
	  if (optarg == 0)
	    do_debugging = 1;
	  else
	    {
	      typedef struct
	      {
		const char * option;
		int *        variable;
	      }
	      debug_dump_long_opts;

	      debug_dump_long_opts opts_table [] =
		{
		  /* Please keep this table alpha- sorted.  */
		  { "Ranges", & do_debug_ranges },
		  { "abbrev", & do_debug_abbrevs },
		  { "aranges", & do_debug_aranges },
		  { "frames", & do_debug_frames },
		  { "frames-interp", & do_debug_frames_interp },
		  { "info", & do_debug_info },
		  { "line", & do_debug_lines },
		  { "loc",  & do_debug_loc },
		  { "macro", & do_debug_macinfo },
		  { "pubnames", & do_debug_pubnames },
		  /* This entry is for compatability
		     with earlier versions of readelf.  */
		  { "ranges", & do_debug_aranges },
		  { "str", & do_debug_str },
		  { NULL, NULL }
		};

	      const char *p;

	      do_debugging = 0;

	      p = optarg;
	      while (*p)
		{
		  debug_dump_long_opts * entry;

		  for (entry = opts_table; entry->option; entry++)
		    {
		      size_t len = strlen (entry->option);

		      if (strneq (p, entry->option, len)
			  && (p[len] == ',' || p[len] == '\0'))
			{
			  * entry->variable = 1;

			  /* The --debug-dump=frames-interp option also
			     enables the --debug-dump=frames option.  */
			  if (do_debug_frames_interp)
			    do_debug_frames = 1;

			  p += len;
			  break;
			}
		    }

		  if (entry->option == NULL)
		    {
		      warn (_("Unrecognized debug option '%s'\n"), p);
		      p = strchr (p, ',');
		      if (p == NULL)
			break;
		    }

		  if (*p == ',')
		    p++;
		}
	    }
	  break;
#ifdef SUPPORT_DISASSEMBLY
	case 'i': do_dump++;
	  section = strtoul (optarg, & cp, 0);
	  if (! *cp && section >= 0)
	    {
	      request_dump (section, DISASS_DUMP);
	      break;
	    }
	  goto oops;
#endif /* SUPPORT_DISASSEMBLY */
	case 'v': print_version(program_name); break;
	case 'V': do_version++; break;
	case 'W': do_wide++; break;
	default:
	oops:
	  /* xgettext:c-format */
	  error(_("Invalid option '-%c'\n"), c);
	  /* Drop through: */
	  ATTRIBUTE_FALLTHROUGH;
	case '?': usage();
	}
    }

  if (!do_dynamic && !do_syms && !do_reloc && !do_unwind && !do_sections
      && !do_segments && !do_header && !do_dump && !do_version
      && !do_histogram && !do_debugging && !do_arch && !do_notes
      && !do_section_groups)
    usage();
  else if (argc < 3)
    {
      warn(_("Nothing to do.\n"));
      usage();
    }
}

static const char *
get_elf_class(unsigned int elf_class)
{
  static char buff[32];

  switch (elf_class)
    {
    case ELFCLASSNONE: return _("none");
    case ELFCLASS32:   return "ELF32";
    case ELFCLASS64:   return "ELF64";
    default:
      snprintf(buff, sizeof(buff), _("<unknown: %x>"), elf_class);
      return buff;
    }
}

static const char *
get_data_encoding (unsigned int encoding)
{
  static char buff[32];

  switch (encoding)
    {
    case ELFDATANONE: return _("none");
    case ELFDATA2LSB: return _("2's complement, little endian");
    case ELFDATA2MSB: return _("2's complement, big endian");
    default:
      snprintf(buff, sizeof(buff), _("<unknown: %x>"), encoding);
      return buff;
    }
}

/* Decode the data held in 'elf_header'.  */

static int
process_file_header (void)
{
  if (   elf_header.e_ident[EI_MAG0] != ELFMAG0
      || elf_header.e_ident[EI_MAG1] != ELFMAG1
      || elf_header.e_ident[EI_MAG2] != ELFMAG2
      || elf_header.e_ident[EI_MAG3] != ELFMAG3)
    {
      error(_("Not an ELF file - it has the wrong magic bytes at the start\n"));
      return 0;
    }

  if (do_header)
    {
      int i;

      printf (_("ELF Header:\n"));
      printf (_("  Magic:   "));
      for (i = 0; i < EI_NIDENT; i++)
	printf ("%2.2x ", elf_header.e_ident[i]);
      printf ("\n");
      printf (_("  Class:                             %s\n"),
	      get_elf_class (elf_header.e_ident[EI_CLASS]));
      printf (_("  Data:                              %s\n"),
	      get_data_encoding (elf_header.e_ident[EI_DATA]));
      printf (_("  Version:                           %d %s\n"),
	      elf_header.e_ident[EI_VERSION],
	      (elf_header.e_ident[EI_VERSION] == EV_CURRENT
	       ? "(current)"
	       : (elf_header.e_ident[EI_VERSION] != EV_NONE
		  ? "<unknown: %lx>"
		  : "")));
      printf (_("  OS/ABI:                            %s\n"),
	      get_osabi_name (elf_header.e_ident[EI_OSABI]));
      printf (_("  ABI Version:                       %d\n"),
	      elf_header.e_ident[EI_ABIVERSION]);
      printf (_("  Type:                              %s\n"),
	      get_file_type (elf_header.e_type));
      printf (_("  Machine:                           %s\n"),
	      get_machine_name (elf_header.e_machine));
      printf (_("  Version:                           0x%lx\n"),
	      (unsigned long) elf_header.e_version);

      printf (_("  Entry point address:               "));
      print_vma ((bfd_vma) elf_header.e_entry, PREFIX_HEX);
      printf (_("\n  Start of program headers:          "));
      print_vma ((bfd_vma) elf_header.e_phoff, DEC);
      printf (_(" (bytes into file)\n  Start of section headers:          "));
      print_vma ((bfd_vma) elf_header.e_shoff, DEC);
      printf (_(" (bytes into file)\n"));

      printf (_("  Flags:                             0x%lx%s\n"),
	      (unsigned long) elf_header.e_flags,
	      get_machine_flags (elf_header.e_flags, elf_header.e_machine));
      printf (_("  Size of this header:               %ld (bytes)\n"),
	      (long) elf_header.e_ehsize);
      printf (_("  Size of program headers:           %ld (bytes)\n"),
	      (long) elf_header.e_phentsize);
      printf (_("  Number of program headers:         %ld\n"),
	      (long) elf_header.e_phnum);
      printf (_("  Size of section headers:           %ld (bytes)\n"),
	      (long) elf_header.e_shentsize);
      printf (_("  Number of section headers:         %ld"),
	      (long) elf_header.e_shnum);
      if (section_headers != NULL && elf_header.e_shnum == 0)
	printf (" (%ld)", (long) section_headers[0].sh_size);
      putc ('\n', stdout);
      printf (_("  Section header string table index: %ld"),
	      (long) elf_header.e_shstrndx);
      if (section_headers != NULL && elf_header.e_shstrndx == SHN_XINDEX)
	printf (" (%ld)", (long) section_headers[0].sh_link);
      putc ('\n', stdout);
    }

  if (section_headers != NULL)
    {
      if (elf_header.e_shnum == 0)
	elf_header.e_shnum = section_headers[0].sh_size;
      if (elf_header.e_shstrndx == SHN_XINDEX)
	elf_header.e_shstrndx = section_headers[0].sh_link;
      free (section_headers);
      section_headers = NULL;
    }

  return 1;
}


static int
get_32bit_program_headers(FILE *file,
                          Elf_Internal_Phdr *elf_program_headers)
{
  Elf32_External_Phdr *phdrs;
  Elf32_External_Phdr *external;
  Elf_Internal_Phdr *internal;
  unsigned int i;

  phdrs = (Elf32_External_Phdr *)get_data(NULL, file, elf_header.e_phoff,
                                          elf_header.e_phentsize,
                                          elf_header.e_phnum,
                                          _("program headers"));
  if (!phdrs) {
    return 0;
  }

  for (i = 0, internal = elf_program_headers, external = phdrs;
       i < elf_header.e_phnum;
       i++, internal++, external++)
    {
      internal->p_type = BYTE_GET(external->p_type);
      internal->p_offset = BYTE_GET(external->p_offset);
      internal->p_vaddr = BYTE_GET(external->p_vaddr);
      internal->p_paddr = BYTE_GET(external->p_paddr);
      internal->p_filesz = BYTE_GET(external->p_filesz);
      internal->p_memsz = BYTE_GET(external->p_memsz);
      internal->p_flags = BYTE_GET(external->p_flags);
      internal->p_align = BYTE_GET(external->p_align);
    }

  free(phdrs);

  return 1;
}

static int
get_64bit_program_headers(FILE *file, Elf_Internal_Phdr *myprogram_headers)
{
  Elf64_External_Phdr *phdrs;
  Elf64_External_Phdr *external;
  Elf_Internal_Phdr *internal;
  unsigned int i;

  phdrs = (Elf64_External_Phdr *)get_data(NULL, file, elf_header.e_phoff,
                                          elf_header.e_phentsize,
                                          elf_header.e_phnum,
                                          _("program headers"));
  if (!phdrs)
    return 0;

  for (i = 0, internal = myprogram_headers, external = phdrs;
       i < elf_header.e_phnum;
       i++, internal++, external++)
    {
      internal->p_type = BYTE_GET(external->p_type);
      internal->p_flags = BYTE_GET(external->p_flags);
      internal->p_offset = BYTE_GET(external->p_offset);
      internal->p_vaddr = BYTE_GET(external->p_vaddr);
      internal->p_paddr = BYTE_GET(external->p_paddr);
      internal->p_filesz = BYTE_GET(external->p_filesz);
      internal->p_memsz = BYTE_GET(external->p_memsz);
      internal->p_align = BYTE_GET(external->p_align);
    }

  free(phdrs);

  return 1;
}

/* Returns 1 if the program headers were read into `program_headers': */
static int
get_program_headers(FILE *file)
{
  Elf_Internal_Phdr *phdrs;

  /* Check cache of prior read: */
  if (program_headers != NULL) {
    return 1;
  }

  phdrs = ( Elf_Internal_Phdr *)cmalloc(elf_header.e_phnum,
                                        sizeof(Elf_Internal_Phdr));

  if (phdrs == NULL)
    {
      error(_("Out of memory\n"));
      return 0;
    }

  if (is_32bit_elf
      ? get_32bit_program_headers(file, phdrs)
      : get_64bit_program_headers(file, phdrs))
    {
      program_headers = phdrs;
      return 1;
    }

  free(phdrs);
  return 0;
}

/* Returns 1 if the program headers were loaded: */
static int
process_program_headers(FILE *file)
{
  Elf_Internal_Phdr *segment;
  unsigned int i;

  if (elf_header.e_phnum == 0)
    {
      if (do_segments)
	printf (_("\nThere are no program headers in this file.\n"));
      return 0;
    }

  if (do_segments && !do_header)
    {
      printf (_("\nElf file type is %s\n"), get_file_type (elf_header.e_type));
      printf (_("Entry point "));
      print_vma ((bfd_vma) elf_header.e_entry, PREFIX_HEX);
      printf (_("\nThere are %d program headers, starting at offset "),
	      elf_header.e_phnum);
      print_vma ((bfd_vma) elf_header.e_phoff, DEC);
      printf ("\n");
    }

  if (! get_program_headers (file))
      return 0;

  if (do_segments)
    {
      if (elf_header.e_phnum > 1)
	printf (_("\nProgram Headers:\n"));
      else
	printf (_("\nProgram Headers:\n"));

      if (is_32bit_elf)
	printf
	  (_("  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align\n"));
      else if (do_wide)
	printf
	  (_("  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align\n"));
      else
	{
	  printf
	    (_("  Type           Offset             VirtAddr           PhysAddr\n"));
	  printf
	    (_("                 FileSiz            MemSiz              Flags  Align\n"));
	}
    }

  dynamic_addr = 0;
  dynamic_size = 0;

  for (i = 0, segment = program_headers;
       i < elf_header.e_phnum;
       i++, segment++)
    {
      if (do_segments)
	{
	  printf ("  %-14.14s ", get_segment_type (segment->p_type));

	  if (is_32bit_elf)
	    {
	      printf ("0x%6.6lx ", (unsigned long) segment->p_offset);
	      printf ("0x%8.8lx ", (unsigned long) segment->p_vaddr);
	      printf ("0x%8.8lx ", (unsigned long) segment->p_paddr);
	      printf ("0x%5.5lx ", (unsigned long) segment->p_filesz);
	      printf ("0x%5.5lx ", (unsigned long) segment->p_memsz);
	      printf ("%c%c%c ",
		      (segment->p_flags & PF_R ? 'R' : ' '),
		      (segment->p_flags & PF_W ? 'W' : ' '),
		      (segment->p_flags & PF_X ? 'E' : ' '));
	      printf ("%#lx", (unsigned long) segment->p_align);
	    }
	  else if (do_wide)
	    {
	      if ((unsigned long) segment->p_offset == segment->p_offset)
		printf ("0x%6.6lx ", (unsigned long) segment->p_offset);
	      else
		{
		  print_vma (segment->p_offset, FULL_HEX);
		  putchar (' ');
		}

	      print_vma (segment->p_vaddr, FULL_HEX);
	      putchar (' ');
	      print_vma (segment->p_paddr, FULL_HEX);
	      putchar (' ');

	      if ((unsigned long) segment->p_filesz == segment->p_filesz)
		printf ("0x%6.6lx ", (unsigned long) segment->p_filesz);
	      else
		{
		  print_vma (segment->p_filesz, FULL_HEX);
		  putchar (' ');
		}

	      if ((unsigned long) segment->p_memsz == segment->p_memsz)
		printf ("0x%6.6lx", (unsigned long) segment->p_memsz);
	      else
		{
		  print_vma (segment->p_offset, FULL_HEX);
		}

	      printf (" %c%c%c ",
		      (segment->p_flags & PF_R ? 'R' : ' '),
		      (segment->p_flags & PF_W ? 'W' : ' '),
		      (segment->p_flags & PF_X ? 'E' : ' '));

	      if ((unsigned long) segment->p_align == segment->p_align)
		printf ("%#lx", (unsigned long) segment->p_align);
	      else
		{
		  print_vma (segment->p_align, PREFIX_HEX);
		}
	    }
	  else
	    {
	      print_vma (segment->p_offset, FULL_HEX);
	      putchar (' ');
	      print_vma (segment->p_vaddr, FULL_HEX);
	      putchar (' ');
	      print_vma (segment->p_paddr, FULL_HEX);
	      printf ("\n                 ");
	      print_vma (segment->p_filesz, FULL_HEX);
	      putchar (' ');
	      print_vma (segment->p_memsz, FULL_HEX);
	      printf ("  %c%c%c    ",
		      (segment->p_flags & PF_R ? 'R' : ' '),
		      (segment->p_flags & PF_W ? 'W' : ' '),
		      (segment->p_flags & PF_X ? 'E' : ' '));
	      print_vma (segment->p_align, HEX);
	    }
	}

      switch (segment->p_type)
	{
	case PT_DYNAMIC:
	  if (dynamic_addr)
	    error (_("more than one dynamic segment\n"));

	  /* Try to locate the .dynamic section. If there is
	     a section header table, we can easily locate it.  */
	  if (section_headers != NULL)
	    {
	      Elf_Internal_Shdr *sec;

	      sec = find_section (".dynamic");
	      if (sec == NULL || sec->sh_size == 0)
		{
		  error (_("no .dynamic section in the dynamic segment"));
		  break;
		}

	      dynamic_addr = sec->sh_offset;
	      dynamic_size = sec->sh_size;

	      if (dynamic_addr < segment->p_offset
		  || dynamic_addr > segment->p_offset + segment->p_filesz)
		warn (_("the .dynamic section is not contained within the dynamic segment"));
	      else if (dynamic_addr > segment->p_offset)
		warn (_("the .dynamic section is not the first section in the dynamic segment."));
	    }
	  else
	    {
	      /* Otherwise, we can only assume that the .dynamic
		 section is the first section in the DYNAMIC segment.  */
	      dynamic_addr = segment->p_offset;
	      dynamic_size = segment->p_filesz;
	    }
	  break;

	case PT_INTERP:
	  if (fseek (file, archive_file_offset + (long) segment->p_offset,
		     SEEK_SET))
	    error (_("Unable to find program interpreter name\n"));
	  else
	    {
	      program_interpreter[0] = 0;
	      fscanf (file, "%63s", program_interpreter);

	      if (do_segments)
		printf (_("\n      [Requesting program interpreter: %s]"),
		    program_interpreter);
	    }
	  break;

	default:
	  break;
	}

      if (do_segments)
	putc ('\n', stdout);
    }

  if (do_segments && section_headers != NULL && string_table != NULL)
    {
      printf (_("\n Section to Segment mapping:\n"));
      printf (_("  Segment Sections...\n"));

      for (i = 0; i < elf_header.e_phnum; i++)
	{
	  unsigned int j;
	  Elf_Internal_Shdr *section;

	  segment = program_headers + i;
	  section = section_headers;

	  printf ("   %2.2d     ", i);

	  for (j = 1; j < elf_header.e_shnum; j++, section++)
	    {
	      if (section->sh_size > 0
		  /* Compare allocated sections by VMA, unallocated
		     sections by file offset.  */
		  && (section->sh_flags & SHF_ALLOC
		      ? (section->sh_addr >= segment->p_vaddr
			 && section->sh_addr + section->sh_size
			 <= segment->p_vaddr + segment->p_memsz)
		      : ((bfd_vma) section->sh_offset >= segment->p_offset
			 && (section->sh_offset + section->sh_size
			     <= segment->p_offset + segment->p_filesz)))
		  /* .tbss is special.  It doesn't contribute memory space
		     to normal segments.  */
		  && (!((section->sh_flags & SHF_TLS) != 0
			&& section->sh_type == SHT_NOBITS)
		      || segment->p_type == PT_TLS))
		printf ("%s ", SECTION_NAME (section));
	    }

	  putc ('\n',stdout);
	}
    }

  return 1;
}


/* Find the file offset corresponding to VMA by using the program headers.  */

static long
offset_from_vma (FILE *file, bfd_vma vma, bfd_size_type size)
{
  Elf_Internal_Phdr *seg;

  if (! get_program_headers (file))
    {
      warn (_("Cannot interpret virtual addresses without program headers.\n"));
      return (long) vma;
    }

  for (seg = program_headers;
       seg < program_headers + elf_header.e_phnum;
       ++seg)
    {
      if (seg->p_type != PT_LOAD)
	continue;

      if (vma >= (seg->p_vaddr & -seg->p_align)
	  && vma + size <= seg->p_vaddr + seg->p_filesz)
	return vma - seg->p_vaddr + seg->p_offset;
    }

  warn (_("Virtual address 0x%lx not located in any PT_LOAD segment.\n"),
	(long)vma);
  return (long)vma;
}


static int
get_32bit_section_headers(FILE *file, unsigned int num)
{
  Elf32_External_Shdr *shdrs;
  Elf_Internal_Shdr *internal;
  unsigned int i;

  shdrs = (Elf32_External_Shdr *)get_data(NULL, file, elf_header.e_shoff,
                                          elf_header.e_shentsize, num,
                                          _("section headers"));
  if (!shdrs) {
    return 0;
  }

  section_headers = (Elf_Internal_Shdr *)cmalloc(num, sizeof(Elf_Internal_Shdr));

  if (section_headers == NULL)
    {
      error(_("Out of memory\n"));
      return 0;
    }

  for (i = 0, internal = section_headers;
       i < num;
       i++, internal++)
    {
      internal->sh_name = BYTE_GET(shdrs[i].sh_name);
      internal->sh_type = BYTE_GET(shdrs[i].sh_type);
      internal->sh_flags = BYTE_GET(shdrs[i].sh_flags);
      internal->sh_addr = BYTE_GET(shdrs[i].sh_addr);
      internal->sh_offset = BYTE_GET(shdrs[i].sh_offset);
      internal->sh_size  = BYTE_GET(shdrs[i].sh_size);
      internal->sh_link = BYTE_GET(shdrs[i].sh_link);
      internal->sh_info = BYTE_GET(shdrs[i].sh_info);
      internal->sh_addralign = BYTE_GET(shdrs[i].sh_addralign);
      internal->sh_entsize = BYTE_GET(shdrs[i].sh_entsize);
    }

  free(shdrs);

  return 1;
}

static int
get_64bit_section_headers(FILE *file, unsigned int num)
{
  Elf64_External_Shdr *shdrs;
  Elf_Internal_Shdr *internal;
  unsigned int i;

  shdrs = (Elf64_External_Shdr *)get_data(NULL, file, elf_header.e_shoff,
                                          elf_header.e_shentsize, num,
                                          _("section headers"));
  if (!shdrs) {
    return 0;
  }

  section_headers = (Elf_Internal_Shdr *)cmalloc(num, sizeof(Elf_Internal_Shdr));

  if (section_headers == NULL)
    {
      error(_("Out of memory\n"));
      return 0;
    }

  for (i = 0, internal = section_headers;
       i < num;
       i++, internal++)
    {
      internal->sh_name = BYTE_GET(shdrs[i].sh_name);
      internal->sh_type = BYTE_GET(shdrs[i].sh_type);
      internal->sh_flags = BYTE_GET(shdrs[i].sh_flags);
      internal->sh_addr = BYTE_GET(shdrs[i].sh_addr);
      internal->sh_size = BYTE_GET(shdrs[i].sh_size);
      internal->sh_entsize = BYTE_GET(shdrs[i].sh_entsize);
      internal->sh_link = BYTE_GET(shdrs[i].sh_link);
      internal->sh_info = BYTE_GET(shdrs[i].sh_info);
      internal->sh_offset = BYTE_GET(shdrs[i].sh_offset);
      internal->sh_addralign = BYTE_GET(shdrs[i].sh_addralign);
    }

  free(shdrs);

  return 1;
}

static Elf_Internal_Sym *
get_32bit_elf_symbols(FILE *file, Elf_Internal_Shdr *section)
{
  unsigned long number;
  Elf32_External_Sym *esyms;
  Elf_External_Sym_Shndx *shndx;
  Elf_Internal_Sym *isyms;
  Elf_Internal_Sym *psym;
  unsigned int j;

  esyms = (Elf32_External_Sym *)get_data(NULL, file, section->sh_offset, 1,
                                         section->sh_size, _("symbols"));
  if (!esyms) {
    return NULL;
  }

  shndx = (Elf_External_Sym_Shndx *)NULL;
  if ((symtab_shndx_hdr != NULL)
      && (symtab_shndx_hdr->sh_link
	  == (unsigned long)SECTION_HEADER_NUM(section - section_headers)))
    {
      shndx = (Elf_External_Sym_Shndx *)get_data(NULL, file,
                                                 symtab_shndx_hdr->sh_offset,
                                                 1, symtab_shndx_hdr->sh_size,
                                                 _("symtab shndx"));
      if (!shndx)
	{
	  free(esyms);
	  return NULL;
	}
    }

  number = (section->sh_size / section->sh_entsize);
  isyms = (Elf_Internal_Sym *)cmalloc(number, sizeof(Elf_Internal_Sym));

  if (isyms == NULL)
    {
      error(_("Out of memory\n"));
      if (shndx) {
	free(shndx);
      }
      free(esyms);
      return NULL;
    }

  for (j = 0, psym = isyms;
       j < number;
       j++, psym++)
    {
      psym->st_name  = BYTE_GET (esyms[j].st_name);
      psym->st_value = BYTE_GET (esyms[j].st_value);
      psym->st_size  = BYTE_GET (esyms[j].st_size);
      psym->st_shndx = BYTE_GET (esyms[j].st_shndx);
      if (psym->st_shndx == SHN_XINDEX && shndx != NULL)
	psym->st_shndx
	  = byte_get ((unsigned char *) &shndx[j], sizeof (shndx[j]));
      psym->st_info  = BYTE_GET (esyms[j].st_info);
      psym->st_other = BYTE_GET (esyms[j].st_other);
    }

  if (shndx)
    free (shndx);
  free (esyms);

  return isyms;
}

static Elf_Internal_Sym *
get_64bit_elf_symbols(FILE *file, Elf_Internal_Shdr *section)
{
  unsigned long number;
  Elf64_External_Sym *esyms;
  Elf_External_Sym_Shndx *shndx;
  Elf_Internal_Sym *isyms;
  Elf_Internal_Sym *psym;
  unsigned int j;

  esyms = (Elf64_External_Sym *)get_data(NULL, file, section->sh_offset, 1,
                                         section->sh_size, _("symbols"));
  if (!esyms) {
    return NULL;
  }

  shndx = (Elf_External_Sym_Shndx *)NULL;
  if ((symtab_shndx_hdr != NULL)
      && (symtab_shndx_hdr->sh_link
	  == (unsigned long)SECTION_HEADER_NUM(section - section_headers)))
    {
      shndx = (Elf_External_Sym_Shndx *)get_data(NULL, file,
                                                 symtab_shndx_hdr->sh_offset,
                                                 1, symtab_shndx_hdr->sh_size,
                                                 _("symtab shndx"));
      if (!shndx)
	{
	  free(esyms);
	  return NULL;
	}
    }

  number = (section->sh_size / section->sh_entsize);
  isyms = (Elf_Internal_Sym *)cmalloc(number, sizeof(Elf_Internal_Sym));

  if (isyms == NULL)
    {
      error(_("Out of memory\n"));
      if (shndx) {
	free(shndx);
      }
      free(esyms);
      return NULL;
    }

  for (j = 0, psym = isyms;
       j < number;
       j++, psym++)
    {
      psym->st_name  = BYTE_GET (esyms[j].st_name);
      psym->st_info  = BYTE_GET (esyms[j].st_info);
      psym->st_other = BYTE_GET (esyms[j].st_other);
      psym->st_shndx = BYTE_GET (esyms[j].st_shndx);
      if (psym->st_shndx == SHN_XINDEX && shndx != NULL)
	psym->st_shndx
	  = byte_get ((unsigned char *) &shndx[j], sizeof (shndx[j]));
      psym->st_value = BYTE_GET (esyms[j].st_value);
      psym->st_size  = BYTE_GET (esyms[j].st_size);
    }

  if (shndx)
    free (shndx);
  free (esyms);

  return isyms;
}

static const char *
get_elf_section_flags (bfd_vma sh_flags)
{
  static char buff[1024];
  char *p = buff;
  int iindex;
  size_t size = (sizeof(buff) - (8UL + 4UL + 1UL));
  const struct
    {
      const char *str;
      int len;
    }
  flags[] =
    {
	{ "WRITE", 5 },
	{ "ALLOC", 5 },
	{ "EXEC", 4 },
	{ "MERGE", 5 },
	{ "STRINGS", 7 },
	{ "INFO LINK", 9 },
	{ "LINK ORDER", 10 },
	{ "OS NONCONF", 10 },
	{ "GROUP", 5 },
	{ "TLS", 3 }
    };

  if (do_section_details)
    {
      snprintf(buff, sizeof(buff), "[%8.8lx]: ", (unsigned long)sh_flags);
      p += 8 + 4;
    }

  while (sh_flags)
    {
      bfd_vma flag;

      flag = sh_flags & -sh_flags;
      sh_flags &= ~flag;

      if (do_section_details)
	{
	  switch (flag)
	    {
	    case SHF_WRITE:		iindex = 0; break;
	    case SHF_ALLOC:		iindex = 1; break;
	    case SHF_EXECINSTR:		iindex = 2; break;
	    case SHF_MERGE:		iindex = 3; break;
	    case SHF_STRINGS:		iindex = 4; break;
	    case SHF_INFO_LINK:		iindex = 5; break;
	    case SHF_LINK_ORDER:	iindex = 6; break;
	    case SHF_OS_NONCONFORMING:	iindex = 7; break;
	    case SHF_GROUP:		iindex = 8; break;
	    case SHF_TLS:		iindex = 9; break;

	    default:
	      iindex = -1;
	      break;
	    }

	  if (p != buff + 8 + 4)
	    {
	      if (size < 10 + 2)
		abort ();
	      size -= 2;
	      *p++ = ',';
	      *p++ = ' ';
	    }

	  if (iindex != -1)
	    {
	      size -= flags [iindex].len;
	      p = stpcpy (p, flags [iindex].str);
	    }
	  else if (flag & SHF_MASKOS)
	    {
	      size -= 5 + 8;
	      sprintf (p, "OS (%8.8lx)", (unsigned long) flag);
	      p += 5 + 8;
	    }
	  else if (flag & SHF_MASKPROC)
	    {
	      size -= 7 + 8;
	      sprintf (p, "PROC (%8.8lx)", (unsigned long) flag);
	      p += 7 + 8;
	    }
	  else
	    {
	      size -= 10 + 8;
	      sprintf (p, "UNKNOWN (%8.8lx)", (unsigned long) flag);
	      p += 10 + 8;
	    }
	}
      else
	{
	  switch (flag)
	    {
	    case SHF_WRITE:		*p = 'W'; break;
	    case SHF_ALLOC:		*p = 'A'; break;
	    case SHF_EXECINSTR:		*p = 'X'; break;
	    case SHF_MERGE:		*p = 'M'; break;
	    case SHF_STRINGS:		*p = 'S'; break;
	    case SHF_INFO_LINK:		*p = 'I'; break;
	    case SHF_LINK_ORDER:	*p = 'L'; break;
	    case SHF_OS_NONCONFORMING:	*p = 'O'; break;
	    case SHF_GROUP:		*p = 'G'; break;
	    case SHF_TLS:		*p = 'T'; break;

	    default:
	      if (elf_header.e_machine == EM_X86_64
		  && flag == SHF_X86_64_LARGE)
		*p = 'l';
	      else if (flag & SHF_MASKOS)
		{
		  *p = 'o';
		  sh_flags &= ~ SHF_MASKOS;
		}
	      else if (flag & SHF_MASKPROC)
		{
		  *p = 'p';
		  sh_flags &= ~ SHF_MASKPROC;
		}
	      else
		*p = 'x';
	      break;
	    }
	  p++;
	}
    }

  *p = '\0';
  return buff;
}

static int
process_section_headers (FILE *file)
{
  Elf_Internal_Shdr *section;
  unsigned int i;

  section_headers = NULL;

  if (elf_header.e_shnum == 0)
    {
      if (do_sections)
	printf (_("\nThere are no sections in this file.\n"));

      return 1;
    }

  if (do_sections && !do_header)
    printf (_("There are %d section headers, starting at offset 0x%lx:\n"),
	    elf_header.e_shnum, (unsigned long) elf_header.e_shoff);

  if (is_32bit_elf)
    {
      if (! get_32bit_section_headers (file, elf_header.e_shnum))
	return 0;
    }
  else if (! get_64bit_section_headers (file, elf_header.e_shnum))
    return 0;

  /* Read in the string table, so that we have names to display.  */
  if (SECTION_HEADER_INDEX(elf_header.e_shstrndx) < elf_header.e_shnum)
    {
      section = SECTION_HEADER(elf_header.e_shstrndx);

      if (section->sh_size != 0)
	{
	  string_table = (char *)get_data(NULL, file, section->sh_offset,
                                          1, section->sh_size,
                                          _("string table"));

	  string_table_length = ((string_table != NULL) ? section->sh_size : 0);
	}
    }

  /* Scan the sections for the dynamic symbol table
     and dynamic string table and debug sections.  */
  dynamic_symbols = NULL;
  dynamic_strings = NULL;
  dynamic_syminfo = NULL;
  symtab_shndx_hdr = NULL;

  eh_addr_size = is_32bit_elf ? 4 : 8;
  switch (elf_header.e_machine)
    {
    case EM_MIPS:
    case EM_MIPS_RS3_LE:
      /* The 64-bit MIPS EABI uses a combination of 32-bit ELF and 64-bit
	 FDE addresses.  However, the ABI also has a semi-official ILP32
	 variant for which the normal FDE address size rules apply.

	 GCC 4.0 marks EABI64 objects with a dummy .gcc_compiled_longXX
	 section, where XX is the size of longs in bits.  Unfortunately,
	 earlier compilers provided no way of distinguishing ILP32 objects
	 from LP64 objects, so if there is any doubt, we should assume that
	 the official LP64 form is being used.  */
      if (((elf_header.e_flags & EF_MIPS_ABI) == E_MIPS_ABI_EABI64)
	  && (find_section(".gcc_compiled_long32") == NULL))
	eh_addr_size = 8;
      break;
    default:
      break;
    }

#define CHECK_ENTSIZE_VALUES(section, i, size32, size64) \
  do									    \
    {									    \
      size_t expected_entsize						    \
	= is_32bit_elf ? size32 : size64;				    \
      if (section->sh_entsize != expected_entsize)			    \
	error (_("Section %d has invalid sh_entsize %lx (expected %lx)\n"), \
	       i, (unsigned long int) section->sh_entsize,		    \
	       (unsigned long int) expected_entsize);			    \
      section->sh_entsize = expected_entsize;				    \
    }									    \
  while (0)
#define CHECK_ENTSIZE(section, i, type) \
  CHECK_ENTSIZE_VALUES (section, i, sizeof (Elf32_External_##type),	    \
			sizeof (Elf64_External_##type))

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      const char *name = SECTION_NAME(section);

      if (section->sh_type == SHT_DYNSYM)
	{
	  if (dynamic_symbols != NULL)
	    {
	      error (_("File contains multiple dynamic symbol tables\n"));
	      continue;
	    }

	  CHECK_ENTSIZE (section, i, Sym);
	  num_dynamic_syms = section->sh_size / section->sh_entsize;
	  dynamic_symbols = GET_ELF_SYMBOLS (file, section);
	}
      else if (section->sh_type == SHT_STRTAB
	       && streq (name, ".dynstr"))
	{
	  if (dynamic_strings != NULL)
	    {
	      error (_("File contains multiple dynamic string tables\n"));
	      continue;
	    }

	  dynamic_strings = (char *)get_data(NULL, file,
                                             section->sh_offset,
                                             1, section->sh_size,
                                             _("dynamic strings"));
	  dynamic_strings_length = section->sh_size;
	}
      else if (section->sh_type == SHT_SYMTAB_SHNDX)
	{
	  if (symtab_shndx_hdr != NULL)
	    {
	      error (_("File contains multiple symtab shndx tables\n"));
	      continue;
	    }
	  symtab_shndx_hdr = section;
	}
      else if (section->sh_type == SHT_SYMTAB)
	CHECK_ENTSIZE (section, i, Sym);
      else if (section->sh_type == SHT_GROUP)
	CHECK_ENTSIZE_VALUES (section, i, GRP_ENTRY_SIZE, GRP_ENTRY_SIZE);
      else if (section->sh_type == SHT_REL)
	CHECK_ENTSIZE (section, i, Rel);
      else if (section->sh_type == SHT_RELA)
	CHECK_ENTSIZE (section, i, Rela);
      else if ((do_debugging || do_debug_info || do_debug_abbrevs
		|| do_debug_lines || do_debug_pubnames || do_debug_aranges
		|| do_debug_frames || do_debug_macinfo || do_debug_str
		|| do_debug_loc || do_debug_ranges)
	       && strneq (name, ".debug_", 7))
	{
	  name += 7;

	  if (do_debugging
	      || (do_debug_info     && streq (name, "info"))
	      || (do_debug_abbrevs  && streq (name, "abbrev"))
	      || (do_debug_lines    && streq (name, "line"))
	      || (do_debug_pubnames && streq (name, "pubnames"))
	      || (do_debug_aranges  && streq (name, "aranges"))
	      || (do_debug_ranges   && streq (name, "ranges"))
	      || (do_debug_frames   && streq (name, "frame"))
	      || (do_debug_macinfo  && streq (name, "macinfo"))
	      || (do_debug_str      && streq (name, "str"))
	      || (do_debug_loc      && streq (name, "loc"))
	      )
	    request_dump (i, DEBUG_DUMP);
	}
      /* linkonce section to be combined with .debug_info at link time.  */
      else if ((do_debugging || do_debug_info)
	       && strneq (name, ".gnu.linkonce.wi.", 17))
	request_dump (i, DEBUG_DUMP);
      else if (do_debug_frames && streq (name, ".eh_frame"))
	request_dump (i, DEBUG_DUMP);
    }

  if (! do_sections)
    return 1;

  if (elf_header.e_shnum > 1)
    printf (_("\nSection Headers:\n"));
  else
    printf (_("\nSection Header:\n"));

  if (is_32bit_elf)
    {
      if (do_section_details)
	{
	  printf (_("  [Nr] Name\n"));
	  printf (_("       Type            Addr     Off    Size   ES   Lk Inf Al\n"));
	}
      else
	printf
	  (_("  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al\n"));
    }
  else if (do_wide)
    {
      if (do_section_details)
	{
	  printf (_("  [Nr] Name\n"));
	  printf (_("       Type            Address          Off    Size   ES   Lk Inf Al\n"));
	}
      else
	printf
	  (_("  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al\n"));
    }
  else
    {
      if (do_section_details)
	{
	  printf (_("  [Nr] Name\n"));
	  printf (_("       Type              Address          Offset            Link\n"));
	  printf (_("       Size              EntSize          Info              Align\n"));
	}
      else
	{
	  printf (_("  [Nr] Name              Type             Address           Offset\n"));
	  printf (_("       Size              EntSize          Flags  Link  Info  Align\n"));
	}
    }

  if (do_section_details)
    printf (_("       Flags\n"));

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      if (do_section_details)
	{
	  printf ("  [%2u] %s\n",
		  SECTION_HEADER_NUM (i),
		  SECTION_NAME (section));
	  if (is_32bit_elf || do_wide)
	    printf ("       %-15.15s ",
		    get_section_type_name (section->sh_type));
	}
      else
	printf ("  [%2u] %-17.17s %-15.15s ",
		SECTION_HEADER_NUM (i),
		SECTION_NAME (section),
		get_section_type_name (section->sh_type));

      if (is_32bit_elf)
	{
	  print_vma (section->sh_addr, LONG_HEX);

	  printf ( " %6.6lx %6.6lx %2.2lx",
		   (unsigned long) section->sh_offset,
		   (unsigned long) section->sh_size,
		   (unsigned long) section->sh_entsize);

	  if (do_section_details)
	    fputs ("  ", stdout);
	  else
	    printf (" %3s ", get_elf_section_flags (section->sh_flags));

	  printf ("%2ld %3lu %2ld\n",
		  (unsigned long) section->sh_link,
		  (unsigned long) section->sh_info,
		  (unsigned long) section->sh_addralign);
	}
      else if (do_wide)
	{
	  print_vma (section->sh_addr, LONG_HEX);

	  if ((long) section->sh_offset == section->sh_offset)
	    printf (" %6.6lx", (unsigned long) section->sh_offset);
	  else
	    {
	      putchar (' ');
	      print_vma (section->sh_offset, LONG_HEX);
	    }

	  if ((unsigned long) section->sh_size == section->sh_size)
	    printf (" %6.6lx", (unsigned long) section->sh_size);
	  else
	    {
	      putchar (' ');
	      print_vma (section->sh_size, LONG_HEX);
	    }

	  if ((unsigned long) section->sh_entsize == section->sh_entsize)
	    printf (" %2.2lx", (unsigned long) section->sh_entsize);
	  else
	    {
	      putchar (' ');
	      print_vma (section->sh_entsize, LONG_HEX);
	    }

	  if (do_section_details)
	    fputs ("  ", stdout);
	  else
	    printf (" %3s ", get_elf_section_flags (section->sh_flags));

	  printf ("%2ld %3lu ",
		  (unsigned long) section->sh_link,
		  (unsigned long) section->sh_info);

	  if ((unsigned long) section->sh_addralign == section->sh_addralign)
	    printf ("%2ld\n", (unsigned long) section->sh_addralign);
	  else
	    {
	      print_vma (section->sh_addralign, DEC);
	      putchar ('\n');
	    }
	}
      else if (do_section_details)
	{
	  printf ("       %-15.15s  ",
		  get_section_type_name (section->sh_type));
	  print_vma (section->sh_addr, LONG_HEX);
	  if ((long) section->sh_offset == section->sh_offset)
	    printf ("  %16.16lx", (unsigned long) section->sh_offset);
	  else
	    {
	      printf ("  ");
	      print_vma (section->sh_offset, LONG_HEX);
	    }
	  printf ("  %ld\n       ", (unsigned long) section->sh_link);
	  print_vma (section->sh_size, LONG_HEX);
	  putchar (' ');
	  print_vma (section->sh_entsize, LONG_HEX);

	  printf ("  %-16lu  %ld\n",
		  (unsigned long) section->sh_info,
		  (unsigned long) section->sh_addralign);
	}
      else
	{
	  putchar (' ');
	  print_vma (section->sh_addr, LONG_HEX);
	  if ((long) section->sh_offset == section->sh_offset)
	    printf ("  %8.8lx", (unsigned long) section->sh_offset);
	  else
	    {
	      printf ("  ");
	      print_vma (section->sh_offset, LONG_HEX);
	    }
	  printf ("\n       ");
	  print_vma (section->sh_size, LONG_HEX);
	  printf ("  ");
	  print_vma (section->sh_entsize, LONG_HEX);

	  printf (" %3s ", get_elf_section_flags (section->sh_flags));

	  printf ("     %2ld   %3lu     %ld\n",
		  (unsigned long) section->sh_link,
		  (unsigned long) section->sh_info,
		  (unsigned long) section->sh_addralign);
	}

      if (do_section_details)
	printf ("       %s\n", get_elf_section_flags (section->sh_flags));
    }

  if (!do_section_details)
    printf (_("Key to Flags:\n\
  W (write), A (alloc), X (execute), M (merge), S (strings)\n\
  I (info), L (link order), G (group), x (unknown)\n\
  O (extra OS processing required) o (OS specific), p (processor specific)\n"));

  return 1;
}

static const char *
get_group_flags (unsigned int flags)
{
  static char buff[32];
  switch (flags)
    {
    case GRP_COMDAT:
      return "COMDAT";

   default:
      snprintf (buff, sizeof (buff), _("[<unknown>: 0x%x]"), flags);
      break;
    }
  return buff;
}

static int
process_section_groups (FILE *file)
{
  Elf_Internal_Shdr *section;
  unsigned int i;
  struct group *group;
  Elf_Internal_Shdr *symtab_sec, *strtab_sec;
  Elf_Internal_Sym *symtab;
  char *strtab;
  size_t strtab_size;

  /* Do NOT process section groups unless needed.  */
  if (!do_unwind && !do_section_groups)
    return 1;

  if (elf_header.e_shnum == 0)
    {
      if (do_section_groups)
	printf(_("\nThere are no sections in this file.\n"));

      return 1;
    }

  if (section_headers == NULL)
    {
      error(_("Section headers are not available!\n"));
      abort();
    }

  section_headers_groups = (struct group **)calloc(elf_header.e_shnum,
                                                   sizeof(struct group *));

  if (section_headers_groups == NULL)
    {
      error(_("Out of memory\n"));
      return 0;
    }

  /* Scan the sections for the group section.  */
  group_count = 0;
  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    if (section->sh_type == SHT_GROUP)
      group_count++;

  if (group_count == 0)
    {
      if (do_section_groups)
	printf(_("\nThere are no section groups in this file.\n"));

      return 1;
    }

  section_groups = (struct group *)calloc(group_count,
                                          sizeof(struct group));

  if (section_groups == NULL)
    {
      error(_("Out of memory\n"));
      return 0;
    }

  symtab_sec = NULL;
  strtab_sec = NULL;
  symtab = NULL;
  strtab = NULL;
  strtab_size = 0;
  for (i = 0, section = section_headers, group = section_groups;
       i < elf_header.e_shnum;
       i++, section++)
    {
      if (section->sh_type == SHT_GROUP)
	{
	  const char *name = SECTION_NAME(section);
	  const char *group_name;
	  unsigned char *start, *indices;
	  unsigned int entry, j, size;
	  Elf_Internal_Shdr *sec;
	  Elf_Internal_Sym *sym;

	  /* Get the symbol table.  */
	  if (SECTION_HEADER_INDEX (section->sh_link) >= elf_header.e_shnum
	      || ((sec = SECTION_HEADER (section->sh_link))->sh_type
		  != SHT_SYMTAB))
	    {
	      error (_("Bad sh_link in group section `%s'\n"), name);
	      continue;
	    }

	  if (symtab_sec != sec)
	    {
	      symtab_sec = sec;
	      if (symtab)
		free (symtab);
	      symtab = GET_ELF_SYMBOLS (file, symtab_sec);
	    }

	  sym = symtab + section->sh_info;

	  if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    {
	      bfd_vma sec_index = SECTION_HEADER_INDEX (sym->st_shndx);
	      if (sec_index == 0)
		{
		  error (_("Bad sh_info in group section `%s'\n"), name);
		  continue;
		}

	      group_name = SECTION_NAME (section_headers + sec_index);
	      strtab_sec = NULL;
	      if (strtab)
		free (strtab);
	      strtab = NULL;
	      strtab_size = 0;
	    }
	  else
	    {
	      /* Get the string table.  */
	      if (SECTION_HEADER_INDEX (symtab_sec->sh_link)
		  >= elf_header.e_shnum)
		{
		  strtab_sec = NULL;
		  if (strtab)
		    free (strtab);
		  strtab = NULL;
		  strtab_size = 0;
		}
	      else if (strtab_sec
		       != (sec = SECTION_HEADER(symtab_sec->sh_link)))
		{
		  strtab_sec = sec;
		  if (strtab)
		    free(strtab);
		  strtab = (char *)get_data(NULL, file,
                                            strtab_sec->sh_offset, 1,
                                            strtab_sec->sh_size,
                                            _("string table"));
		  strtab_size = ((strtab != NULL) ? strtab_sec->sh_size : 0);
		}
	      group_name = ((sym->st_name < strtab_size)
                            ? (strtab + sym->st_name) : "<corrupt>");
	    }

	  start = (unsigned char *)get_data(NULL, file, section->sh_offset,
                                            1, section->sh_size,
                                            _("section data"));

	  indices = start;
	  size = (section->sh_size / section->sh_entsize) - 1;
	  entry = byte_get(indices, 4);
	  indices += 4;

	  if (do_section_groups)
	    {
	      printf("\n%s group section [%5u] `%s' [%s] contains %u sections:\n",
                     get_group_flags(entry), i, name, group_name, size);

	      printf(_("   [Index]    Name\n"));
	    }

	  group->group_index = i;

	  for (j = 0; j < size; j++)
	    {
	      struct group_list *g;

	      entry = byte_get (indices, 4);
	      indices += 4;

	      if (SECTION_HEADER_INDEX (entry) >= elf_header.e_shnum)
		{
		  error (_("section [%5u] in group section [%5u] > maximum section [%5u]\n"),
			 entry, i, elf_header.e_shnum - 1);
		  continue;
		}
	      else if (entry >= SHN_LORESERVE && entry <= SHN_HIRESERVE)
		{
		  error (_("invalid section [%5u] in group section [%5u]\n"),
			 entry, i);
		  continue;
		}

	      if (section_headers_groups [SECTION_HEADER_INDEX (entry)]
		  != NULL)
		{
		  if (entry)
		    {
		      error (_("section [%5u] in group section [%5u] already in group section [%5u]\n"),
			     entry, i,
			     section_headers_groups [SECTION_HEADER_INDEX (entry)]->group_index);
		      continue;
		    }
		  else
		    {
		      /* Intel C/C++ compiler may put section 0 in a
			 section group. We just warn it the first time
			 and ignore it afterwards.  */
		      static int warned = 0;
		      if (!warned)
			{
			  error(_("section 0 in group section [%5u]\n"),
                                section_headers_groups[SECTION_HEADER_INDEX(entry)]->group_index);
			  warned++;
			}
		    }
		}

	      section_headers_groups[SECTION_HEADER_INDEX(entry)]
		= group;

	      if (do_section_groups)
		{
		  sec = SECTION_HEADER(entry);
		  printf("   [%5u]   %s\n", entry, SECTION_NAME(sec));
		}

	      g = (struct group_list *)xmalloc(sizeof(struct group_list));
	      g->section_index = entry;
	      g->next = group->root;
	      group->root = g;
	    }

	  if (start)
	    free (start);

	  group++;
	}
    }

  if (symtab)
    free (symtab);
  if (strtab)
    free (strtab);
  return 1;
}

static struct
{
  const char *name;
  int reloc;
  int size;
  int rela;
} dynamic_relocations [] =
{
    { "REL", DT_REL, DT_RELSZ, FALSE },
    { "RELA", DT_RELA, DT_RELASZ, TRUE },
    { "PLT", DT_JMPREL, DT_PLTRELSZ, UNKNOWN }
};

/* Process the reloc section.  */

static int
process_relocs (FILE *file)
{
  unsigned long rel_size;
  unsigned long rel_offset;


  if (!do_reloc)
    return 1;

  if (do_using_dynamic)
    {
      int is_rela;
      const char *name;
      int has_dynamic_reloc;
      unsigned int i;

      has_dynamic_reloc = 0;

      for (i = 0; i < ARRAY_SIZE (dynamic_relocations); i++)
	{
	  is_rela = dynamic_relocations [i].rela;
	  name = dynamic_relocations [i].name;
	  rel_size = dynamic_info [dynamic_relocations [i].size];
	  rel_offset = dynamic_info [dynamic_relocations [i].reloc];

	  has_dynamic_reloc |= rel_size;

	  if (is_rela == UNKNOWN)
	    {
	      if (dynamic_relocations [i].reloc == DT_JMPREL)
		switch (dynamic_info[DT_PLTREL])
		  {
		  case DT_REL:
		    is_rela = FALSE;
		    break;
		  case DT_RELA:
		    is_rela = TRUE;
		    break;
                  default:
                    break;
		  }
	    }

	  if (rel_size)
	    {
	      printf
		(_("\n'%s' relocation section at offset 0x%lx contains %ld bytes:\n"),
		 name, rel_offset, rel_size);

	      dump_relocations (file,
				offset_from_vma (file, rel_offset, rel_size),
				rel_size,
				dynamic_symbols, num_dynamic_syms,
				dynamic_strings, dynamic_strings_length, is_rela);
	    }
	}

      if (! has_dynamic_reloc)
	printf (_("\nThere are no dynamic relocations in this file.\n"));
    }
  else
    {
      Elf_Internal_Shdr *section;
      unsigned long i;
      int found = 0;

      for (i = 0, section = section_headers;
	   i < elf_header.e_shnum;
	   i++, section++)
	{
	  if (   section->sh_type != SHT_RELA
	      && section->sh_type != SHT_REL)
	    continue;

	  rel_offset = section->sh_offset;
	  rel_size   = section->sh_size;

	  if (rel_size)
	    {
	      Elf_Internal_Shdr *strsec;
	      int is_rela;

	      printf (_("\nRelocation section "));

	      if (string_table == NULL)
		printf ("%d", section->sh_name);
	      else
		printf (_("'%s'"), SECTION_NAME (section));

	      printf (_(" at offset 0x%lx contains %lu entries:\n"),
		 rel_offset, (unsigned long) (rel_size / section->sh_entsize));

	      is_rela = section->sh_type == SHT_RELA;

	      if (section->sh_link
		  && SECTION_HEADER_INDEX (section->sh_link)
		     < elf_header.e_shnum)
		{
		  Elf_Internal_Shdr *symsec;
		  Elf_Internal_Sym *symtab;
		  unsigned long nsyms;
		  unsigned long strtablen = 0;
		  char *strtab = NULL;

		  symsec = SECTION_HEADER (section->sh_link);
		  if (symsec->sh_type != SHT_SYMTAB
		      && symsec->sh_type != SHT_DYNSYM)
                    continue;

		  nsyms = (symsec->sh_size / symsec->sh_entsize);
		  symtab = GET_ELF_SYMBOLS(file, symsec);

		  if (symtab == NULL)
		    continue;

		  if (SECTION_HEADER_INDEX(symsec->sh_link)
		      < elf_header.e_shnum)
		    {
		      strsec = SECTION_HEADER(symsec->sh_link);

		      strtab = (char *)get_data(NULL, file,
                                                strsec->sh_offset, 1,
                                                strsec->sh_size,
                                                _("string table"));
		      strtablen = ((strtab == NULL) ? 0 : strsec->sh_size);
		    }

		  dump_relocations(file, rel_offset, rel_size, symtab,
                                   nsyms, strtab, strtablen, is_rela);
		  if (strtab)
		    free (strtab);
		  free (symtab);
		}
	      else
		dump_relocations (file, rel_offset, rel_size,
				  NULL, 0, NULL, 0, is_rela);

	      found = 1;
	    }
	}

      if (! found)
	printf (_("\nThere are no relocations in this file.\n"));
    }

  return 1;
}

/* Process the unwind section.  */

#include "unwind-ia64.h"

/* An absolute address consists of a section and an offset.  If the
   section is NULL, the offset itself is the address, otherwise, the
   address equals to LOAD_ADDRESS(section) + offset.  */

struct absaddr
  {
    unsigned short section;
    bfd_vma offset;
  };

struct ia64_unw_table_entry
  {
    struct absaddr start;
    struct absaddr end;
    struct absaddr info;
  };

struct ia64_unw_aux_info
  {
    struct ia64_unw_table_entry *table;		/* Unwind table.  */
    unsigned long table_len;	/* Length of unwind table.  */
    unsigned char *info;	/* Unwind info.  */
    unsigned long info_size;	/* Size of unwind info.  */
    bfd_vma info_addr;		/* starting address of unwind info.  */
    bfd_vma seg_base;		/* Starting address of segment.  */
    Elf_Internal_Sym *symtab;	/* The symbol table.  */
    unsigned long nsyms;	/* Number of symbols.  */
    char *strtab;		/* The string table.  */
    unsigned long strtab_size;	/* Size of string table.  */
  };

static void
find_symbol_for_address (Elf_Internal_Sym *symtab,
			 unsigned long nsyms,
			 const char *strtab,
			 unsigned long strtab_size,
			 struct absaddr addr,
			 const char **symname,
			 bfd_vma *offset)
{
  bfd_vma dist = 0x100000;
  Elf_Internal_Sym *sym, *best = NULL;
  unsigned long i;

  for (i = 0, sym = symtab; i < nsyms; ++i, ++sym)
    {
      if (ELF_ST_TYPE (sym->st_info) == STT_FUNC
	  && sym->st_name != 0
	  && (addr.section == SHN_UNDEF || addr.section == sym->st_shndx)
	  && addr.offset >= sym->st_value
	  && addr.offset - sym->st_value < dist)
	{
	  best = sym;
	  dist = addr.offset - sym->st_value;
	  if (!dist)
	    break;
	}
    }
  if (best)
    {
      *symname = (best->st_name >= strtab_size
		  ? "<corrupt>" : strtab + best->st_name);
      *offset = dist;
      return;
    }
  *symname = NULL;
  *offset = addr.offset;
}

static void
dump_ia64_unwind (struct ia64_unw_aux_info *aux)
{
  struct ia64_unw_table_entry *tp;
  int in_body;

  for (tp = aux->table; tp < aux->table + aux->table_len; ++tp)
    {
      bfd_vma stamp;
      bfd_vma offset;
      const unsigned char *dp;
      const unsigned char *head;
      const char *procname;

      find_symbol_for_address (aux->symtab, aux->nsyms, aux->strtab,
			       aux->strtab_size, tp->start, &procname, &offset);

      fputs ("\n<", stdout);

      if (procname)
	{
	  fputs (procname, stdout);

	  if (offset)
	    printf ("+%lx", (unsigned long) offset);
	}

      fputs (">: [", stdout);
      print_vma (tp->start.offset, PREFIX_HEX);
      fputc ('-', stdout);
      print_vma (tp->end.offset, PREFIX_HEX);
      printf ("], info at +0x%lx\n",
	      (unsigned long) (tp->info.offset - aux->seg_base));

      head = aux->info + (tp->info.offset - aux->info_addr);
      stamp = byte_get ((unsigned char *) head, sizeof (stamp));

      printf ("  v%u, flags=0x%lx (%s%s), len=%lu bytes\n",
	      (unsigned) UNW_VER (stamp),
	      (unsigned long) ((stamp & UNW_FLAG_MASK) >> 32),
	      UNW_FLAG_EHANDLER (stamp) ? " ehandler" : "",
	      UNW_FLAG_UHANDLER (stamp) ? " uhandler" : "",
	      (unsigned long) (eh_addr_size * UNW_LENGTH (stamp)));

      if (UNW_VER (stamp) != 1)
	{
	  printf ("\tUnknown version.\n");
	  continue;
	}

      in_body = 0;
      for (dp = head + 8; dp < head + 8 + eh_addr_size * UNW_LENGTH (stamp);)
	dp = unw_decode (dp, in_body, & in_body);
    }
}

/* */
static int ATTRIBUTE_NONNULL(2)
slurp_ia64_unwind_table(FILE *file, struct ia64_unw_aux_info *aux,
                        Elf_Internal_Shdr *sec)
{
  unsigned long size, nrelas, i;
  Elf_Internal_Phdr *seg;
  struct ia64_unw_table_entry *tep;
  Elf_Internal_Shdr *relsec;
  Elf_Internal_Rela *rela, *rp;
  unsigned char *table, *tp;
  Elf_Internal_Sym *sym;
  const char *relname;

  /* 1st, find starting address of segment that includes this section: */
  if (elf_header.e_phnum)
    {
      if (! get_program_headers(file)) {
        return 0;
      }

      for (seg = program_headers;
	   seg < (program_headers + elf_header.e_phnum);
	   ++seg)
	{
	  if (seg->p_type != PT_LOAD) {
	    continue;
          }

	  if ((sec->sh_addr >= seg->p_vaddr)
	      && ((sec->sh_addr + sec->sh_size)
                  <= (seg->p_vaddr + seg->p_memsz)))
	    {
	      aux->seg_base = seg->p_vaddr;
	      break;
	    }
	}
    }

  /* 2nd, build unwind table from the contents of the unwind section: */
  size = sec->sh_size;
  table = (unsigned char *)get_data(NULL, file, sec->sh_offset, 1, size,
                                    _("unwind table"));
  if (!table) {
    return 0;
  }

  aux->table = ((struct ia64_unw_table_entry *)
  		xcmalloc(size / (3 * eh_addr_size), sizeof(aux->table[0])));
  tep = aux->table;
  for (tp = table; tp < (table + size); tp += (3 * eh_addr_size), ++tep)
    {
      tep->start.section = SHN_UNDEF;
      tep->end.section = SHN_UNDEF;
      tep->info.section = SHN_UNDEF;
      if (is_32bit_elf)
	{
	  tep->start.offset = byte_get((unsigned char *)tp + 0, 4);
	  tep->end.offset = byte_get((unsigned char *)tp + 4, 4);
	  tep->info.offset = byte_get((unsigned char *)tp + 8, 4);
	}
      else
	{
	  tep->start.offset = BYTE_GET((unsigned char *)tp + 0);
	  tep->end.offset = BYTE_GET((unsigned char *)tp + 8);
	  tep->info.offset = BYTE_GET((unsigned char *)tp + 16);
	}
      tep->start.offset += aux->seg_base;
      tep->end.offset += aux->seg_base;
      tep->info.offset += aux->seg_base;
    }
  free(table);

  /* Third, apply any relocations to the unwind table: */

  for (relsec = section_headers;
       relsec < (section_headers + elf_header.e_shnum);
       ++relsec)
    {
      if ((relsec->sh_type != SHT_RELA)
	  || (SECTION_HEADER_INDEX(relsec->sh_info) >= elf_header.e_shnum)
	  || (SECTION_HEADER(relsec->sh_info) != sec))
	continue;

      if (!slurp_rela_relocs(file, relsec->sh_offset, relsec->sh_size,
			     &rela, &nrelas))
	return 0;

      for (rp = rela; rp < (rela + nrelas); ++rp)
	{
	  if (is_32bit_elf)
	    {
	      relname = elf_ia64_reloc_type(ELF32_R_TYPE(rp->r_info));
	      sym = (aux->symtab + ELF32_R_SYM(rp->r_info));
	    }
	  else
	    {
	      relname = elf_ia64_reloc_type(ELF64_R_TYPE(rp->r_info));
	      sym = (aux->symtab + ELF64_R_SYM(rp->r_info));
	    }

	  if (!strneq(relname, "R_IA64_SEGREL", 13))
	    {
	      warn(_("Skipping unexpected relocation type %s\n"), relname);
	      continue;
	    }

	  i = (rp->r_offset / (3 * eh_addr_size));

	  switch ((rp->r_offset / eh_addr_size) % 3)
	    {
	    case 0:
	      aux->table[i].start.section = ((sym) ? sym->st_shndx : 0U);
	      aux->table[i].start.offset += (rp->r_addend
                                             + ((sym) ? sym->st_value : 0U));
	      break;
	    case 1:
	      aux->table[i].end.section = ((sym) ? sym->st_shndx : 0U);
	      aux->table[i].end.offset += (rp->r_addend
                                           + ((sym) ? sym->st_value : 0U));
	      break;
	    case 2:
	      aux->table[i].info.section = ((sym) ? sym->st_shndx : 0U);
	      aux->table[i].info.offset += (rp->r_addend
                                            + ((sym) ? sym->st_value : 0U));
	      break;
	    default:
	      break;
	    }
	}

      free(rela);
    }

  aux->table_len = (size / (3 * eh_addr_size));
  return 1;
}

/* */
static int
ia64_process_unwind(FILE *file)
{
  Elf_Internal_Shdr *sec, *unwsec = NULL, *strsec;
  unsigned long i, unwcount = 0UL, unwstart = 0UL;
  struct ia64_unw_aux_info aux;

  memset(&aux, 0, sizeof(aux));

  for (i = 0, sec = section_headers; i < elf_header.e_shnum; ++i, ++sec)
    {
      if ((sec->sh_type == SHT_SYMTAB)
	  && (SECTION_HEADER_INDEX(sec->sh_link) < elf_header.e_shnum))
	{
	  aux.nsyms = (sec->sh_size / sec->sh_entsize);
	  aux.symtab = GET_ELF_SYMBOLS(file, sec);

	  strsec = SECTION_HEADER(sec->sh_link);
	  aux.strtab = (char *)get_data(NULL, file, strsec->sh_offset,
                                        1, strsec->sh_size,
                                        _("string table"));
	  aux.strtab_size = ((aux.strtab != NULL) ? strsec->sh_size : 0);
	}
      else if (sec->sh_type == SHT_IA_64_UNWIND)
	unwcount++;
    }

  if (!unwcount)
    printf(_("\nThere are no unwind sections in this file.\n"));

  while (unwcount-- > 0)
    {
      const char *suffix;
      size_t len, len2;

      for (i = unwstart, sec = (section_headers + unwstart);
	   i < elf_header.e_shnum; ++i, ++sec)
	if (sec->sh_type == SHT_IA_64_UNWIND)
	  {
	    unwsec = sec;
	    break;
	  }

      unwstart = (i + 1);
      len = (sizeof(ELF_STRING_ia64_unwind_once) - 1UL);

      if ((unwsec != NULL) && ((unwsec->sh_flags & SHF_GROUP) != 0))
	{
	  /* We need to find which section group it is in: */
	  struct group_list *g = section_headers_groups[i]->root;

	  for (; g != NULL; g = g->next)
	    {
	      sec = SECTION_HEADER(g->section_index);

	      if (streq(SECTION_NAME(sec), ELF_STRING_ia64_unwind_info))
		break;
	    }

	  if (g == NULL)
	    i = elf_header.e_shnum;
	}
      else if (strneq(SECTION_NAME(unwsec), ELF_STRING_ia64_unwind_once, len))
	{
	  /* .gnu.linkonce.ia64unw.FOO -> .gnu.linkonce.ia64unwi.FOO.  */
	  len2 = sizeof (ELF_STRING_ia64_unwind_info_once) - 1;
	  suffix = SECTION_NAME (unwsec) + len;
	  for (i = 0, sec = section_headers; i < elf_header.e_shnum;
	       ++i, ++sec)
	    if (strneq (SECTION_NAME (sec), ELF_STRING_ia64_unwind_info_once, len2)
		&& streq (SECTION_NAME (sec) + len2, suffix))
	      break;
	}
      else
	{
	  /* .IA_64.unwindFOO -> .IA_64.unwind_infoFOO
	     .IA_64.unwind or BAR -> .IA_64.unwind_info.  */
	  len = sizeof (ELF_STRING_ia64_unwind) - 1;
	  len2 = sizeof (ELF_STRING_ia64_unwind_info) - 1;
	  suffix = "";
	  if (strneq (SECTION_NAME (unwsec), ELF_STRING_ia64_unwind, len))
	    suffix = SECTION_NAME (unwsec) + len;
	  for (i = 0, sec = section_headers; i < elf_header.e_shnum;
	       ++i, ++sec)
	    if (strneq (SECTION_NAME (sec), ELF_STRING_ia64_unwind_info, len2)
		&& streq (SECTION_NAME (sec) + len2, suffix))
	      break;
	}

      if (i == elf_header.e_shnum)
	{
	  printf(_("\nCould not find unwind info section for "));

	  if (string_table == NULL)
	    printf("%d", unwsec->sh_name);
	  else
	    printf(_("'%s'"), SECTION_NAME(unwsec));
	}
      else
	{
	  aux.info_size = sec->sh_size;
	  aux.info_addr = sec->sh_addr;
	  aux.info = (unsigned char *)get_data (NULL, file, sec->sh_offset,
                                                1, aux.info_size,
                                                _("unwind info"));

	  printf(_("\nUnwind section "));

	  if (string_table == NULL)
	    printf("%d", unwsec->sh_name);
	  else
	    printf(_("'%s'"), SECTION_NAME (unwsec));

	  printf(_(" at offset 0x%lx contains %lu entries:\n"),
                 (unsigned long)unwsec->sh_offset,
                 (unsigned long)(unwsec->sh_size / (3 * eh_addr_size)));

	  (void)slurp_ia64_unwind_table(file, & aux, unwsec);

	  if (aux.table_len > 0)
	    dump_ia64_unwind (& aux);

	  if (aux.table)
	    free((char *)aux.table);
	  if (aux.info)
	    free((char *)aux.info);
	  aux.table = NULL;
	  aux.info = NULL;
	}
    }

  if (aux.symtab)
    free(aux.symtab);
  if (aux.strtab)
    free((char *)aux.strtab);

  return 1;
}

struct hppa_unw_table_entry
  {
    struct absaddr start;
    struct absaddr end;
    unsigned int Cannot_unwind:1;			/* 0 */
    unsigned int Millicode:1;			/* 1 */
    unsigned int Millicode_save_sr0:1;		/* 2 */
    unsigned int Region_description:2;		/* 3..4 */
    unsigned int reserved1:1;			/* 5 */
    unsigned int Entry_SR:1;			/* 6 */
    unsigned int Entry_FR:4;     /* number saved */	/* 7..10 */
    unsigned int Entry_GR:5;     /* number saved */	/* 11..15 */
    unsigned int Args_stored:1;			/* 16 */
    unsigned int Variable_Frame:1;			/* 17 */
    unsigned int Separate_Package_Body:1;		/* 18 */
    unsigned int Frame_Extension_Millicode:1;	/* 19 */
    unsigned int Stack_Overflow_Check:1;		/* 20 */
    unsigned int Two_Instruction_SP_Increment:1;	/* 21 */
    unsigned int Ada_Region:1;			/* 22 */
    unsigned int cxx_info:1;			/* 23 */
    unsigned int cxx_try_catch:1;			/* 24 */
    unsigned int sched_entry_seq:1;			/* 25 */
    unsigned int reserved2:1;			/* 26 */
    unsigned int Save_SP:1;				/* 27 */
    unsigned int Save_RP:1;				/* 28 */
    unsigned int Save_MRP_in_frame:1;		/* 29 */
    unsigned int extn_ptr_defined:1;		/* 30 */
    unsigned int Cleanup_defined:1;			/* 31 */

    unsigned int MPE_XL_interrupt_marker:1;		/* 0 */
    unsigned int HP_UX_interrupt_marker:1;		/* 1 */
    unsigned int Large_frame:1;			/* 2 */
    unsigned int Pseudo_SP_Set:1;			/* 3 */
    unsigned int reserved4:1;			/* 4 */
    unsigned int Total_frame_size:27;		/* 5..31 */
  };

struct hppa_unw_aux_info
  {
    struct hppa_unw_table_entry *table;		/* Unwind table.  */
    unsigned long table_len;	/* Length of unwind table.  */
    bfd_vma seg_base;		/* Starting address of segment.  */
    Elf_Internal_Sym *symtab;	/* The symbol table.  */
    unsigned long nsyms;	/* Number of symbols.  */
    char *strtab;		/* The string table.  */
    unsigned long strtab_size;	/* Size of string table.  */
  };

static void
dump_hppa_unwind (struct hppa_unw_aux_info *aux)
{
  struct hppa_unw_table_entry *tp;

  for (tp = aux->table; tp < aux->table + aux->table_len; ++tp)
    {
      bfd_vma offset;
      const char *procname;

      find_symbol_for_address (aux->symtab, aux->nsyms, aux->strtab,
			       aux->strtab_size, tp->start, &procname,
			       &offset);

      fputs ("\n<", stdout);

      if (procname)
	{
	  fputs (procname, stdout);

	  if (offset)
	    printf ("+%lx", (unsigned long) offset);
	}

      fputs (">: [", stdout);
      print_vma (tp->start.offset, PREFIX_HEX);
      fputc ('-', stdout);
      print_vma (tp->end.offset, PREFIX_HEX);
      printf ("]\n\t");

#define PF(_m) if (tp->_m) printf (#_m " ");
#define PV(_m) if (tp->_m) printf (#_m "=%d ", tp->_m);
      PF(Cannot_unwind);
      PF(Millicode);
      PF(Millicode_save_sr0);
      /* PV(Region_description);  */
      PF(Entry_SR);
      PV(Entry_FR);
      PV(Entry_GR);
      PF(Args_stored);
      PF(Variable_Frame);
      PF(Separate_Package_Body);
      PF(Frame_Extension_Millicode);
      PF(Stack_Overflow_Check);
      PF(Two_Instruction_SP_Increment);
      PF(Ada_Region);
      PF(cxx_info);
      PF(cxx_try_catch);
      PF(sched_entry_seq);
      PF(Save_SP);
      PF(Save_RP);
      PF(Save_MRP_in_frame);
      PF(extn_ptr_defined);
      PF(Cleanup_defined);
      PF(MPE_XL_interrupt_marker);
      PF(HP_UX_interrupt_marker);
      PF(Large_frame);
      PF(Pseudo_SP_Set);
      PV(Total_frame_size);
#undef PF
#undef PV
    }

  printf ("\n");
}

/* */
static int ATTRIBUTE_NONNULL(2)
slurp_hppa_unwind_table(FILE *file, struct hppa_unw_aux_info *aux,
                        Elf_Internal_Shdr *sec)
{
  unsigned long size, unw_ent_size, nentries, nrelas, i;
  Elf_Internal_Phdr *seg;
  struct hppa_unw_table_entry *tep;
  Elf_Internal_Shdr *relsec;
  Elf_Internal_Rela *rela, *rp;
  unsigned char *table, *tp;
  Elf_Internal_Sym *sym;
  const char *relname;

  /* 1st, find starting address of segment that includes this section: */
  if (elf_header.e_phnum)
    {
      if (! get_program_headers(file)) {
	return 0;
      }

      for (seg = program_headers;
	   seg < (program_headers + elf_header.e_phnum);
	   ++seg)
	{
	  if (seg->p_type != PT_LOAD) {
	    continue;
          }

	  if ((sec->sh_addr >= seg->p_vaddr)
	      && ((sec->sh_addr + sec->sh_size) <= (seg->p_vaddr + seg->p_memsz)))
	    {
	      aux->seg_base = seg->p_vaddr;
	      break;
	    }
	}
    }

  /* 2nd, build the unwind table from contents of the unwind section: */
  size = sec->sh_size;
  table = (unsigned char *)get_data(NULL, file, sec->sh_offset, 1, size,
                                    _("unwind table"));
  if (!table) {
    return 0;
  }

  unw_ent_size = 16;
  nentries = (size / unw_ent_size);
  size = (unw_ent_size * nentries);

  tep = aux->table = ((struct hppa_unw_table_entry *)
                      xcmalloc(nentries, sizeof(aux->table[0])));

  if (tep == NULL) {
    xmalloc_failed(sizeof(aux->table[0]));
  }

  for (tp = table; tp < (table + size); tp += unw_ent_size, ++tep)
    {
      unsigned int tmp1, tmp2;

      tep->start.section = SHN_UNDEF;
      tep->end.section = SHN_UNDEF;

      tep->start.offset = byte_get((unsigned char *)tp + 0, 4);
      tep->end.offset = byte_get((unsigned char *)tp + 4, 4);
      tmp1 = byte_get((unsigned char *)tp + 8, 4);
      tmp2 = byte_get((unsigned char *)tp + 12, 4);

      tep->start.offset += aux->seg_base;
      tep->end.offset += aux->seg_base;

      tep->Cannot_unwind = ((tmp1 >> 31) & 0x1); /*GCC PR 39170*/
      tep->Millicode = (tmp1 >> 30) & 0x1;
      tep->Millicode_save_sr0 = (tmp1 >> 29) & 0x1;
      tep->Region_description = (tmp1 >> 27) & 0x3;
      tep->reserved1 = (tmp1 >> 26) & 0x1;
      tep->Entry_SR = (tmp1 >> 25) & 0x1;
      tep->Entry_FR = (tmp1 >> 21) & 0xf;
      tep->Entry_GR = (tmp1 >> 16) & 0x1f;
      tep->Args_stored = (tmp1 >> 15) & 0x1;
      tep->Variable_Frame = (tmp1 >> 14) & 0x1;
      tep->Separate_Package_Body = (tmp1 >> 13) & 0x1;
      tep->Frame_Extension_Millicode = (tmp1 >> 12) & 0x1;
      tep->Stack_Overflow_Check = (tmp1 >> 11) & 0x1;
      tep->Two_Instruction_SP_Increment = (tmp1 >> 10) & 0x1;
      tep->Ada_Region = (tmp1 >> 9) & 0x1;
      tep->cxx_info = (tmp1 >> 8) & 0x1;
      tep->cxx_try_catch = (tmp1 >> 7) & 0x1;
      tep->sched_entry_seq = (tmp1 >> 6) & 0x1;
      tep->reserved2 = (tmp1 >> 5) & 0x1;
      tep->Save_SP = (tmp1 >> 4) & 0x1;
      tep->Save_RP = (tmp1 >> 3) & 0x1;
      tep->Save_MRP_in_frame = (tmp1 >> 2) & 0x1;
      tep->extn_ptr_defined = (tmp1 >> 1) & 0x1;
      tep->Cleanup_defined = tmp1 & 0x1;

      tep->MPE_XL_interrupt_marker = ((tmp2 >> 31) & 0x1); /*GCC PR 39170*/
      tep->HP_UX_interrupt_marker = (tmp2 >> 30) & 0x1;
      tep->Large_frame = (tmp2 >> 29) & 0x1;
      tep->Pseudo_SP_Set = (tmp2 >> 28) & 0x1;
      tep->reserved4 = (tmp2 >> 27) & 0x1;
      tep->Total_frame_size = tmp2 & 0x7ffffff;
    }
  free(table);

  /* Third, apply any relocations to the unwind table.  */

  for (relsec = section_headers;
       relsec < (section_headers + elf_header.e_shnum);
       ++relsec)
    {
      if ((relsec->sh_type != SHT_RELA)
	  || (SECTION_HEADER_INDEX(relsec->sh_info) >= elf_header.e_shnum)
	  || (SECTION_HEADER(relsec->sh_info) != sec))
	continue;

      if (!slurp_rela_relocs(file, relsec->sh_offset, relsec->sh_size,
                             &rela, & nrelas))
	return 0;

      for (rp = rela; rp < (rela + nrelas); ++rp)
	{
	  if (is_32bit_elf)
	    {
	      relname = elf_hppa_reloc_type(ELF32_R_TYPE(rp->r_info));
	      sym = (aux->symtab + ELF32_R_SYM(rp->r_info));
	    }
	  else
	    {
	      relname = elf_hppa_reloc_type(ELF64_R_TYPE(rp->r_info));
	      sym = (aux->symtab + ELF64_R_SYM(rp->r_info));
	    }

	  /* R_PARISC_SEGREL32 or R_PARISC_SEGREL64.  */
	  if (strncmp(relname, "R_PARISC_SEGREL", 15) != 0)
	    {
	      warn(_("Skipping unexpected relocation type %s\n"), relname);
	      continue;
	    }

	  i = (rp->r_offset / unw_ent_size);

	  switch ((rp->r_offset % unw_ent_size) / eh_addr_size)
	    {
	    case 0:
	      aux->table[i].start.section = ((sym) ? sym->st_shndx : 0U);
	      aux->table[i].start.offset += (((sym) ? sym->st_value : 0U)
                                             + rp->r_addend);
	      break;
	    case 1:
	      aux->table[i].end.section = ((sym) ? sym->st_shndx : 0U);
	      aux->table[i].end.offset += (((sym) ? sym->st_value : 0U)
                                           + rp->r_addend);
	      break;
	    default:
	      break;
	    }
	}

      free(rela);
    }

  aux->table_len = nentries;

  return 1;
}

/* */
static int
hppa_process_unwind(FILE *file)
{
  struct hppa_unw_aux_info aux;
  Elf_Internal_Shdr *unwsec = NULL;
  Elf_Internal_Shdr *strsec;
  Elf_Internal_Shdr *sec;
  unsigned long i;

  memset(&aux, 0, sizeof(aux));

  if (string_table == NULL)
    return 1;

  for (i = 0, sec = section_headers; i < elf_header.e_shnum; ++i, ++sec)
    {
      if ((sec->sh_type == SHT_SYMTAB)
	  && (SECTION_HEADER_INDEX(sec->sh_link) < elf_header.e_shnum))
	{
	  aux.nsyms = (sec->sh_size / sec->sh_entsize);
	  aux.symtab = GET_ELF_SYMBOLS(file, sec);

	  strsec = SECTION_HEADER(sec->sh_link);
	  aux.strtab = (char *)get_data(NULL, file, strsec->sh_offset,
                                        1, strsec->sh_size,
                                        _("string table"));
	  aux.strtab_size = ((aux.strtab != NULL) ? strsec->sh_size : 0);
	}
      else if (streq(SECTION_NAME(sec), ".PARISC.unwind"))
	unwsec = sec;
    }

  if (!unwsec)
    printf (_("\nThere are no unwind sections in this file.\n"));

  for (i = 0, sec = section_headers; i < elf_header.e_shnum; ++i, ++sec)
    {
      if (streq (SECTION_NAME (sec), ".PARISC.unwind"))
	{
	  printf (_("\nUnwind section "));
	  printf (_("'%s'"), SECTION_NAME (sec));

	  printf (_(" at offset 0x%lx contains %lu entries:\n"),
		  (unsigned long) sec->sh_offset,
		  (unsigned long) (sec->sh_size / (2 * eh_addr_size + 8)));

          slurp_hppa_unwind_table (file, &aux, sec);
	  if (aux.table_len > 0)
	    dump_hppa_unwind (&aux);

	  if (aux.table)
	    free ((char *) aux.table);
	  aux.table = NULL;
	}
    }

  if (aux.symtab)
    free (aux.symtab);
  if (aux.strtab)
    free ((char *) aux.strtab);

  return 1;
}

static int
process_unwind (FILE *file)
{
  struct unwind_handler {
    int machtype;
    int (*handler)(FILE *file);
  } handlers[] = {
    { EM_IA_64, ia64_process_unwind },
    { EM_PARISC, hppa_process_unwind },
    { 0, 0 }
  };
  int i;

  if (!do_unwind)
    return 1;

  for (i = 0; handlers[i].handler != NULL; i++)
    if (elf_header.e_machine == handlers[i].machtype)
      return handlers[i].handler (file);

  printf (_("\nThere are no unwind sections in this file.\n"));
  return 1;
}

static void
dynamic_section_mips_val(Elf_Internal_Dyn *entry)
{
  switch (entry->d_tag)
    {
    case DT_MIPS_FLAGS:
      if (entry->d_un.d_val == 0)
	printf("NONE\n");
      else
	{
	  static const char * opts[] =
	  {
	    "QUICKSTART", "NOTPOT", "NO_LIBRARY_REPLACEMENT",
	    "NO_MOVE", "SGI_ONLY", "GUARANTEE_INIT", "DELTA_C_PLUS_PLUS",
	    "GUARANTEE_START_INIT", "PIXIE", "DEFAULT_DELAY_LOAD",
	    "REQUICKSTART", "REQUICKSTARTED", "CORD", "NO_UNRES_UNDEF",
	    "RLD_ORDER_SAFE"
	  };
	  unsigned int cnt;
	  int first = 1;
	  for (cnt = 0; cnt < NUM_ELEM (opts); ++cnt)
	    if (entry->d_un.d_val & (1 << cnt))
	      {
		printf("%s%s", first ? "" : " ", opts[cnt]);
		first = 0;
	      }
	  puts("");
	}
      break;

    case DT_MIPS_IVERSION:
      if (VALID_DYNAMIC_NAME(entry->d_un.d_val))
	printf("Interface Version: %s\n", GET_DYNAMIC_NAME(entry->d_un.d_val));
      else
	printf("<corrupt: %ld>\n", (long)entry->d_un.d_ptr);
      break;

    case DT_MIPS_TIME_STAMP:
      {
	char timebuf[66]; /* big enough for -Wformat-length */
	struct tm *tmp;

	time_t time = (time_t)entry->d_un.d_val;
	tmp = gmtime(&time);
	snprintf(timebuf, sizeof(timebuf), "%04u-%02u-%02uT%02u:%02u:%02u",
		 (tmp->tm_year + 1900), (tmp->tm_mon + 1), tmp->tm_mday,
		 tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	printf("Time Stamp: %s\n", timebuf);
      }
      break;

    case DT_MIPS_RLD_VERSION:
    case DT_MIPS_LOCAL_GOTNO:
    case DT_MIPS_CONFLICTNO:
    case DT_MIPS_LIBLISTNO:
    case DT_MIPS_SYMTABNO:
    case DT_MIPS_UNREFEXTNO:
    case DT_MIPS_HIPAGENO:
    case DT_MIPS_DELTA_CLASS_NO:
    case DT_MIPS_DELTA_INSTANCE_NO:
    case DT_MIPS_DELTA_RELOC_NO:
    case DT_MIPS_DELTA_SYM_NO:
    case DT_MIPS_DELTA_CLASSSYM_NO:
    case DT_MIPS_COMPACT_SIZE:
      printf ("%ld\n", (long) entry->d_un.d_ptr);
      break;

    default:
      printf ("%#lx\n", (long) entry->d_un.d_ptr);
    }
}


static void
dynamic_section_parisc_val(Elf_Internal_Dyn *entry)
{
  switch (entry->d_tag)
    {
    case DT_HP_DLD_FLAGS:
      {
	static struct
	{
	  long int bit;
	  const char *str;
	}
	flags[] =
	{
	  { DT_HP_DEBUG_PRIVATE, "HP_DEBUG_PRIVATE" },
	  { DT_HP_DEBUG_CALLBACK, "HP_DEBUG_CALLBACK" },
	  { DT_HP_DEBUG_CALLBACK_BOR, "HP_DEBUG_CALLBACK_BOR" },
	  { DT_HP_NO_ENVVAR, "HP_NO_ENVVAR" },
	  { DT_HP_BIND_NOW, "HP_BIND_NOW" },
	  { DT_HP_BIND_NONFATAL, "HP_BIND_NONFATAL" },
	  { DT_HP_BIND_VERBOSE, "HP_BIND_VERBOSE" },
	  { DT_HP_BIND_RESTRICTED, "HP_BIND_RESTRICTED" },
	  { DT_HP_BIND_SYMBOLIC, "HP_BIND_SYMBOLIC" },
	  { DT_HP_RPATH_FIRST, "HP_RPATH_FIRST" },
	  { DT_HP_BIND_DEPTH_FIRST, "HP_BIND_DEPTH_FIRST" },
	  { DT_HP_GST, "HP_GST" },
	  { DT_HP_SHLIB_FIXED, "HP_SHLIB_FIXED" },
	  { DT_HP_MERGE_SHLIB_SEG, "HP_MERGE_SHLIB_SEG" },
	  { DT_HP_NODELETE, "HP_NODELETE" },
	  { DT_HP_GROUP, "HP_GROUP" },
	  { DT_HP_PROTECT_LINKAGE_TABLE, "HP_PROTECT_LINKAGE_TABLE" }
	};
	int first = 1;
	size_t cnt;
	bfd_vma val = entry->d_un.d_val;

	for (cnt = 0; cnt < (sizeof(flags) / sizeof(flags[0])); ++cnt)
	  if (val & flags[cnt].bit)
	    {
	      if (! first) {
		putchar(' ');
              }
	      fputs(flags[cnt].str, stdout);
	      first = 0;
	      val ^= flags[cnt].bit;
	    }

	if ((val != 0) || first)
	  {
	    if (! first) {
	      putchar(' ');
            }
	    print_vma(val, HEX);
	  }
      }
      break;

    default:
      print_vma(entry->d_un.d_ptr, PREFIX_HEX);
      break;
    }
  putchar('\n');
}

static void
dynamic_section_ia64_val(Elf_Internal_Dyn *entry)
{
  switch (entry->d_tag)
    {
    case DT_IA_64_PLT_RESERVE:
      /* First 3 slots reserved: */
      print_vma(entry->d_un.d_ptr, PREFIX_HEX);
      printf(" -- ");
      print_vma(entry->d_un.d_ptr + (3 * 8), PREFIX_HEX);
      break;
    default:
      print_vma(entry->d_un.d_ptr, PREFIX_HEX);
      break;
    }
  putchar('\n');
}

static int
get_32bit_dynamic_section(FILE *file)
{
  Elf32_External_Dyn *edyn, *ext;
  Elf_Internal_Dyn *entry;

  edyn = (Elf32_External_Dyn *)get_data(NULL, file, dynamic_addr, 1,
                                        dynamic_size,
                                        _("dynamic section"));
  if (!edyn) {
    return 0;
  }

  /* SGI's ELF has more than one section in the DYNAMIC segment, and we
   * might not have the luxury of section headers.  Look for the DT_NULL
   * terminator to determine the number of entries: */
  for (ext = edyn, dynamic_nent = 0;
       (char *)ext < ((char *)edyn + dynamic_size);
       ext++)
    {
      dynamic_nent++;
      if (BYTE_GET(ext->d_tag) == DT_NULL) {
	break;
      }
    }

  dynamic_section = (Elf_Internal_Dyn *)cmalloc(dynamic_nent,
                                                sizeof(*entry));
  if (dynamic_section == NULL)
    {
      error(_("Out of memory\n"));
      free(edyn);
      return 0;
    }

  for (ext = edyn, entry = dynamic_section;
       entry < (dynamic_section + dynamic_nent);
       ext++, entry++)
    {
      entry->d_tag = BYTE_GET(ext->d_tag);
      entry->d_un.d_val = BYTE_GET(ext->d_un.d_val);
    }

  free (edyn);

  return 1;
}

static int
get_64bit_dynamic_section(FILE *file)
{
  Elf64_External_Dyn *edyn, *ext;
  Elf_Internal_Dyn *entry;

  edyn = (Elf64_External_Dyn *)get_data(NULL, file, dynamic_addr, 1,
                                        dynamic_size,
                                        _("dynamic section"));
  if (!edyn) {
    return 0;
  }

  /* SGI's ELF has more than one section in the DYNAMIC segment, and we
   * might not have the luxury of section headers.  Look for the DT_NULL
   * terminator to determine the number of entries: */
  for (ext = edyn, dynamic_nent = 0;
       (char *)ext < ((char *)edyn + dynamic_size);
       ext++)
    {
      dynamic_nent++;
      if (BYTE_GET(ext->d_tag) == DT_NULL) {
	break;
      }
    }

  dynamic_section = (Elf_Internal_Dyn *)cmalloc(dynamic_nent,
                                                sizeof(*entry));
  if (dynamic_section == NULL)
    {
      error(_("Out of memory\n"));
      free(edyn);
      return 0;
    }

  for (ext = edyn, entry = dynamic_section;
       entry < dynamic_section + dynamic_nent;
       ext++, entry++)
    {
      entry->d_tag      = BYTE_GET (ext->d_tag);
      entry->d_un.d_val = BYTE_GET (ext->d_un.d_val);
    }

  free (edyn);

  return 1;
}

static void
print_dynamic_flags (bfd_vma flags)
{
  int first = 1;

  while (flags)
    {
      bfd_vma flag;

      flag = flags & - flags;
      flags &= ~ flag;

      if (first)
	first = 0;
      else
	putc (' ', stdout);

      switch (flag)
	{
	case DF_ORIGIN:		fputs ("ORIGIN", stdout); break;
	case DF_SYMBOLIC:	fputs ("SYMBOLIC", stdout); break;
	case DF_TEXTREL:	fputs ("TEXTREL", stdout); break;
	case DF_BIND_NOW:	fputs ("BIND_NOW", stdout); break;
	case DF_STATIC_TLS:	fputs ("STATIC_TLS", stdout); break;
	default:		fputs ("unknown", stdout); break;
	}
    }
  puts ("");
}

/* Parse and display the contents of the dynamic section.  */

static int
process_dynamic_section (FILE *file)
{
  Elf_Internal_Dyn *entry;

  if (dynamic_size == 0)
    {
      if (do_dynamic)
	printf (_("\nThere is no dynamic section in this file.\n"));

      return 1;
    }

  if (is_32bit_elf)
    {
      if (! get_32bit_dynamic_section (file))
	return 0;
    }
  else if (! get_64bit_dynamic_section (file))
    return 0;

  /* Find the appropriate symbol table.  */
  if (dynamic_symbols == NULL)
    {
      for (entry = dynamic_section;
	   entry < dynamic_section + dynamic_nent;
	   ++entry)
	{
	  Elf_Internal_Shdr section;

	  if (entry->d_tag != DT_SYMTAB)
	    continue;

	  dynamic_info[DT_SYMTAB] = entry->d_un.d_val;

	  /* Since we do not know how big the symbol table is,
	     we default to reading in the entire file (!) and
	     processing that. This is overkill, I know, but it
	     should work.  */
	  section.sh_offset = offset_from_vma (file, entry->d_un.d_val, 0);

	  if (archive_file_offset != 0)
	    section.sh_size = archive_file_size - section.sh_offset;
	  else
	    {
	      if (fseek (file, 0, SEEK_END))
		error (_("Unable to seek to end of file!"));

	      section.sh_size = ftell (file) - section.sh_offset;
	    }

	  if (is_32bit_elf)
	    section.sh_entsize = sizeof (Elf32_External_Sym);
	  else
	    section.sh_entsize = sizeof (Elf64_External_Sym);

	  num_dynamic_syms = section.sh_size / section.sh_entsize;
	  if (num_dynamic_syms < 1)
	    {
	      error (_("Unable to determine the number of symbols to load\n"));
	      continue;
	    }

	  dynamic_symbols = GET_ELF_SYMBOLS (file, &section);
	}
    }

  /* Similarly find a string table.  */
  if (dynamic_strings == NULL)
    {
      for (entry = dynamic_section;
	   entry < dynamic_section + dynamic_nent;
	   ++entry)
	{
	  unsigned long offset;
	  long str_tab_len;

	  if (entry->d_tag != DT_STRTAB)
	    continue;

	  dynamic_info[DT_STRTAB] = entry->d_un.d_val;

	  /* Since we do not know how big the string table is,
	     we default to reading in the entire file (!) and
	     processing that. This is overkill, I know, but it
	     should work.  */

	  offset = offset_from_vma (file, entry->d_un.d_val, 0);

	  if (archive_file_offset != 0)
	    str_tab_len = archive_file_size - offset;
	  else
	    {
	      if (fseek (file, 0, SEEK_END))
		error (_("Unable to seek to end of file\n"));
	      str_tab_len = ftell (file) - offset;
	    }

	  if (str_tab_len < 1)
	    {
	      error
		(_("Unable to determine the length of the dynamic string table\n"));
	      continue;
	    }

	  dynamic_strings = (char *)get_data(NULL, file, offset, 1,
                                             str_tab_len,
                                             _("dynamic string table"));
	  dynamic_strings_length = str_tab_len;
	  break;
	}
    }

  /* And find the syminfo section if available: */
  if (dynamic_syminfo == NULL)
    {
      unsigned long syminsz = 0;

      for (entry = dynamic_section;
	   entry < dynamic_section + dynamic_nent;
	   ++entry)
	{
	  if (entry->d_tag == DT_SYMINENT)
	    {
	      /* Note: these braces are necessary to avoid a syntax
		   * error from the SunOS4 C compiler.  */
	      assert (sizeof (Elf_External_Syminfo) == entry->d_un.d_val);
	    }
	  else if (entry->d_tag == DT_SYMINSZ)
	    syminsz = entry->d_un.d_val;
	  else if (entry->d_tag == DT_SYMINFO)
	    dynamic_syminfo_offset = offset_from_vma (file, entry->d_un.d_val,
						      syminsz);
	}

      if ((dynamic_syminfo_offset != 0) && (syminsz != 0))
	{
	  Elf_External_Syminfo *extsyminfo, *extsym;
	  Elf_Internal_Syminfo *syminfo;

	  /* There is a syminfo section, so read the data: */
	  extsyminfo = (Elf_External_Syminfo *)get_data(NULL, file,
                                                        dynamic_syminfo_offset,
                                                        1, syminsz,
                                                        _("symbol information"));
	  if (!extsyminfo) {
	    return 0;
          }

	  dynamic_syminfo = (Elf_Internal_Syminfo *)malloc(syminsz);
	  if (dynamic_syminfo == NULL)
	    {
	      error(_("Out of memory\n"));
	      return 0;
	    }

	  dynamic_syminfo_nent = (syminsz / sizeof(Elf_External_Syminfo));
	  for (syminfo = dynamic_syminfo, extsym = extsyminfo;
	       syminfo < dynamic_syminfo + dynamic_syminfo_nent;
	       ++syminfo, ++extsym)
	    {
	      syminfo->si_boundto = BYTE_GET (extsym->si_boundto);
	      syminfo->si_flags = BYTE_GET (extsym->si_flags);
	    }

	  free (extsyminfo);
	}
    }

  if (do_dynamic && dynamic_addr)
    printf (_("\nDynamic section at offset 0x%lx contains %u entries:\n"),
	    dynamic_addr, dynamic_nent);
  if (do_dynamic)
    printf (_("  Tag        Type                         Name/Value\n"));

  for (entry = dynamic_section;
       entry < dynamic_section + dynamic_nent;
       entry++)
    {
      if (do_dynamic)
	{
	  const char *dtype;

	  putchar(' ');
	  print_vma(entry->d_tag, FULL_HEX);
	  dtype = get_dynamic_type(entry->d_tag);
	  printf(" (%s)%*s", dtype,
		 ((is_32bit_elf ? 27 : 19)
		  - (int8_t)strlen(dtype)),
		 " ");
	}

      switch (entry->d_tag)
	{
	case DT_FLAGS:
	  if (do_dynamic)
	    print_dynamic_flags (entry->d_un.d_val);
	  break;

	case DT_AUXILIARY:
	case DT_FILTER:
	case DT_CONFIG:
	case DT_DEPAUDIT:
	case DT_AUDIT:
	  if (do_dynamic)
	    {
	      switch (entry->d_tag)
		{
		case DT_AUXILIARY:
		  printf(_("Auxiliary library"));
		  break;

		case DT_FILTER:
		  printf(_("Filter library"));
		  break;

		case DT_CONFIG:
		  printf(_("Configuration file"));
		  break;

		case DT_DEPAUDIT:
		  printf(_("Dependency audit library"));
		  break;

		case DT_AUDIT:
		  printf(_("Audit library"));
		  break;

                default:
                  break;
		}

	      if (VALID_DYNAMIC_NAME (entry->d_un.d_val))
		printf (": [%s]\n", GET_DYNAMIC_NAME (entry->d_un.d_val));
	      else
		{
		  printf (": ");
		  print_vma (entry->d_un.d_val, PREFIX_HEX);
		  putchar ('\n');
		}
	    }
	  break;

	case DT_FEATURE:
	  if (do_dynamic)
	    {
	      printf (_("Flags:"));

	      if (entry->d_un.d_val == 0)
		printf (_(" None\n"));
	      else
		{
		  unsigned long int val = (unsigned long int)entry->d_un.d_val;

		  if (val & DTF_1_PARINIT)
		    {
		      printf (" PARINIT");
		      val ^= DTF_1_PARINIT;
		    }
		  if (val & DTF_1_CONFEXP)
		    {
		      printf (" CONFEXP");
		      val ^= DTF_1_CONFEXP;
		    }
		  if (val != 0)
		    printf (" %lx", val);
		  puts ("");
		}
	    }
	  break;

	case DT_POSFLAG_1:
	  if (do_dynamic)
	    {
	      printf (_("Flags:"));

	      if (entry->d_un.d_val == 0)
		printf (_(" None\n"));
	      else
		{
		  unsigned long int val = (unsigned long int)entry->d_un.d_val;

		  if (val & DF_P1_LAZYLOAD)
		    {
		      printf (" LAZYLOAD");
		      val ^= DF_P1_LAZYLOAD;
		    }
		  if (val & DF_P1_GROUPPERM)
		    {
		      printf (" GROUPPERM");
		      val ^= DF_P1_GROUPPERM;
		    }
		  if (val != 0)
		    printf (" %lx", val);
		  puts ("");
		}
	    }
	  break;

	case DT_FLAGS_1:
	  if (do_dynamic)
	    {
	      printf (_("Flags:"));
	      if (entry->d_un.d_val == 0)
		printf (_(" None\n"));
	      else
		{
		  unsigned long int val = (unsigned long int)entry->d_un.d_val;

		  if (val & DF_1_NOW)
		    {
		      printf (" NOW");
		      val ^= DF_1_NOW;
		    }
		  if (val & DF_1_GLOBAL)
		    {
		      printf (" GLOBAL");
		      val ^= DF_1_GLOBAL;
		    }
		  if (val & DF_1_GROUP)
		    {
		      printf (" GROUP");
		      val ^= DF_1_GROUP;
		    }
		  if (val & DF_1_NODELETE)
		    {
		      printf (" NODELETE");
		      val ^= DF_1_NODELETE;
		    }
		  if (val & DF_1_LOADFLTR)
		    {
		      printf (" LOADFLTR");
		      val ^= DF_1_LOADFLTR;
		    }
		  if (val & DF_1_INITFIRST)
		    {
		      printf (" INITFIRST");
		      val ^= DF_1_INITFIRST;
		    }
		  if (val & DF_1_NOOPEN)
		    {
		      printf (" NOOPEN");
		      val ^= DF_1_NOOPEN;
		    }
		  if (val & DF_1_ORIGIN)
		    {
		      printf (" ORIGIN");
		      val ^= DF_1_ORIGIN;
		    }
		  if (val & DF_1_DIRECT)
		    {
		      printf (" DIRECT");
		      val ^= DF_1_DIRECT;
		    }
		  if (val & DF_1_TRANS)
		    {
		      printf (" TRANS");
		      val ^= DF_1_TRANS;
		    }
		  if (val & DF_1_INTERPOSE)
		    {
		      printf (" INTERPOSE");
		      val ^= DF_1_INTERPOSE;
		    }
		  if (val & DF_1_NODEFLIB)
		    {
		      printf (" NODEFLIB");
		      val ^= DF_1_NODEFLIB;
		    }
		  if (val & DF_1_NODUMP)
		    {
		      printf (" NODUMP");
		      val ^= DF_1_NODUMP;
		    }
		  if (val & DF_1_CONLFAT)
		    {
		      printf (" CONLFAT");
		      val ^= DF_1_CONLFAT;
		    }
		  if (val != 0)
		    printf (" %lx", val);
		  puts ("");
		}
	    }
	  break;

	case DT_PLTREL:
	  dynamic_info[entry->d_tag] = entry->d_un.d_val;
	  if (do_dynamic)
	    puts (get_dynamic_type (entry->d_un.d_val));
	  break;

	case DT_NULL	:
	case DT_NEEDED	:
	case DT_PLTGOT	:
	case DT_HASH	:
	case DT_STRTAB	:
	case DT_SYMTAB	:
	case DT_RELA	:
	case DT_INIT	:
	case DT_FINI	:
	case DT_SONAME	:
	case DT_RPATH	:
	case DT_SYMBOLIC:
	case DT_REL	:
	case DT_DEBUG	:
	case DT_TEXTREL	:
	case DT_JMPREL	:
	case DT_RUNPATH	:
	  dynamic_info[entry->d_tag] = entry->d_un.d_val;

	  if (do_dynamic)
	    {
	      char *name;

	      if (VALID_DYNAMIC_NAME (entry->d_un.d_val))
		name = GET_DYNAMIC_NAME (entry->d_un.d_val);
	      else
		name = NULL;

	      if (name)
		{
		  switch (entry->d_tag)
		    {
		    case DT_NEEDED:
		      printf (_("Shared library: [%s]"), name);

		      if (streq (name, program_interpreter))
			printf (_(" program interpreter"));
		      break;

		    case DT_SONAME:
		      printf (_("Library soname: [%s]"), name);
		      break;

		    case DT_RPATH:
		      printf (_("Library rpath: [%s]"), name);
		      break;

		    case DT_RUNPATH:
		      printf (_("Library runpath: [%s]"), name);
		      break;

		    default:
		      print_vma (entry->d_un.d_val, PREFIX_HEX);
		      break;
		    }
		}
	      else
		print_vma (entry->d_un.d_val, PREFIX_HEX);

	      putchar ('\n');
	    }
	  break;

	case DT_PLTRELSZ:
	case DT_RELASZ:
	case DT_STRSZ:
	case DT_RELSZ:
	case DT_RELAENT:
	case DT_SYMENT:
	case DT_RELENT:
	  dynamic_info[entry->d_tag] = entry->d_un.d_val;
	  ATTRIBUTE_FALLTHROUGH; /* ??? */
	case DT_PLTPADSZ:
	case DT_MOVEENT:
	case DT_MOVESZ:
	case DT_INIT_ARRAYSZ:
	case DT_FINI_ARRAYSZ:
	case DT_GNU_CONFLICTSZ:
	case DT_GNU_LIBLISTSZ:
	  if (do_dynamic)
	    {
	      print_vma (entry->d_un.d_val, UNSIGNED);
	      printf (" (bytes)\n");
	    }
	  break;

	case DT_VERDEFNUM:
	case DT_VERNEEDNUM:
	case DT_RELACOUNT:
	case DT_RELCOUNT:
	  if (do_dynamic)
	    {
	      print_vma (entry->d_un.d_val, UNSIGNED);
	      putchar ('\n');
	    }
	  break;

	case DT_SYMINSZ:
	case DT_SYMINENT:
	case DT_SYMINFO:
	case DT_USED:
	case DT_INIT_ARRAY:
	case DT_FINI_ARRAY:
	  if (do_dynamic)
	    {
	      if (entry->d_tag == DT_USED
		  && VALID_DYNAMIC_NAME (entry->d_un.d_val))
		{
		  char *name = GET_DYNAMIC_NAME (entry->d_un.d_val);

		  if (*name)
		    {
		      printf (_("Not needed object: [%s]\n"), name);
		      break;
		    }
		}

	      print_vma (entry->d_un.d_val, PREFIX_HEX);
	      putchar ('\n');
	    }
	  break;

	case DT_BIND_NOW:
	  /* The value of this entry is ignored.  */
	  if (do_dynamic)
	    putchar ('\n');
	  break;

	case DT_GNU_PRELINKED:
	  if (do_dynamic)
	    {
	      struct tm *tmp;
	      time_t time = (time_t)entry->d_un.d_val;

	      tmp = gmtime (&time);
	      printf ("%04u-%02u-%02uT%02u:%02u:%02u\n",
		      tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
		      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	    }
	  break;

	default:
	  if ((entry->d_tag >= DT_VERSYM) && (entry->d_tag <= DT_VERNEEDNUM))
	    version_info[DT_VERSIONTAGIDX (entry->d_tag)] =
	      entry->d_un.d_val;

	  if (do_dynamic)
	    {
	      switch (elf_header.e_machine)
		{
		case EM_MIPS:
		case EM_MIPS_RS3_LE:
		  dynamic_section_mips_val (entry);
		  break;
		case EM_PARISC:
		  dynamic_section_parisc_val (entry);
		  break;
		case EM_IA_64:
		  dynamic_section_ia64_val (entry);
		  break;
		default:
		  print_vma (entry->d_un.d_val, PREFIX_HEX);
		  putchar ('\n');
		}
	    }
	  break;
	}
    }

  return 1;
}

static const char *
get_ver_flags(unsigned int flags)
{
  static char buff[32];

  buff[0] = 0;

  if (flags == 0)
    return _("none");

  if (flags & VER_FLG_BASE)
    strcat(buff, "BASE ");

  if (flags & VER_FLG_WEAK)
    {
      if (flags & VER_FLG_BASE)
	strcat(buff, "| ");

      strcat(buff, "WEAK ");
    }

  if (flags & ~(VER_FLG_BASE | VER_FLG_WEAK))
    strcat (buff, "| <unknown>");

  return buff;
}

/* Display the contents of the version sections.  */
static int
process_version_sections (FILE *file)
{
  Elf_Internal_Shdr *section;
  unsigned i;
  int found = 0;

  if (! do_version)
    return 1;

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      switch (section->sh_type)
	{
	case SHT_GNU_verdef:
	  {
	    Elf_External_Verdef *edefs;
	    unsigned int idx;
	    unsigned int cnt;

	    found = 1;

	    printf(_("\nVersion definition section '%s' contains %ld entries:\n"),
                   SECTION_NAME(section), section->sh_info);

	    printf(_("  Addr: 0x"));
	    printf_vma(section->sh_addr);
	    printf(_("  Offset: %#08lx  Link: %lx (%s)\n"),
                   (unsigned long)section->sh_offset, section->sh_link,
                   ((SECTION_HEADER_INDEX(section->sh_link)
                     < elf_header.e_shnum)
                    ? SECTION_NAME(SECTION_HEADER(section->sh_link))
                    : "<corrupt>"));

	    edefs = (Elf_External_Verdef *)get_data(NULL, file,
                                                    section->sh_offset, 1,
                                                    section->sh_size,
                                                    _("version definition section"));
	    if (!edefs) {
	      break;
            }

	    for (idx = cnt = 0; cnt < section->sh_info; ++cnt)
	      {
		char *vstart;
		Elf_External_Verdef *edef;
		Elf_Internal_Verdef ent;
		Elf_External_Verdaux *eaux;
		Elf_Internal_Verdaux aux;
		int j;
		int isum;

		vstart = (((char *)edefs) + idx);

		edef = (Elf_External_Verdef *)vstart;

		ent.vd_version = BYTE_GET(edef->vd_version);
		ent.vd_flags = BYTE_GET(edef->vd_flags);
		ent.vd_ndx = BYTE_GET(edef->vd_ndx);
		ent.vd_cnt = BYTE_GET(edef->vd_cnt);
		ent.vd_hash = BYTE_GET(edef->vd_hash);
		ent.vd_aux = BYTE_GET(edef->vd_aux);
		ent.vd_next = BYTE_GET(edef->vd_next);

		printf(_("  %#06x: Rev: %d  Flags: %s"),
                       idx, ent.vd_version, get_ver_flags(ent.vd_flags));

		printf(_("  Index: %d  Cnt: %d  "),
                       ent.vd_ndx, ent.vd_cnt);

		vstart += ent.vd_aux;

		eaux = (Elf_External_Verdaux *)vstart;

		aux.vda_name = BYTE_GET(eaux->vda_name);
		aux.vda_next = BYTE_GET(eaux->vda_next);

		if (VALID_DYNAMIC_NAME(aux.vda_name)) {
		  printf(_("Name: %s\n"), GET_DYNAMIC_NAME(aux.vda_name));
		} else {
		  printf(_("Name index: %ld\n"), aux.vda_name);
                }

		isum = (idx + ent.vd_aux);

		for (j = 1; j < ent.vd_cnt; j++)
		  {
		    isum += aux.vda_next;
		    vstart += aux.vda_next;

		    eaux = (Elf_External_Verdaux *)vstart;

		    aux.vda_name = BYTE_GET(eaux->vda_name);
		    aux.vda_next = BYTE_GET(eaux->vda_next);

		    if (VALID_DYNAMIC_NAME(aux.vda_name)) {
		      printf(_("  %#06x: Parent %d: %s\n"),
                             isum, j, GET_DYNAMIC_NAME(aux.vda_name));
		    } else {
		      printf(_("  %#06x: Parent %d, name index: %ld\n"),
                             isum, j, aux.vda_name);
                    }
		  }

		idx += ent.vd_next;
	      }

	    free(edefs);
	  }
	  break;

	case SHT_GNU_verneed:
	  {
	    Elf_External_Verneed *eneed;
	    unsigned int idx;
	    unsigned int cnt;

	    found = 1;

	    printf(_("\nVersion needs section '%s' contains %ld entries:\n"),
                   SECTION_NAME(section), section->sh_info);

	    printf(_(" Addr: 0x"));
	    printf_vma(section->sh_addr);
	    printf(_("  Offset: %#08lx  Link to section: %ld (%s)\n"),
                   (unsigned long)section->sh_offset, section->sh_link,
                   ((SECTION_HEADER_INDEX(section->sh_link)
                     < elf_header.e_shnum)
                    ? SECTION_NAME(SECTION_HEADER(section->sh_link))
                    : "<corrupt>"));

	    eneed = (Elf_External_Verneed *)get_data(NULL, file,
                                                     section->sh_offset, 1,
                                                     section->sh_size,
                                                     _("version need section"));
	    if (!eneed) {
	      break;
            }

	    for (idx = cnt = 0; cnt < section->sh_info; ++cnt)
	      {
		Elf_External_Verneed *entry;
		Elf_Internal_Verneed ent;
		int j;
		int isum;
		char *vstart;

		vstart = ((char *) eneed) + idx;

		entry = (Elf_External_Verneed *) vstart;

		ent.vn_version = BYTE_GET (entry->vn_version);
		ent.vn_cnt     = BYTE_GET (entry->vn_cnt);
		ent.vn_file    = BYTE_GET (entry->vn_file);
		ent.vn_aux     = BYTE_GET (entry->vn_aux);
		ent.vn_next    = BYTE_GET (entry->vn_next);

		printf (_("  %#06x: Version: %d"), idx, ent.vn_version);

		if (VALID_DYNAMIC_NAME (ent.vn_file))
		  printf (_("  File: %s"), GET_DYNAMIC_NAME (ent.vn_file));
		else
		  printf (_("  File: %lx"), ent.vn_file);

		printf (_("  Cnt: %d\n"), ent.vn_cnt);

		vstart += ent.vn_aux;

		for (j = 0, isum = idx + ent.vn_aux; j < ent.vn_cnt; ++j)
		  {
		    Elf_External_Vernaux *eaux;
		    Elf_Internal_Vernaux aux;

		    eaux = (Elf_External_Vernaux *) vstart;

		    aux.vna_hash  = BYTE_GET (eaux->vna_hash);
		    aux.vna_flags = BYTE_GET (eaux->vna_flags);
		    aux.vna_other = BYTE_GET (eaux->vna_other);
		    aux.vna_name  = BYTE_GET (eaux->vna_name);
		    aux.vna_next  = BYTE_GET (eaux->vna_next);

		    if (VALID_DYNAMIC_NAME (aux.vna_name))
		      printf (_("  %#06x:   Name: %s"),
			      isum, GET_DYNAMIC_NAME (aux.vna_name));
		    else
		      printf (_("  %#06x:   Name index: %lx"),
			      isum, aux.vna_name);

		    printf (_("  Flags: %s  Version: %d\n"),
			    get_ver_flags (aux.vna_flags), aux.vna_other);

		    isum   += aux.vna_next;
		    vstart += aux.vna_next;
		  }

		idx += ent.vn_next;
	      }

	    free (eneed);
	  }
	  break;

	case SHT_GNU_versym:
	  {
	    Elf_Internal_Shdr *link_section;
	    int total;
	    int cnt;
	    unsigned char *edata;
	    unsigned short *data;
	    char *strtab;
	    Elf_Internal_Sym *symbols;
	    Elf_Internal_Shdr *string_sec;
	    long off;

	    if (SECTION_HEADER_INDEX (section->sh_link) >= elf_header.e_shnum)
	      break;

	    link_section = SECTION_HEADER (section->sh_link);
	    total = section->sh_size / sizeof (Elf_External_Versym);

	    if (SECTION_HEADER_INDEX (link_section->sh_link)
		>= elf_header.e_shnum)
	      break;

	    found = 1;

	    symbols = GET_ELF_SYMBOLS (file, link_section);

	    string_sec = SECTION_HEADER (link_section->sh_link);

	    strtab = (char *)get_data(NULL, file, string_sec->sh_offset, 1,
                                      string_sec->sh_size,
                                      _("version string table"));
	    if (!strtab)
	      break;

	    printf(_("\nVersion symbols section '%s' contains %d entries:\n"),
                   SECTION_NAME(section), total);

	    printf(_(" Addr: "));
	    printf_vma(section->sh_addr);
	    printf(_("  Offset: %#08lx  Link: %lx (%s)\n"),
                   (unsigned long)section->sh_offset, section->sh_link,
                   SECTION_NAME(link_section));

	    off = offset_from_vma(file,
                                  version_info[DT_VERSIONTAGIDX(DT_VERSYM)],
                                  total * sizeof(short));
	    edata = (unsigned char *)get_data(NULL, file, off, total,
                                              sizeof(short),
                                              _("version symbol data"));
	    if (!edata)
	      {
		free(strtab);
		break;
	      }

	    data = (unsigned short *)cmalloc(total, sizeof(short));

	    for (cnt = total; cnt --;)
	      data[cnt] = byte_get(edata + cnt * sizeof(short),
                                   sizeof(short));

	    free(edata);

	    for (cnt = 0; cnt < total; cnt += 4)
	      {
		int j, nn;
		int check_def, check_need;
		char *name;

		printf ("  %03x:", cnt);

		for (j = 0; (j < 4) && (cnt + j) < total; ++j)
		  switch (data[cnt + j])
		    {
		    case 0:
		      fputs (_("   0 (*local*)    "), stdout);
		      break;

		    case 1:
		      fputs (_("   1 (*global*)   "), stdout);
		      break;

		    default:
		      nn = printf ("%4x%c", data[cnt + j] & 0x7fff,
				   data[cnt + j] & 0x8000 ? 'h' : ' ');

		      check_def = 1;
		      check_need = 1;
		      if (SECTION_HEADER_INDEX (symbols[cnt + j].st_shndx)
			  >= elf_header.e_shnum
			  || SECTION_HEADER (symbols[cnt + j].st_shndx)->sh_type
			     != SHT_NOBITS)
			{
			  if (symbols[cnt + j].st_shndx == SHN_UNDEF)
			    check_def = 0;
			  else
			    check_need = 0;
			}

		      if (check_need
			  && version_info[DT_VERSIONTAGIDX (DT_VERNEED)])
			{
			  Elf_Internal_Verneed ivn;
			  unsigned long offset;

			  offset = offset_from_vma
			    (file, version_info[DT_VERSIONTAGIDX (DT_VERNEED)],
			     sizeof (Elf_External_Verneed));

			  do
			    {
			      Elf_Internal_Vernaux ivna;
			      Elf_External_Verneed evn;
			      Elf_External_Vernaux evna;
			      unsigned long a_off;

			      get_data (&evn, file, offset, sizeof (evn), 1,
					_("version need"));

			      ivn.vn_aux  = BYTE_GET (evn.vn_aux);
			      ivn.vn_next = BYTE_GET (evn.vn_next);

			      a_off = offset + ivn.vn_aux;

			      do {
				  get_data(&evna, file, a_off, sizeof(evna),
                                           1, _("version need aux (2)"));

				  ivna.vna_next  = BYTE_GET(evna.vna_next);
				  ivna.vna_other = BYTE_GET(evna.vna_other);

				  a_off += ivna.vna_next;
                              } while ((ivna.vna_other != data[cnt + j])
                                       && (ivna.vna_next != 0));

			      if (ivna.vna_other == data[cnt + j])
				{
				  ivna.vna_name = BYTE_GET(evna.vna_name);

				  name = strtab + ivna.vna_name;
				  nn += printf("(%s%-*s", name,
					       (12 - (int8_t)strlen(name)),
					       ")");
				  check_def = 0;
				  break;
				}

			      offset += ivn.vn_next;
			    }
			  while (ivn.vn_next);
			}

		      if (check_def && (data[cnt + j] != 0x8001)
			  && version_info[DT_VERSIONTAGIDX(DT_VERDEF)])
			{
			  Elf_Internal_Verdef ivd;
			  Elf_External_Verdef evd;
			  unsigned long offset;

			  offset
                            = offset_from_vma(file,
                                              version_info[DT_VERSIONTAGIDX(DT_VERDEF)],
                                              sizeof(evd));

			  do {
			      get_data (&evd, file, offset, sizeof(evd), 1,
					_("version def"));

			      ivd.vd_next = BYTE_GET (evd.vd_next);
			      ivd.vd_ndx  = BYTE_GET (evd.vd_ndx);

			      offset += ivd.vd_next;
                          } while ((ivd.vd_ndx != (data[cnt + j] & 0x7fff))
                                   && (ivd.vd_next != 0));

			  if (ivd.vd_ndx == (data[cnt + j] & 0x7fff))
			    {
			      Elf_External_Verdaux evda;
			      Elf_Internal_Verdaux ivda;

			      ivd.vd_aux = BYTE_GET (evd.vd_aux);

			      get_data (&evda, file,
					offset - ivd.vd_next + ivd.vd_aux,
					sizeof(evda), 1,
					_("version def aux"));

			      ivda.vda_name = BYTE_GET(evda.vda_name);

			      name = (strtab + ivda.vda_name);
			      nn += printf("(%s%-*s", name,
					   (12 - (int8_t)strlen(name)), ")");
			    }
			}

		      if (nn < 18)
			printf("%*c", (18 - (int8_t)nn), ' ');
		    }

		putchar('\n');
	      }

	    free(data);
	    free(strtab);
	    free(symbols);
	  }
	  break;

	default:
	  break;
	}
    }

  if (! found)
    printf(_("\nNo version information found in this file.\n"));

  return 1;
}

static const char *
get_symbol_binding(unsigned int binding)
{
  static char buff[32];

  switch (binding)
    {
    case STB_LOCAL:	return "LOCAL";
    case STB_GLOBAL:	return "GLOBAL";
    case STB_WEAK:	return "WEAK";
    default:
      if (binding >= STB_LOPROC && binding <= STB_HIPROC)
	snprintf (buff, sizeof (buff), _("<processor specific>: %d"),
		  binding);
      else if (binding >= STB_LOOS && binding <= STB_HIOS)
	snprintf (buff, sizeof (buff), _("<OS specific>: %d"), binding);
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %d"), binding);
      return buff;
    }
}

static const char *
get_symbol_type (unsigned int type)
{
  static char buff[34]; /* 32 + 2, for -Wformat-truncation */

  switch (type)
    {
    case STT_NOTYPE:	return "NOTYPE";
    case STT_OBJECT:	return "OBJECT";
    case STT_FUNC:	return "FUNC";
    case STT_SECTION:	return "SECTION";
    case STT_FILE:	return "FILE";
    case STT_COMMON:	return "COMMON";
    case STT_TLS:	return "TLS";
    default:
      if (type >= STT_LOPROC && type <= STT_HIPROC)
	{
	  if (elf_header.e_machine == EM_ARM && type == STT_ARM_TFUNC)
	    return "THUMB_FUNC";

	  if (elf_header.e_machine == EM_SPARCV9 && type == STT_REGISTER)
	    return "REGISTER";

	  if (elf_header.e_machine == EM_PARISC && type == STT_PARISC_MILLI)
	    return "PARISC_MILLI";

	  snprintf(buff, sizeof(buff), _("<processor specific>: %d"), type);
	}
      else if (type >= STT_LOOS && type <= STT_HIOS)
	{
	  if (elf_header.e_machine == EM_PARISC)
	    {
	      if (type == STT_HP_OPAQUE)
		return "HP_OPAQUE";
	      if (type == STT_HP_STUB)
		return "HP_STUB";
	    }

	  snprintf (buff, sizeof (buff), _("<OS specific>: %d"), type);
	}
      else
	snprintf (buff, sizeof (buff), _("<unknown>: %d"), type);
      return buff;
    }
}

static const char *
get_symbol_visibility (unsigned int visibility)
{
  switch (visibility)
    {
    case STV_DEFAULT:	return "DEFAULT";
    case STV_INTERNAL:	return "INTERNAL";
    case STV_HIDDEN:	return "HIDDEN";
    case STV_PROTECTED: return "PROTECTED";
    default: abort ();
    }
}

static const char *
get_symbol_index_type (unsigned int type)
{
  static char buff[32];

  switch (type)
    {
    case SHN_UNDEF:	return "UND";
    case SHN_ABS:	return "ABS";
    case SHN_COMMON:	return "COM";
    default:
      if (type == SHN_IA_64_ANSI_COMMON
	  && elf_header.e_machine == EM_IA_64
	  && elf_header.e_ident[EI_OSABI] == ELFOSABI_HPUX)
	return "ANSI_COM";
      else if (elf_header.e_machine == EM_X86_64
	       && type == SHN_X86_64_LCOMMON)
	return "LARGE_COM";
      else if (type >= SHN_LOPROC && type <= SHN_HIPROC)
	sprintf (buff, "PRC[0x%04x]", type);
      else if (type >= SHN_LOOS && type <= SHN_HIOS)
	sprintf (buff, "OS [0x%04x]", type);
      else if (type >= SHN_LORESERVE && type <= SHN_HIRESERVE)
	sprintf (buff, "RSV[0x%04x]", type);
      else
	sprintf (buff, "%3d", type);
      break;
    }

  return buff;
}

static bfd_vma *
get_dynamic_data(FILE *file, unsigned int number, unsigned int ent_size)
{
  unsigned char *e_data;
  bfd_vma *i_data;

  e_data = (unsigned char *)cmalloc(number, ent_size);

  if (e_data == NULL)
    {
      error(_("Out of memory\n"));
      return NULL;
    }

  if (fread(e_data, ent_size, number, file) != number)
    {
      error(_("Unable to read in dynamic data\n"));
      return NULL;
    }

  i_data = (bfd_vma *)cmalloc(number, sizeof(*i_data));

  if (i_data == NULL)
    {
      error(_("Out of memory\n"));
      free(e_data);
      return NULL;
    }

  while (number--)
    i_data[number] = byte_get(e_data + number * ent_size, ent_size);

  free(e_data);

  return i_data;
}

/* Dump the symbol table: */
static int
process_symbol_table(FILE *file)
{
  Elf_Internal_Shdr *section;
  bfd_vma nbuckets = 0;
  bfd_vma nchains = 0;
  bfd_vma *buckets = NULL;
  bfd_vma *chains = NULL;

  if (! do_syms && !do_histogram)
    return 1;

  if (dynamic_info[DT_HASH] && ((do_using_dynamic && (dynamic_strings != NULL))
				|| do_histogram))
    {
      unsigned char nb[8];
      unsigned char nc[8];
      int hash_ent_size = 4;

      if ((elf_header.e_machine == EM_ALPHA
	   || elf_header.e_machine == EM_S390
	   || elf_header.e_machine == EM_S390_OLD)
	  && elf_header.e_ident[EI_CLASS] == ELFCLASS64)
	hash_ent_size = 8;

      if (fseek(file,
                (archive_file_offset
                 + offset_from_vma(file, dynamic_info[DT_HASH],
                                   (sizeof(nb) + sizeof(nc)))),
                SEEK_SET))
	{
	  error(_("Unable to seek to start of dynamic information"));
	  return 0;
	}

      if (fread(nb, hash_ent_size, 1, file) != 1)
	{
	  error(_("Failed to read in number of buckets\n"));
	  return 0;
	}

      if (fread(nc, hash_ent_size, 1, file) != 1)
	{
	  error(_("Failed to read in number of chains\n"));
	  return 0;
	}

      nbuckets = byte_get(nb, hash_ent_size);
      nchains = byte_get(nc, hash_ent_size);

      buckets = get_dynamic_data(file, nbuckets, hash_ent_size);
      if (buckets == NULL)
	return 0;
      chains = get_dynamic_data(file, nchains, hash_ent_size);
      if (chains == NULL)
        {
          free(buckets);
	  return 0;
        }
    }

  if (do_syms && dynamic_info[DT_HASH] && do_using_dynamic
      && (dynamic_strings != NULL))
    {
      unsigned long hn;
      bfd_vma si;

      printf (_("\nSymbol table for image:\n"));
      if (is_32bit_elf)
	printf (_("  Num Buc:    Value  Size   Type   Bind Vis      Ndx Name\n"));
      else
	printf (_("  Num Buc:    Value          Size   Type   Bind Vis      Ndx Name\n"));

      for (hn = 0; hn < nbuckets; hn++)
	{
	  if (! buckets[hn])
	    continue;

	  for (si = buckets[hn]; si < nchains && si > 0; si = chains[si])
	    {
	      Elf_Internal_Sym *psym;
	      int n;

	      psym = (dynamic_symbols + si);

	      n = print_vma(si, DEC_5);
	      if (n < 5)
		fputs(&"     "[n], stdout);
	      printf(" %3lu: ", hn);
	      print_vma(psym->st_value, LONG_HEX);
	      putchar(' ');
	      print_vma(psym->st_size, DEC_5);

	      printf("  %6s", get_symbol_type(ELF_ST_TYPE(psym->st_info)));
	      printf(" %6s",  get_symbol_binding(ELF_ST_BIND(psym->st_info)));
	      printf(" %3s",  get_symbol_visibility(ELF_ST_VISIBILITY(psym->st_other)));
	      printf(" %3.3s ", get_symbol_index_type(psym->st_shndx));
	      if (VALID_DYNAMIC_NAME(psym->st_name))
		print_symbol(25, GET_DYNAMIC_NAME(psym->st_name));
	      else
		printf(" <corrupt: %14ld>", psym->st_name);
	      putchar('\n');
	    }
	}
    }
  else if (do_syms && !do_using_dynamic)
    {
      unsigned int i;

      for (i = 0, section = section_headers;
	   i < elf_header.e_shnum;
	   i++, section++)
	{
	  unsigned int si;
	  char *strtab = NULL;
	  unsigned long int strtab_size = 0;
	  Elf_Internal_Sym *symtab;
	  Elf_Internal_Sym *psym;


	  if (   section->sh_type != SHT_SYMTAB
	      && section->sh_type != SHT_DYNSYM)
	    continue;

	  printf (_("\nSymbol table '%s' contains %lu entries:\n"),
		  SECTION_NAME (section),
		  (unsigned long) (section->sh_size / section->sh_entsize));
	  if (is_32bit_elf)
	    printf (_("   Num:    Value  Size Type    Bind   Vis      Ndx Name\n"));
	  else
	    printf (_("   Num:    Value          Size Type    Bind   Vis      Ndx Name\n"));

	  symtab = GET_ELF_SYMBOLS (file, section);
	  if (symtab == NULL)
	    continue;

	  if (section->sh_link == elf_header.e_shstrndx)
	    {
	      strtab = string_table;
	      strtab_size = string_table_length;
	    }
	  else if (SECTION_HEADER_INDEX(section->sh_link) < elf_header.e_shnum)
	    {
	      Elf_Internal_Shdr *string_sec;

	      string_sec = SECTION_HEADER(section->sh_link);

	      strtab = (char *)get_data(NULL, file, string_sec->sh_offset,
                                        1, string_sec->sh_size,
                                        _("string table"));
	      strtab_size = ((strtab != NULL) ? string_sec->sh_size : 0);
	    }

	  for (si = 0, psym = symtab;
	       si < (section->sh_size / section->sh_entsize);
	       si++, psym++)
	    {
	      printf("%6d: ", si);
	      print_vma(psym->st_value, LONG_HEX);
	      putchar(' ');
	      print_vma(psym->st_size, DEC_5);
	      printf(" %-7s", get_symbol_type(ELF_ST_TYPE(psym->st_info)));
	      printf(" %-6s", get_symbol_binding(ELF_ST_BIND(psym->st_info)));
	      printf(" %-3s", get_symbol_visibility(ELF_ST_VISIBILITY(psym->st_other)));
	      printf(" %4s ", get_symbol_index_type(psym->st_shndx));
	      print_symbol(25, ((psym->st_name < strtab_size)
                                ? (strtab + psym->st_name) : "<corrupt>"));

	      if ((section->sh_type == SHT_DYNSYM) &&
		  version_info[DT_VERSIONTAGIDX(DT_VERSYM)] != 0)
		{
		  unsigned char data[2];
		  unsigned short vers_data;
		  unsigned long an_offset;
		  int is_nobits;
		  int check_def;

		  an_offset
                    = offset_from_vma(file,
                                      version_info[DT_VERSIONTAGIDX(DT_VERSYM)],
                                      sizeof(data) + si * sizeof(vers_data));

		  get_data(&data, file, an_offset + si * sizeof(vers_data),
                           sizeof(data), 1, _("version data"));

		  vers_data = byte_get(data, 2);

		  is_nobits = ((SECTION_HEADER_INDEX(psym->st_shndx)
                                < elf_header.e_shnum)
			       && (SECTION_HEADER(psym->st_shndx)->sh_type
                                   == SHT_NOBITS));

		  check_def = (psym->st_shndx != SHN_UNDEF);

		  if ((vers_data & 0x8000) || (vers_data > 1))
		    {
		      if (version_info[DT_VERSIONTAGIDX (DT_VERNEED)]
			  && (is_nobits || ! check_def))
			{
			  Elf_External_Verneed evn;
			  Elf_Internal_Verneed ivn;
			  Elf_Internal_Vernaux ivna;

			  /* We must test both: */
			  an_offset
                            = offset_from_vma(file,
                                              version_info[DT_VERSIONTAGIDX (DT_VERNEED)],
                                              sizeof(evn));

			  do {
			      unsigned long vna_off;

			      get_data(&evn, file, an_offset, sizeof(evn),
                                       1, _("version need"));

			      ivn.vn_aux = BYTE_GET(evn.vn_aux);
			      ivn.vn_next = BYTE_GET(evn.vn_next);

			      vna_off = (an_offset + ivn.vn_aux);

			      do {
				  Elf_External_Vernaux evna;

				  get_data (&evna, file, vna_off,
					    sizeof (evna), 1,
					    _("version need aux (3)"));

				  ivna.vna_other = BYTE_GET(evna.vna_other);
				  ivna.vna_next = BYTE_GET(evna.vna_next);
				  ivna.vna_name = BYTE_GET(evna.vna_name);

				  vna_off += ivna.vna_next;
                              } while ((ivna.vna_other != vers_data)
                                       && (ivna.vna_next != 0));

			      if (ivna.vna_other == vers_data)
				break;

			      an_offset += ivn.vn_next;
                          } while (ivn.vn_next != 0);

			  if (ivna.vna_other == vers_data)
			    {
			      printf("@%s (%d)",
                                     (ivna.vna_name < strtab_size)
                                     ? (strtab + ivna.vna_name)
                                     : "<corrupt>", ivna.vna_other);
			      check_def = 0;
			    }
			  else if (! is_nobits)
			    error(_("bad dynamic symbol"));
			  else
			    check_def = 1;
			}

		      if (check_def)
			{
			  if ((vers_data != 0x8001)
			      && version_info[DT_VERSIONTAGIDX(DT_VERDEF)])
			    {
			      Elf_Internal_Verdef ivd;
			      Elf_Internal_Verdaux ivda;
			      Elf_External_Verdaux evda;
			      unsigned long offset_amt;

			      offset_amt
                                = offset_from_vma(file,
                                                  version_info[DT_VERSIONTAGIDX(DT_VERDEF)],
                                                  sizeof(Elf_External_Verdef));

			      do {
				  Elf_External_Verdef evd;

				  get_data(&evd, file, offset_amt, sizeof(evd),
                                           1, _("version def"));

				  ivd.vd_ndx = BYTE_GET(evd.vd_ndx);
				  ivd.vd_aux = BYTE_GET(evd.vd_aux);
				  ivd.vd_next = BYTE_GET(evd.vd_next);

				  offset_amt += ivd.vd_next;
                              } while ((ivd.vd_ndx != (vers_data & 0x7fff))
                                       && (ivd.vd_next != 0));

			      offset_amt -= ivd.vd_next;
			      offset_amt += ivd.vd_aux;

			      get_data(&evda, file, offset_amt,
                                       sizeof(evda), 1,
                                       _("version def aux"));

			      ivda.vda_name = BYTE_GET(evda.vda_name);

			      if (psym->st_name != ivda.vda_name) {
				printf((vers_data & 0x8000)
                                       ? "@%s" : "@@%s",
                                       (ivda.vda_name < strtab_size)
                                       ? (strtab + ivda.vda_name)
                                       : "<corrupt>");
                              }
			    }
			}
		    }
		}

	      putchar('\n');
	    }

	  free (symtab);
	  if (strtab != string_table)
	    free (strtab);
	}
    }
  else if (do_syms)
    printf
      (_("\nDynamic symbol information is not available for displaying symbols.\n"));

  if (do_histogram && buckets != NULL)
    {
      unsigned long *lengths;
      unsigned long *counts;
      unsigned long hn;
      bfd_vma si;
      unsigned long maxlength = 0UL;
      unsigned long nzero_counts = 0UL;
      unsigned long nsyms = 0UL;

      printf(_("\nHistogram for bucket list length (total of %lu buckets):\n"),
             (unsigned long)nbuckets);
      printf(_(" Length  Number     %% of total  Coverage\n"));

      lengths = (unsigned long *)calloc(nbuckets, sizeof(*lengths));
      if (lengths == NULL)
	{
          free(buckets);
          free(chains);
	  error(_("Out of memory"));
	  return 0;
	}
      for (hn = 0; hn < nbuckets; ++hn)
	{
	  for (si = buckets[hn]; (si > 0) && (si < nchains); si = chains[si])
	    {
	      ++nsyms;
	      if (maxlength < ++lengths[hn])
		++maxlength;
	    }
	}

      counts = (unsigned long *)calloc((maxlength + 1), sizeof(*counts));
      if (counts == NULL)
	{
	  free(buckets);
   	  free(chains);
          free(lengths);
	  error(_("Out of memory"));
	  return 0;
	}

      for (hn = 0; hn < nbuckets; ++hn)
	++counts[lengths[hn]];

      if (nbuckets > 0)
	{
	  unsigned long i;
	  printf("      0  %-10lu (%5.1f%%)\n",
                 counts[0], (counts[0] * (double)100.0f) / nbuckets);
	  for (i = 1UL; (i <= maxlength) && (i < ULONG_MAX); ++i)
	    {
	      nzero_counts += (counts[i] * i);
	      printf("%7lu  %-10lu (%5.1f%%)    %5.1f%%\n",
                     i, counts[i], (counts[i] * (double)100.0f) / nbuckets,
                     (nzero_counts * (double)100.0f) / nsyms);
              if (i == (unsigned long)INFINITY) {
                break; /* attempt to silence '-Wunsafe-loop-optimizations' */
              }
	    }
	}

      free (counts);
      free (lengths);
    }

  if (buckets != NULL)
    {
      free (buckets);
      free (chains);
    }

  return 1;
}

static int
process_syminfo (FILE *file ATTRIBUTE_UNUSED)
{
  unsigned int i;

  if (dynamic_syminfo == NULL
      || !do_dynamic)
    /* No syminfo, this is ok.  */
    return 1;

  /* There better should be a dynamic symbol section.  */
  if (dynamic_symbols == NULL || dynamic_strings == NULL)
    return 0;

  if (dynamic_addr)
    printf (_("\nDynamic info segment at offset 0x%lx contains %d entries:\n"),
	    dynamic_syminfo_offset, dynamic_syminfo_nent);

  printf (_(" Num: Name                           BoundTo     Flags\n"));
  for (i = 0; i < dynamic_syminfo_nent; ++i)
    {
      unsigned short int flags = dynamic_syminfo[i].si_flags;

      printf ("%4d: ", i);
      if (VALID_DYNAMIC_NAME (dynamic_symbols[i].st_name))
	print_symbol (30, GET_DYNAMIC_NAME (dynamic_symbols[i].st_name));
      else
	printf ("<corrupt: %19ld>", dynamic_symbols[i].st_name);
      putchar (' ');

      switch (dynamic_syminfo[i].si_boundto)
	{
	case SYMINFO_BT_SELF:
	  fputs ("SELF       ", stdout);
	  break;
	case SYMINFO_BT_PARENT:
	  fputs ("PARENT     ", stdout);
	  break;
	default:
	  if (dynamic_syminfo[i].si_boundto > 0
	      && dynamic_syminfo[i].si_boundto < dynamic_nent
	      && VALID_DYNAMIC_NAME (dynamic_section[dynamic_syminfo[i].si_boundto].d_un.d_val))
	    {
	      print_symbol (10, GET_DYNAMIC_NAME (dynamic_section[dynamic_syminfo[i].si_boundto].d_un.d_val));
	      putchar (' ' );
	    }
	  else
	    printf ("%-10d ", dynamic_syminfo[i].si_boundto);
	  break;
	}

      if (flags & SYMINFO_FLG_DIRECT)
	printf (" DIRECT");
      if (flags & SYMINFO_FLG_PASSTHRU)
	printf (" PASSTHRU");
      if (flags & SYMINFO_FLG_COPY)
	printf (" COPY");
      if (flags & SYMINFO_FLG_LAZYLOAD)
	printf (" LAZYLOAD");

      puts ("");
    }

  return 1;
}

#ifdef SUPPORT_DISASSEMBLY
static int
disassemble_section (Elf_Internal_Shdr *section, FILE *file)
{
  printf (_("\nAssembly dump of section %s\n"),
	  SECTION_NAME (section));

  /* XXX -- to be done --- XXX */

  return 1;
}
#endif /* SUPPORT_DISASSEMBLY */

static int
dump_section (Elf_Internal_Shdr *section, FILE *file)
{
  bfd_size_type bytes;
  bfd_vma addr;
  unsigned char *data;
  unsigned char *start;

  bytes = section->sh_size;

  if (bytes == 0 || section->sh_type == SHT_NOBITS)
    {
      printf (_("\nSection '%s' has no data to dump.\n"),
	      SECTION_NAME (section));
      return 0;
    }
  else
    printf (_("\nHex dump of section '%s':\n"), SECTION_NAME (section));

  addr = section->sh_addr;

  start = (unsigned char *)get_data(NULL, file, section->sh_offset, 1,
                                    bytes, _("section data"));
  if (!start)
    return 0;

  data = start;

  while (bytes)
    {
      int j;
      int k;
      int lbytes;

      lbytes = (bytes > 16 ? 16 : bytes);

      printf ("  0x%8.8lx ", (unsigned long) addr);

      switch (elf_header.e_ident[EI_DATA])
	{
	default:
	case ELFDATA2LSB:
	  for (j = 15; j >= 0; j --)
	    {
	      if (j < lbytes)
		printf ("%2.2x", data[j]);
	      else
		printf ("  ");

	      if (!(j & 0x3))
		printf (" ");
	    }
	  break;

	case ELFDATA2MSB:
	  for (j = 0; j < 16; j++)
	    {
	      if (j < lbytes)
		printf ("%2.2x", data[j]);
	      else
		printf ("  ");

	      if ((j & 3) == 3)
		printf (" ");
	    }
	  break;
	}

      for (j = 0; j < lbytes; j++)
	{
	  k = data[j];
	  if (k >= ' ' && k < 0x7f)
	    printf ("%c", k);
	  else
	    printf (".");
	}

      putchar ('\n');

      data  += lbytes;
      addr  += lbytes;
      bytes -= lbytes;
    }

  free (start);

  return 1;
}


static unsigned long int
read_leb128 (unsigned char *data, unsigned int *length_return, int sign)
{
  unsigned long int result = 0;
  unsigned int num_read = 0;
  unsigned int shift = 0;
  unsigned char byte;

  do {
    byte = *data++;
    num_read++;

    result |= ((unsigned long int)(byte & 0x7f)) << shift;

    shift += 7;
  } while (byte & 0x80);

  if (length_return != NULL)
    *length_return = num_read;

  if (sign && (shift < 8 * sizeof (result)) && (byte & 0x40))
    result |= -1UL << shift;

  return result;
}

typedef struct State_Machine_Registers
{
  unsigned long address;
  unsigned int file;
  unsigned int line;
  unsigned int column;
  int is_stmt;
  int basic_block;
  int end_sequence;
/* This variable hold the number of the last entry seen
   in the File Table.  */
  unsigned int last_file_entry;
} SMR;

static SMR state_machine_regs;

static void
reset_state_machine (int is_stmt)
{
  state_machine_regs.address = 0;
  state_machine_regs.file = 1;
  state_machine_regs.line = 1;
  state_machine_regs.column = 0;
  state_machine_regs.is_stmt = is_stmt;
  state_machine_regs.basic_block = 0;
  state_machine_regs.end_sequence = 0;
  state_machine_regs.last_file_entry = 0;
}

/* Handled an extend line op.
   Returns the number of bytes read.  */

static int
process_extended_line_op (unsigned char *data, int is_stmt, int pointer_size)
{
  unsigned char op_code;
  unsigned int bytes_read;
  unsigned int len;
  unsigned char *name;
  unsigned long adr;

  len = read_leb128 (data, & bytes_read, 0);
  data += bytes_read;

  if (len == 0)
    {
      warn (_("badly formed extended line op encountered!\n"));
      return bytes_read;
    }

  len += bytes_read;
  op_code = *data++;

  printf (_("  Extended opcode %d: "), op_code);

  switch (op_code)
    {
    case DW_LNE_end_sequence:
      printf (_("End of Sequence\n\n"));
      reset_state_machine (is_stmt);
      break;

    case DW_LNE_set_address:
      adr = byte_get (data, pointer_size);
      printf (_("set Address to 0x%lx\n"), adr);
      state_machine_regs.address = adr;
      break;

    case DW_LNE_define_file:
      printf (_("  define new File Table entry\n"));
      printf (_("  Entry\tDir\tTime\tSize\tName\n"));

      printf (_("   %d\t"), ++state_machine_regs.last_file_entry);
      name = data;
      data += strlen ((char *) data) + 1;
      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      data += bytes_read;
      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      data += bytes_read;
      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
      printf (_("%s\n\n"), name);
      break;

    default:
      printf (_("UNKNOWN: length %d\n"), len - bytes_read);
      break;
    }

  return len;
}

static const char *debug_str_contents;
static bfd_vma debug_str_size;

static void
load_debug_str(FILE *file)
{
  Elf_Internal_Shdr *sec;

  /* If it is already loaded, then do nothing: */
  if (debug_str_contents != NULL)
    return;

  /* Locate the .debug_str section: */
  sec = find_section(".debug_str");
  if (sec == NULL)
    return;

  debug_str_size = sec->sh_size;

  debug_str_contents = (const char *)get_data(NULL, file, sec->sh_offset,
                                              1, sec->sh_size,
                                              _("debug_str section data"));
}

static void
free_debug_str (void)
{
  if (debug_str_contents == NULL)
    return;

  free ((char *) debug_str_contents);
  debug_str_contents = NULL;
  debug_str_size = 0;
}

static const char *
fetch_indirect_string (unsigned long offset)
{
  if (debug_str_contents == NULL)
    return _("<no .debug_str section>");

  if (offset > debug_str_size)
    {
      warn (_("DW_FORM_strp offset too big: %lx\n"), offset);
      return _("<offset is too big>");
    }

  return debug_str_contents + offset;
}

static const char *debug_loc_contents;
static bfd_vma debug_loc_size;

static void
load_debug_loc(FILE *file)
{
  Elf_Internal_Shdr *sec;

  /* If it is already loaded, then do nothing: */
  if (debug_loc_contents != NULL)
    return;

  /* Locate the .debug_loc section: */
  sec = find_section(".debug_loc");
  if (sec == NULL)
    return;

  debug_loc_size = sec->sh_size;

  debug_loc_contents = (const char *)get_data(NULL, file, sec->sh_offset,
                                              1, sec->sh_size,
                                              _("debug_loc section data"));
}

static void
free_debug_loc(void)
{
  if (debug_loc_contents == NULL)
    return;

  free((char *)debug_loc_contents);
  debug_loc_contents = NULL;
  debug_loc_size = 0;
}

static const char *debug_range_contents;
static unsigned long debug_range_size;

static void
load_debug_range(FILE *file)
{
  Elf_Internal_Shdr *sec;

  /* If it is already loaded, then do nothing: */
  if (debug_range_contents != NULL)
    return;

  /* Locate the .debug_ranges section: */
  sec = find_section(".debug_ranges");
  if (sec == NULL)
    return;

  debug_range_size = sec->sh_size;

  debug_range_contents = (const char *)get_data(NULL, file, sec->sh_offset,
                                                1, sec->sh_size,
                                                _("debug_range section data"));
}

static void
free_debug_range (void)
{
  if (debug_range_contents == NULL)
    return;

  free ((char *) debug_range_contents);
  debug_range_contents = NULL;
  debug_range_size = 0;
}

/* Apply addends of RELA relocations.  */

static int
debug_apply_rela_addends (FILE *file,
			  Elf_Internal_Shdr *section,
			  int reloc_size,
			  unsigned char *sec_data,
			  unsigned char *start,
			  unsigned char *end)
{
  Elf_Internal_Shdr *relsec;

  if ((end - start) < reloc_size)
    return 1;

  for (relsec = section_headers;
       relsec < (section_headers + elf_header.e_shnum);
       ++relsec)
    {
      unsigned long nrelas;
      Elf_Internal_Rela *rela, *rp;
      Elf_Internal_Shdr *symsec;
      Elf_Internal_Sym *symtab = NULL;
      Elf_Internal_Sym *sym = NULL;

      if ((relsec->sh_type != SHT_RELA)
	  || (SECTION_HEADER_INDEX(relsec->sh_info) >= elf_header.e_shnum)
	  || (SECTION_HEADER(relsec->sh_info) != section)
	  || (relsec->sh_size == 0)
	  || (SECTION_HEADER_INDEX(relsec->sh_link) >= elf_header.e_shnum))
	continue;

      if (!slurp_rela_relocs(file, relsec->sh_offset, relsec->sh_size,
			     &rela, &nrelas))
	return 0;

      symsec = SECTION_HEADER(relsec->sh_link);
      symtab = GET_ELF_SYMBOLS(file, symsec);

      for (rp = rela; rp < (rela + nrelas); ++rp)
	{
	  unsigned char *loc;

	  if ((rp->r_offset >= (bfd_vma)(start - sec_data))
	      && (rp->r_offset < ((bfd_vma)(end - sec_data) - reloc_size)))
	    loc = (sec_data + rp->r_offset);
	  else
	    continue;

	  if (is_32bit_elf)
	    {
	      sym = (symtab + ELF32_R_SYM(rp->r_info));

	      if ((ELF32_R_SYM(rp->r_info) != 0)
		  && (ELF32_ST_TYPE(sym->st_info) != STT_SECTION)
		  /* Relocations against object symbols can happen,
		     eg when referencing a global array. For an
		     example of this see the _clz.o binary in libgcc.a.  */
		  && (ELF32_ST_TYPE(sym->st_info) != STT_OBJECT))
		{
		  warn(_("skipping unexpected symbol type %s in relocation in section .rela%s\n"),
                       get_symbol_type(ELF32_ST_TYPE(sym->st_info)),
                       SECTION_NAME(section));
		  continue;
		}
	    }
	  else
	    {
	      /* In MIPS little-endian objects, r_info is NOT really a
               * 64-bit little-endian value: it has a 32-bit little-endian
               * symbol index followed by four individual byte fields.
               * Reorder INFO accordingly.  */
	      if ((elf_header.e_machine == EM_MIPS)
		  && (elf_header.e_ident[EI_DATA] != ELFDATA2MSB))
		rp->r_info = (((rp->r_info & 0xffffffff) << 32)
			      | ((rp->r_info >> 56) & 0xff)
			      | ((rp->r_info >> 40) & 0xff00)
			      | ((rp->r_info >> 24) & 0xff0000)
			      | ((rp->r_info >> 8) & 0xff000000));

	      sym = (symtab + ELF64_R_SYM(rp->r_info));

	      if ((ELF64_R_SYM(rp->r_info) != 0)
		  && (ELF64_ST_TYPE(sym->st_info) != STT_SECTION)
		  && (ELF64_ST_TYPE(sym->st_info) != STT_OBJECT))
		{
		  warn(_("skipping unexpected symbol type %s in relocation in section .rela.%s\n"),
                       get_symbol_type(ELF64_ST_TYPE(sym->st_info)),
                       SECTION_NAME(section));
		  continue;
		}
	    }

	  byte_put (loc, rp->r_addend, reloc_size);
	}

      free (symtab);
      free (rela);
      break;
    }
  return 1;
}

/* FIXME: There are better and more efficient ways to handle
   these structures. For now though, I just want something that
   is simple to implement.  */
typedef struct abbrev_attr
{
  unsigned long attribute;
  unsigned long form;
  struct abbrev_attr *next;
}
abbrev_attr;

typedef struct abbrev_entry
{
  unsigned long entry;
  unsigned long tag;
  int children;
  struct abbrev_attr *first_attr;
  struct abbrev_attr *last_attr;
  struct abbrev_entry *next;
}
abbrev_entry;

static abbrev_entry *first_abbrev = NULL;
static abbrev_entry *last_abbrev = NULL;

static void
free_abbrevs (void)
{
  abbrev_entry *abbrev;

  for (abbrev = first_abbrev; abbrev;)
    {
      abbrev_entry *next_entry = abbrev->next;
      abbrev_attr *attr;

      for (attr = abbrev->first_attr; attr;)
	{
	  abbrev_attr *next_attr = attr->next;

	  free(attr);
	  attr = next_attr;
	}

      free(abbrev);
      abbrev = next_entry;
    }

  last_abbrev = first_abbrev = NULL;
}

static void
add_abbrev(unsigned long number, unsigned long tag, int children)
{
  abbrev_entry *entry;

  entry = (abbrev_entry *)malloc(sizeof(*entry));

  if (entry == NULL) {
    /* ugg: */
    return;
  }

  entry->entry = number;
  entry->tag = tag;
  entry->children = children;
  entry->first_attr = NULL;
  entry->last_attr = NULL;
  entry->next = NULL;

  if (first_abbrev == NULL) {
    first_abbrev = entry;
  } else {
    last_abbrev->next = entry;
  }

  last_abbrev = entry;
}

static void
add_abbrev_attr(unsigned long attribute, unsigned long form)
{
  abbrev_attr *attr;

  attr = (abbrev_attr *)malloc(sizeof(*attr));

  if (attr == NULL) {
    /* ugg: */
    return;
  }

  attr->attribute = attribute;
  attr->form = form;
  attr->next = NULL;

  if (last_abbrev->first_attr == NULL) {
    last_abbrev->first_attr = attr;
  } else {
    last_abbrev->last_attr->next = attr;
  }

  last_abbrev->last_attr = attr;
}

/* Processes the (partial) contents of a .debug_abbrev section.
   Returns NULL if the end of the section was encountered.
   Returns the address after the last byte read if the end of
   an abbreviation set was found.  */

static unsigned char *
process_abbrev_section (unsigned char *start, unsigned char *end)
{
  if (first_abbrev != NULL)
    return NULL;

  while (start < end)
    {
      unsigned int bytes_read;
      unsigned long entry;
      unsigned long tag;
      unsigned long attribute;
      int children;

      entry = read_leb128 (start, & bytes_read, 0);
      start += bytes_read;

      /* A single zero is supposed to end the section according
       * to the standard. If there is more, then signal that to
       * the caller.  */
      if (entry == 0)
	return start == end ? NULL : start;

      tag = read_leb128(start, &bytes_read, 0);
      start += bytes_read;

      children = *start++;

      add_abbrev(entry, tag, children);

      do {
        unsigned long form;

        attribute = read_leb128(start, &bytes_read, 0);
        start += bytes_read;

        form = read_leb128(start, &bytes_read, 0);
        start += bytes_read;

        if (attribute != 0)
          add_abbrev_attr(attribute, form);
      } while (attribute != 0);
    }

  return NULL;
}

static const char *
get_TAG_name(unsigned long tag)
{
  switch (tag)
    {
    case DW_TAG_padding:		return "DW_TAG_padding";
    case DW_TAG_array_type:		return "DW_TAG_array_type";
    case DW_TAG_class_type:		return "DW_TAG_class_type";
    case DW_TAG_entry_point:		return "DW_TAG_entry_point";
    case DW_TAG_enumeration_type:	return "DW_TAG_enumeration_type";
    case DW_TAG_formal_parameter:	return "DW_TAG_formal_parameter";
    case DW_TAG_imported_declaration:	return "DW_TAG_imported_declaration";
    case DW_TAG_label:			return "DW_TAG_label";
    case DW_TAG_lexical_block:		return "DW_TAG_lexical_block";
    case DW_TAG_member:			return "DW_TAG_member";
    case DW_TAG_pointer_type:		return "DW_TAG_pointer_type";
    case DW_TAG_reference_type:		return "DW_TAG_reference_type";
    case DW_TAG_compile_unit:		return "DW_TAG_compile_unit";
    case DW_TAG_string_type:		return "DW_TAG_string_type";
    case DW_TAG_structure_type:		return "DW_TAG_structure_type";
    case DW_TAG_subroutine_type:	return "DW_TAG_subroutine_type";
    case DW_TAG_typedef:		return "DW_TAG_typedef";
    case DW_TAG_union_type:		return "DW_TAG_union_type";
    case DW_TAG_unspecified_parameters: return "DW_TAG_unspecified_parameters";
    case DW_TAG_variant:		return "DW_TAG_variant";
    case DW_TAG_common_block:		return "DW_TAG_common_block";
    case DW_TAG_common_inclusion:	return "DW_TAG_common_inclusion";
    case DW_TAG_inheritance:		return "DW_TAG_inheritance";
    case DW_TAG_inlined_subroutine:	return "DW_TAG_inlined_subroutine";
    case DW_TAG_module:			return "DW_TAG_module";
    case DW_TAG_ptr_to_member_type:	return "DW_TAG_ptr_to_member_type";
    case DW_TAG_set_type:		return "DW_TAG_set_type";
    case DW_TAG_subrange_type:		return "DW_TAG_subrange_type";
    case DW_TAG_with_stmt:		return "DW_TAG_with_stmt";
    case DW_TAG_access_declaration:	return "DW_TAG_access_declaration";
    case DW_TAG_base_type:		return "DW_TAG_base_type";
    case DW_TAG_catch_block:		return "DW_TAG_catch_block";
    case DW_TAG_const_type:		return "DW_TAG_const_type";
    case DW_TAG_constant:		return "DW_TAG_constant";
    case DW_TAG_enumerator:		return "DW_TAG_enumerator";
    case DW_TAG_file_type:		return "DW_TAG_file_type";
    case DW_TAG_friend:			return "DW_TAG_friend";
    case DW_TAG_namelist:		return "DW_TAG_namelist";
    case DW_TAG_namelist_item:		return "DW_TAG_namelist_item";
    case DW_TAG_packed_type:		return "DW_TAG_packed_type";
    case DW_TAG_subprogram:		return "DW_TAG_subprogram";
    case DW_TAG_template_type_param:	return "DW_TAG_template_type_param";
    case DW_TAG_template_value_param:	return "DW_TAG_template_value_param";
    case DW_TAG_thrown_type:		return "DW_TAG_thrown_type";
    case DW_TAG_try_block:		return "DW_TAG_try_block";
    case DW_TAG_variant_part:		return "DW_TAG_variant_part";
    case DW_TAG_variable:		return "DW_TAG_variable";
    case DW_TAG_volatile_type:		return "DW_TAG_volatile_type";
    case DW_TAG_MIPS_loop:		return "DW_TAG_MIPS_loop";
    case DW_TAG_format_label:		return "DW_TAG_format_label";
    case DW_TAG_function_template:	return "DW_TAG_function_template";
    case DW_TAG_class_template:		return "DW_TAG_class_template";
      /* DWARF 2.1 values.  */
    case DW_TAG_dwarf_procedure:	return "DW_TAG_dwarf_procedure";
    case DW_TAG_restrict_type:		return "DW_TAG_restrict_type";
    case DW_TAG_interface_type:		return "DW_TAG_interface_type";
    case DW_TAG_namespace:		return "DW_TAG_namespace";
    case DW_TAG_imported_module:	return "DW_TAG_imported_module";
    case DW_TAG_unspecified_type:	return "DW_TAG_unspecified_type";
    case DW_TAG_partial_unit:		return "DW_TAG_partial_unit";
    case DW_TAG_imported_unit:		return "DW_TAG_imported_unit";
      /* UPC values.  */
    case DW_TAG_upc_shared_type:	return "DW_TAG_upc_shared_type";
    case DW_TAG_upc_strict_type:	return "DW_TAG_upc_strict_type";
    case DW_TAG_upc_relaxed_type:	return "DW_TAG_upc_relaxed_type";
    default:
      {
	static char buffer[100];

	snprintf(buffer, sizeof(buffer), _("Unknown TAG value: %lx"), tag);
	return buffer;
      }
    }
}

static const char *
get_FORM_name(unsigned long form)
{
  switch (form)
    {
    case DW_FORM_addr:		return "DW_FORM_addr";
    case DW_FORM_block2:	return "DW_FORM_block2";
    case DW_FORM_block4:	return "DW_FORM_block4";
    case DW_FORM_data2:		return "DW_FORM_data2";
    case DW_FORM_data4:		return "DW_FORM_data4";
    case DW_FORM_data8:		return "DW_FORM_data8";
    case DW_FORM_string:	return "DW_FORM_string";
    case DW_FORM_block:		return "DW_FORM_block";
    case DW_FORM_block1:	return "DW_FORM_block1";
    case DW_FORM_data1:		return "DW_FORM_data1";
    case DW_FORM_flag:		return "DW_FORM_flag";
    case DW_FORM_sdata:		return "DW_FORM_sdata";
    case DW_FORM_strp:		return "DW_FORM_strp";
    case DW_FORM_udata:		return "DW_FORM_udata";
    case DW_FORM_ref_addr:	return "DW_FORM_ref_addr";
    case DW_FORM_ref1:		return "DW_FORM_ref1";
    case DW_FORM_ref2:		return "DW_FORM_ref2";
    case DW_FORM_ref4:		return "DW_FORM_ref4";
    case DW_FORM_ref8:		return "DW_FORM_ref8";
    case DW_FORM_ref_udata:	return "DW_FORM_ref_udata";
    case DW_FORM_indirect:	return "DW_FORM_indirect";
    default:
      {
	static char buffer[100];

	snprintf(buffer, sizeof(buffer), _("Unknown FORM value: %lx"), form);
	return buffer;
      }
    }
}

static unsigned char *
display_block(unsigned char *data, unsigned long length)
{
  printf(_(" %lu byte block: "), length);

  while (length --)
    printf ("%lx ", (unsigned long) byte_get (data++, 1));

  return data;
}

static int
decode_location_expression(unsigned char *data, unsigned int pointer_size,
			   unsigned long length, unsigned long cu_offset)
{
  unsigned op;
  unsigned int bytes_read;
  unsigned long uvalue;
  unsigned char *end = data + length;
  int need_frame_base = 0;

  while (data < end)
    {
      op = *data++;

      switch (op)
	{
	case DW_OP_addr:
	  printf ("DW_OP_addr: %lx",
		  (unsigned long) byte_get (data, pointer_size));
	  data += pointer_size;
	  break;
	case DW_OP_deref:
	  printf ("DW_OP_deref");
	  break;
	case DW_OP_const1u:
	  printf ("DW_OP_const1u: %lu", (unsigned long) byte_get (data++, 1));
	  break;
	case DW_OP_const1s:
	  printf ("DW_OP_const1s: %ld", (long) byte_get_signed (data++, 1));
	  break;
	case DW_OP_const2u:
	  printf ("DW_OP_const2u: %lu", (unsigned long) byte_get (data, 2));
	  data += 2;
	  break;
	case DW_OP_const2s:
	  printf ("DW_OP_const2s: %ld", (long) byte_get_signed (data, 2));
	  data += 2;
	  break;
	case DW_OP_const4u:
	  printf ("DW_OP_const4u: %lu", (unsigned long) byte_get (data, 4));
	  data += 4;
	  break;
	case DW_OP_const4s:
	  printf ("DW_OP_const4s: %ld", (long) byte_get_signed (data, 4));
	  data += 4;
	  break;
	case DW_OP_const8u:
	  printf ("DW_OP_const8u: %lu %lu", (unsigned long) byte_get (data, 4),
		  (unsigned long) byte_get (data + 4, 4));
	  data += 8;
	  break;
	case DW_OP_const8s:
	  printf ("DW_OP_const8s: %ld %ld", (long) byte_get (data, 4),
		  (long) byte_get (data + 4, 4));
	  data += 8;
	  break;
	case DW_OP_constu:
	  printf ("DW_OP_constu: %lu", read_leb128 (data, &bytes_read, 0));
	  data += bytes_read;
	  break;
	case DW_OP_consts:
	  printf ("DW_OP_consts: %ld", read_leb128 (data, &bytes_read, 1));
	  data += bytes_read;
	  break;
	case DW_OP_dup:
	  printf ("DW_OP_dup");
	  break;
	case DW_OP_drop:
	  printf ("DW_OP_drop");
	  break;
	case DW_OP_over:
	  printf ("DW_OP_over");
	  break;
	case DW_OP_pick:
	  printf ("DW_OP_pick: %ld", (unsigned long) byte_get (data++, 1));
	  break;
	case DW_OP_swap:
	  printf ("DW_OP_swap");
	  break;
	case DW_OP_rot:
	  printf ("DW_OP_rot");
	  break;
	case DW_OP_xderef:
	  printf ("DW_OP_xderef");
	  break;
	case DW_OP_abs:
	  printf ("DW_OP_abs");
	  break;
	case DW_OP_and:
	  printf ("DW_OP_and");
	  break;
	case DW_OP_div:
	  printf ("DW_OP_div");
	  break;
	case DW_OP_minus:
	  printf ("DW_OP_minus");
	  break;
	case DW_OP_mod:
	  printf ("DW_OP_mod");
	  break;
	case DW_OP_mul:
	  printf ("DW_OP_mul");
	  break;
	case DW_OP_neg:
	  printf ("DW_OP_neg");
	  break;
	case DW_OP_not:
	  printf ("DW_OP_not");
	  break;
	case DW_OP_or:
	  printf ("DW_OP_or");
	  break;
	case DW_OP_plus:
	  printf ("DW_OP_plus");
	  break;
	case DW_OP_plus_uconst:
	  printf ("DW_OP_plus_uconst: %lu",
		  read_leb128 (data, &bytes_read, 0));
	  data += bytes_read;
	  break;
	case DW_OP_shl:
	  printf ("DW_OP_shl");
	  break;
	case DW_OP_shr:
	  printf ("DW_OP_shr");
	  break;
	case DW_OP_shra:
	  printf ("DW_OP_shra");
	  break;
	case DW_OP_xor:
	  printf ("DW_OP_xor");
	  break;
	case DW_OP_bra:
	  printf ("DW_OP_bra: %ld", (long) byte_get_signed (data, 2));
	  data += 2;
	  break;
	case DW_OP_eq:
	  printf ("DW_OP_eq");
	  break;
	case DW_OP_ge:
	  printf ("DW_OP_ge");
	  break;
	case DW_OP_gt:
	  printf ("DW_OP_gt");
	  break;
	case DW_OP_le:
	  printf ("DW_OP_le");
	  break;
	case DW_OP_lt:
	  printf ("DW_OP_lt");
	  break;
	case DW_OP_ne:
	  printf ("DW_OP_ne");
	  break;
	case DW_OP_skip:
	  printf ("DW_OP_skip: %ld", (long) byte_get_signed (data, 2));
	  data += 2;
	  break;

	case DW_OP_lit0:
	case DW_OP_lit1:
	case DW_OP_lit2:
	case DW_OP_lit3:
	case DW_OP_lit4:
	case DW_OP_lit5:
	case DW_OP_lit6:
	case DW_OP_lit7:
	case DW_OP_lit8:
	case DW_OP_lit9:
	case DW_OP_lit10:
	case DW_OP_lit11:
	case DW_OP_lit12:
	case DW_OP_lit13:
	case DW_OP_lit14:
	case DW_OP_lit15:
	case DW_OP_lit16:
	case DW_OP_lit17:
	case DW_OP_lit18:
	case DW_OP_lit19:
	case DW_OP_lit20:
	case DW_OP_lit21:
	case DW_OP_lit22:
	case DW_OP_lit23:
	case DW_OP_lit24:
	case DW_OP_lit25:
	case DW_OP_lit26:
	case DW_OP_lit27:
	case DW_OP_lit28:
	case DW_OP_lit29:
	case DW_OP_lit30:
	case DW_OP_lit31:
	  printf ("DW_OP_lit%d", op - DW_OP_lit0);
	  break;

	case DW_OP_reg0:
	case DW_OP_reg1:
	case DW_OP_reg2:
	case DW_OP_reg3:
	case DW_OP_reg4:
	case DW_OP_reg5:
	case DW_OP_reg6:
	case DW_OP_reg7:
	case DW_OP_reg8:
	case DW_OP_reg9:
	case DW_OP_reg10:
	case DW_OP_reg11:
	case DW_OP_reg12:
	case DW_OP_reg13:
	case DW_OP_reg14:
	case DW_OP_reg15:
	case DW_OP_reg16:
	case DW_OP_reg17:
	case DW_OP_reg18:
	case DW_OP_reg19:
	case DW_OP_reg20:
	case DW_OP_reg21:
	case DW_OP_reg22:
	case DW_OP_reg23:
	case DW_OP_reg24:
	case DW_OP_reg25:
	case DW_OP_reg26:
	case DW_OP_reg27:
	case DW_OP_reg28:
	case DW_OP_reg29:
	case DW_OP_reg30:
	case DW_OP_reg31:
	  printf ("DW_OP_reg%d", op - DW_OP_reg0);
	  break;

	case DW_OP_breg0:
	case DW_OP_breg1:
	case DW_OP_breg2:
	case DW_OP_breg3:
	case DW_OP_breg4:
	case DW_OP_breg5:
	case DW_OP_breg6:
	case DW_OP_breg7:
	case DW_OP_breg8:
	case DW_OP_breg9:
	case DW_OP_breg10:
	case DW_OP_breg11:
	case DW_OP_breg12:
	case DW_OP_breg13:
	case DW_OP_breg14:
	case DW_OP_breg15:
	case DW_OP_breg16:
	case DW_OP_breg17:
	case DW_OP_breg18:
	case DW_OP_breg19:
	case DW_OP_breg20:
	case DW_OP_breg21:
	case DW_OP_breg22:
	case DW_OP_breg23:
	case DW_OP_breg24:
	case DW_OP_breg25:
	case DW_OP_breg26:
	case DW_OP_breg27:
	case DW_OP_breg28:
	case DW_OP_breg29:
	case DW_OP_breg30:
	case DW_OP_breg31:
	  printf ("DW_OP_breg%d: %ld", op - DW_OP_breg0,
		  read_leb128 (data, &bytes_read, 1));
	  data += bytes_read;
	  break;

	case DW_OP_regx:
	  printf ("DW_OP_regx: %lu", read_leb128 (data, &bytes_read, 0));
	  data += bytes_read;
	  break;
	case DW_OP_fbreg:
	  need_frame_base = 1;
	  printf ("DW_OP_fbreg: %ld", read_leb128 (data, &bytes_read, 1));
	  data += bytes_read;
	  break;
	case DW_OP_bregx:
	  uvalue = read_leb128 (data, &bytes_read, 0);
	  data += bytes_read;
	  printf ("DW_OP_bregx: %lu %ld", uvalue,
		  read_leb128 (data, &bytes_read, 1));
	  data += bytes_read;
	  break;
	case DW_OP_piece:
	  printf ("DW_OP_piece: %lu", read_leb128 (data, &bytes_read, 0));
	  data += bytes_read;
	  break;
	case DW_OP_deref_size:
	  printf ("DW_OP_deref_size: %ld", (long) byte_get (data++, 1));
	  break;
	case DW_OP_xderef_size:
	  printf ("DW_OP_xderef_size: %ld", (long) byte_get (data++, 1));
	  break;
	case DW_OP_nop:
	  printf ("DW_OP_nop");
	  break;

	  /* DWARF 3 extensions.  */
	case DW_OP_push_object_address:
	  printf ("DW_OP_push_object_address");
	  break;
	case DW_OP_call2:
	  /* XXX: Strictly speaking for 64-bit DWARF3 files
	     this ought to be an 8-byte wide computation.  */
	  printf ("DW_OP_call2: <%lx>", (long) byte_get (data, 2) + cu_offset);
	  data += 2;
	  break;
	case DW_OP_call4:
	  /* XXX: Strictly speaking for 64-bit DWARF3 files
	     this ought to be an 8-byte wide computation.  */
	  printf ("DW_OP_call4: <%lx>", (long) byte_get (data, 4) + cu_offset);
	  data += 4;
	  break;
	case DW_OP_call_ref:
	  printf ("DW_OP_call_ref");
	  break;

	  /* GNU extensions.  */
	case DW_OP_GNU_push_tls_address:
	  printf ("DW_OP_GNU_push_tls_address");
	  break;

	default:
	  if (op >= DW_OP_lo_user
	      && op <= DW_OP_hi_user)
	    printf (_("(User defined location op)"));
	  else
	    printf (_("(Unknown location op)"));
	  /* No way to tell where the next op is, so just bail.  */
	  return need_frame_base;
	}

      /* Separate the ops.  */
      if (data < end)
	printf ("; ");
    }

  return need_frame_base;
}

/* This structure records the information that
   we extract from the.debug_info section.  */
typedef struct
{
  unsigned int   pointer_size;
  unsigned long  cu_offset;
  unsigned long	 base_address;
  /* This is an array of offsets to the location list table.  */
  unsigned long *loc_offsets;
  int		*have_frame_base;
  unsigned int   num_loc_offsets;
  unsigned int   max_loc_offsets;
  unsigned long *range_lists;
  unsigned int   num_range_lists;
  unsigned int   max_range_lists;
}
debug_info;

static debug_info *   debug_information = NULL;
static unsigned int   num_debug_info_entries = 0;
static unsigned int   last_pointer_size = 0;
static int            warned_about_missing_comp_units = FALSE;

static unsigned char *
read_and_display_attr_value(unsigned long attribute, unsigned long form,
			    unsigned char *data, unsigned long cu_offset,
			    unsigned long pointer_size,
                            unsigned long offset_size, int dwarf_version,
			    debug_info *debug_info_p, int do_loc)
{
  unsigned long uvalue = 0;
  unsigned char *block_start = NULL;
  unsigned int bytes_read;

  switch (form)
    {
    default:
      break;

    case DW_FORM_ref_addr:
      if (dwarf_version == 2)
	{
	  uvalue = byte_get (data, pointer_size);
	  data += pointer_size;
	}
      else if (dwarf_version == 3)
	{
	  uvalue = byte_get (data, offset_size);
	  data += offset_size;
	}
      else
	{
	  error (_("Internal error: DWARF version is not 2 or 3.\n"));
	}
      break;

    case DW_FORM_addr:
      uvalue = byte_get (data, pointer_size);
      data += pointer_size;
      break;

    case DW_FORM_strp:
      uvalue = byte_get (data, offset_size);
      data += offset_size;
      break;

    case DW_FORM_ref1:
    case DW_FORM_flag:
    case DW_FORM_data1:
      uvalue = byte_get (data++, 1);
      break;

    case DW_FORM_ref2:
    case DW_FORM_data2:
      uvalue = byte_get (data, 2);
      data += 2;
      break;

    case DW_FORM_ref4:
    case DW_FORM_data4:
      uvalue = byte_get (data, 4);
      data += 4;
      break;

    case DW_FORM_sdata:
      uvalue = read_leb128 (data, & bytes_read, 1);
      data += bytes_read;
      break;

    case DW_FORM_ref_udata:
    case DW_FORM_udata:
      uvalue = read_leb128 (data, & bytes_read, 0);
      data += bytes_read;
      break;

    case DW_FORM_indirect:
      form = read_leb128 (data, & bytes_read, 0);
      data += bytes_read;
      if (!do_loc)
	printf (" %s", get_FORM_name (form));
      return read_and_display_attr_value (attribute, form, data,
					  cu_offset, pointer_size,
					  offset_size, dwarf_version,
					  debug_info_p, do_loc);
    }

  switch (form)
    {
    case DW_FORM_ref_addr:
      if (!do_loc)
	printf (" <#%lx>", uvalue);
      break;

    case DW_FORM_ref1:
    case DW_FORM_ref2:
    case DW_FORM_ref4:
    case DW_FORM_ref_udata:
      if (!do_loc)
	printf (" <%lx>", uvalue + cu_offset);
      break;

    case DW_FORM_data4:
    case DW_FORM_addr:
      if (!do_loc)
	printf (" %#lx", uvalue);
      break;

    case DW_FORM_flag:
    case DW_FORM_data1:
    case DW_FORM_data2:
    case DW_FORM_sdata:
    case DW_FORM_udata:
      if (!do_loc)
	printf (" %ld", uvalue);
      break;

    case DW_FORM_ref8:
    case DW_FORM_data8:
      if (!do_loc)
	{
	  uvalue = byte_get (data, 4);
	  printf (" %lx", uvalue);
	  printf (" %lx", (unsigned long) byte_get (data + 4, 4));
	}
      if ((do_loc || do_debug_loc || do_debug_ranges)
	  && num_debug_info_entries == 0)
	{
	  if (sizeof (uvalue) == 8)
	    uvalue = byte_get (data, 8);
	  else
	    error (_("DW_FORM_data8 is unsupported when sizeof (unsigned long) != 8\n"));
	}
      data += 8;
      break;

    case DW_FORM_string:
      if (!do_loc)
	printf (" %s", data);
      data += strlen ((char *) data) + 1;
      break;

    case DW_FORM_block:
      uvalue = read_leb128 (data, & bytes_read, 0);
      block_start = data + bytes_read;
      if (do_loc)
	data = block_start + uvalue;
      else
	data = display_block (block_start, uvalue);
      break;

    case DW_FORM_block1:
      uvalue = byte_get (data, 1);
      block_start = data + 1;
      if (do_loc)
	data = block_start + uvalue;
      else
	data = display_block (block_start, uvalue);
      break;

    case DW_FORM_block2:
      uvalue = byte_get (data, 2);
      block_start = data + 2;
      if (do_loc)
	data = block_start + uvalue;
      else
	data = display_block (block_start, uvalue);
      break;

    case DW_FORM_block4:
      uvalue = byte_get (data, 4);
      block_start = data + 4;
      if (do_loc)
	data = block_start + uvalue;
      else
	data = display_block (block_start, uvalue);
      break;

    case DW_FORM_strp:
      if (!do_loc)
	printf (_(" (indirect string, offset: 0x%lx): %s"),
		uvalue, fetch_indirect_string (uvalue));
      break;

    case DW_FORM_indirect:
      /* Handled above.  */
      break;

    default:
      warn (_("Unrecognized form: %lu\n"), form);
      break;
    }

  /* For some attributes we can display further information.  */
  if ((do_loc || do_debug_loc || do_debug_ranges)
      && num_debug_info_entries == 0)
    {
      switch (attribute)
	{
	case DW_AT_frame_base:
	  have_frame_base = 1; break; /* -Wimplicit-fallthrough */
	case DW_AT_location:
	case DW_AT_data_member_location:
	case DW_AT_vtable_elem_location:
	case DW_AT_allocated:
	case DW_AT_associated:
	case DW_AT_data_location:
	case DW_AT_stride:
	case DW_AT_upper_bound:
	case DW_AT_lower_bound:
	  if (form == DW_FORM_data4 || form == DW_FORM_data8)
	    {
	      /* Process location list.  */
	      unsigned int max = debug_info_p->max_loc_offsets;
	      unsigned int num = debug_info_p->num_loc_offsets;

	      if (max == 0 || num >= max)
		{
		  max += 1024;
		  debug_info_p->loc_offsets
		    = (unsigned long *)xcrealloc(debug_info_p->loc_offsets,
                                                 max, sizeof(*debug_info_p->loc_offsets));
		  debug_info_p->have_frame_base
		    = (int *)xcrealloc(debug_info_p->have_frame_base,
                                       max, sizeof(*debug_info_p->have_frame_base));
		  debug_info_p->max_loc_offsets = max;
		}
	      debug_info_p->loc_offsets [num] = uvalue;
	      debug_info_p->have_frame_base [num] = have_frame_base;
	      debug_info_p->num_loc_offsets++;
	    }
	  break;

	case DW_AT_low_pc:
	  if (need_base_address)
	    debug_info_p->base_address = uvalue;
	  break;

	case DW_AT_ranges:
	  if (form == DW_FORM_data4 || form == DW_FORM_data8)
	    {
	      /* Process range list: */
	      unsigned int max = debug_info_p->max_range_lists;
	      unsigned int num = debug_info_p->num_range_lists;

	      if ((max == 0) || (num >= max))
		{
		  max += 1024;
		  debug_info_p->range_lists
		    = (unsigned long *)xcrealloc(debug_info_p->range_lists,
                                                 max, sizeof(*debug_info_p->range_lists));
		  debug_info_p->max_range_lists = max;
		}
	      debug_info_p->range_lists [num] = uvalue;
	      debug_info_p->num_range_lists++;
	    }
	  break;

	default:
	  break;
	}
    }

  if (do_loc)
    return data;

  printf ("\t");

  switch (attribute)
    {
    case DW_AT_inline:
      switch (uvalue)
	{
	case DW_INL_not_inlined:
	  printf (_("(not inlined)"));
	  break;
	case DW_INL_inlined:
	  printf (_("(inlined)"));
	  break;
	case DW_INL_declared_not_inlined:
	  printf (_("(declared as inline but ignored)"));
	  break;
	case DW_INL_declared_inlined:
	  printf (_("(declared as inline and inlined)"));
	  break;
	default:
	  printf (_("  (Unknown inline attribute value: %lx)"), uvalue);
	  break;
	}
      break;

    case DW_AT_language:
      switch (uvalue)
	{
	case DW_LANG_C:			printf ("(non-ANSI C)"); break;
	case DW_LANG_C89:		printf ("(ANSI C)"); break;
	case DW_LANG_C_plus_plus:	printf ("(C++)"); break;
	case DW_LANG_Fortran77:		printf ("(FORTRAN 77)"); break;
	case DW_LANG_Fortran90:		printf ("(Fortran 90)"); break;
	case DW_LANG_Modula2:		printf ("(Modula 2)"); break;
	case DW_LANG_Pascal83:		printf ("(ANSI Pascal)"); break;
	case DW_LANG_Ada83:		printf ("(Ada)"); break;
	case DW_LANG_Cobol74:		printf ("(Cobol 74)"); break;
	case DW_LANG_Cobol85:		printf ("(Cobol 85)"); break;
	  /* DWARF 2.1 values.	*/
	case DW_LANG_C99:		printf ("(ANSI C99)"); break;
	case DW_LANG_Ada95:		printf ("(ADA 95)"); break;
	case DW_LANG_Fortran95:		printf ("(Fortran 95)"); break;
	  /* MIPS extension.  */
	case DW_LANG_Mips_Assembler:	printf ("(MIPS assembler)"); break;
	  /* UPC extension.  */
	case DW_LANG_Upc:		printf ("(Unified Parallel C)"); break;
	default:
	  printf ("(Unknown: %lx)", uvalue);
	  break;
	}
      break;

    case DW_AT_encoding:
      switch (uvalue)
	{
	case DW_ATE_void:		printf ("(void)"); break;
	case DW_ATE_address:		printf ("(machine address)"); break;
	case DW_ATE_boolean:		printf ("(boolean)"); break;
	case DW_ATE_complex_float:	printf ("(complex float)"); break;
	case DW_ATE_float:		printf ("(float)"); break;
	case DW_ATE_signed:		printf ("(signed)"); break;
	case DW_ATE_signed_char:	printf ("(signed char)"); break;
	case DW_ATE_unsigned:		printf ("(unsigned)"); break;
	case DW_ATE_unsigned_char:	printf ("(unsigned char)"); break;
	  /* DWARF 2.1 value.  */
	case DW_ATE_imaginary_float:	printf ("(imaginary float)"); break;
	case DW_ATE_decimal_float:	printf ("(decimal float)"); break;
	default:
	  if (uvalue >= DW_ATE_lo_user
	      && uvalue <= DW_ATE_hi_user)
	    printf ("(user defined type)");
	  else
	    printf ("(unknown type)");
	  break;
	}
      break;

    case DW_AT_accessibility:
      switch (uvalue)
	{
	case DW_ACCESS_public:		printf ("(public)"); break;
	case DW_ACCESS_protected:	printf ("(protected)"); break;
	case DW_ACCESS_private:		printf ("(private)"); break;
	default:
	  printf ("(unknown accessibility)");
	  break;
	}
      break;

    case DW_AT_visibility:
      switch (uvalue)
	{
	case DW_VIS_local:		printf ("(local)"); break;
	case DW_VIS_exported:		printf ("(exported)"); break;
	case DW_VIS_qualified:		printf ("(qualified)"); break;
	default:			printf ("(unknown visibility)"); break;
	}
      break;

    case DW_AT_virtuality:
      switch (uvalue)
	{
	case DW_VIRTUALITY_none:	printf ("(none)"); break;
	case DW_VIRTUALITY_virtual:	printf ("(virtual)"); break;
	case DW_VIRTUALITY_pure_virtual:printf ("(pure_virtual)"); break;
	default:			printf ("(unknown virtuality)"); break;
	}
      break;

    case DW_AT_identifier_case:
      switch (uvalue)
	{
	case DW_ID_case_sensitive:	printf ("(case_sensitive)"); break;
	case DW_ID_up_case:		printf ("(up_case)"); break;
	case DW_ID_down_case:		printf ("(down_case)"); break;
	case DW_ID_case_insensitive:	printf ("(case_insensitive)"); break;
	default:			printf ("(unknown case)"); break;
	}
      break;

    case DW_AT_calling_convention:
      switch (uvalue)
	{
	case DW_CC_normal:	printf("(normal)"); break;
	case DW_CC_program:	printf("(program)"); break;
	case DW_CC_nocall:	printf("(nocall)"); break;
	default:
	  if ((uvalue >= DW_CC_lo_user)
	      && (uvalue <= DW_CC_hi_user))
	    printf("(user defined)");
	  else
	    printf("(unknown convention)");
	}
      break;

    case DW_AT_ordering:
      switch (uvalue)
	{
	case -1: printf("(undefined)"); break;
	case 0: printf("(row major)"); break;
	case 1: printf("(column major)"); break;
        default: break;
	}
      break;

    case DW_AT_frame_base:
      have_frame_base = 1; break; /* -Wimplicit-fallthrough */
    case DW_AT_location:
    case DW_AT_data_member_location:
    case DW_AT_vtable_elem_location:
    case DW_AT_allocated:
    case DW_AT_associated:
    case DW_AT_data_location:
    case DW_AT_stride:
    case DW_AT_upper_bound:
    case DW_AT_lower_bound:
      if (block_start)
	{
	  int need_frame_base;

	  printf ("(");
	  need_frame_base = decode_location_expression (block_start,
							pointer_size,
							uvalue,
							cu_offset);
	  printf (")");
	  if (need_frame_base && !have_frame_base)
	    printf (_(" [without DW_AT_frame_base]"));
	}
      else if (form == DW_FORM_data4 || form == DW_FORM_data8)
	printf (_("(location list)"));

      break;

    default:
      break;
    }

  return data;
}

static const char *
get_AT_name(unsigned long attribute)
{
  switch (attribute)
    {
    case DW_AT_sibling:			return "DW_AT_sibling";
    case DW_AT_location:		return "DW_AT_location";
    case DW_AT_name:			return "DW_AT_name";
    case DW_AT_ordering:		return "DW_AT_ordering";
    case DW_AT_subscr_data:		return "DW_AT_subscr_data";
    case DW_AT_byte_size:		return "DW_AT_byte_size";
    case DW_AT_bit_offset:		return "DW_AT_bit_offset";
    case DW_AT_bit_size:		return "DW_AT_bit_size";
    case DW_AT_element_list:		return "DW_AT_element_list";
    case DW_AT_stmt_list:		return "DW_AT_stmt_list";
    case DW_AT_low_pc:			return "DW_AT_low_pc";
    case DW_AT_high_pc:			return "DW_AT_high_pc";
    case DW_AT_language:		return "DW_AT_language";
    case DW_AT_member:			return "DW_AT_member";
    case DW_AT_discr:			return "DW_AT_discr";
    case DW_AT_discr_value:		return "DW_AT_discr_value";
    case DW_AT_visibility:		return "DW_AT_visibility";
    case DW_AT_import:			return "DW_AT_import";
    case DW_AT_string_length:		return "DW_AT_string_length";
    case DW_AT_common_reference:	return "DW_AT_common_reference";
    case DW_AT_comp_dir:		return "DW_AT_comp_dir";
    case DW_AT_const_value:		return "DW_AT_const_value";
    case DW_AT_containing_type:		return "DW_AT_containing_type";
    case DW_AT_default_value:		return "DW_AT_default_value";
    case DW_AT_inline:			return "DW_AT_inline";
    case DW_AT_is_optional:		return "DW_AT_is_optional";
    case DW_AT_lower_bound:		return "DW_AT_lower_bound";
    case DW_AT_producer:		return "DW_AT_producer";
    case DW_AT_prototyped:		return "DW_AT_prototyped";
    case DW_AT_return_addr:		return "DW_AT_return_addr";
    case DW_AT_start_scope:		return "DW_AT_start_scope";
    case DW_AT_stride_size:		return "DW_AT_stride_size";
    case DW_AT_upper_bound:		return "DW_AT_upper_bound";
    case DW_AT_abstract_origin:		return "DW_AT_abstract_origin";
    case DW_AT_accessibility:		return "DW_AT_accessibility";
    case DW_AT_address_class:		return "DW_AT_address_class";
    case DW_AT_artificial:		return "DW_AT_artificial";
    case DW_AT_base_types:		return "DW_AT_base_types";
    case DW_AT_calling_convention:	return "DW_AT_calling_convention";
    case DW_AT_count:			return "DW_AT_count";
    case DW_AT_data_member_location:	return "DW_AT_data_member_location";
    case DW_AT_decl_column:		return "DW_AT_decl_column";
    case DW_AT_decl_file:		return "DW_AT_decl_file";
    case DW_AT_decl_line:		return "DW_AT_decl_line";
    case DW_AT_declaration:		return "DW_AT_declaration";
    case DW_AT_discr_list:		return "DW_AT_discr_list";
    case DW_AT_encoding:		return "DW_AT_encoding";
    case DW_AT_external:		return "DW_AT_external";
    case DW_AT_frame_base:		return "DW_AT_frame_base";
    case DW_AT_friend:			return "DW_AT_friend";
    case DW_AT_identifier_case:		return "DW_AT_identifier_case";
    case DW_AT_macro_info:		return "DW_AT_macro_info";
    case DW_AT_namelist_items:		return "DW_AT_namelist_items";
    case DW_AT_priority:		return "DW_AT_priority";
    case DW_AT_segment:			return "DW_AT_segment";
    case DW_AT_specification:		return "DW_AT_specification";
    case DW_AT_static_link:		return "DW_AT_static_link";
    case DW_AT_type:			return "DW_AT_type";
    case DW_AT_use_location:		return "DW_AT_use_location";
    case DW_AT_variable_parameter:	return "DW_AT_variable_parameter";
    case DW_AT_virtuality:		return "DW_AT_virtuality";
    case DW_AT_vtable_elem_location:	return "DW_AT_vtable_elem_location";
      /* DWARF 2.1 values.  */
    case DW_AT_allocated:		return "DW_AT_allocated";
    case DW_AT_associated:		return "DW_AT_associated";
    case DW_AT_data_location:		return "DW_AT_data_location";
    case DW_AT_stride:			return "DW_AT_stride";
    case DW_AT_entry_pc:		return "DW_AT_entry_pc";
    case DW_AT_use_UTF8:		return "DW_AT_use_UTF8";
    case DW_AT_extension:		return "DW_AT_extension";
    case DW_AT_ranges:			return "DW_AT_ranges";
    case DW_AT_trampoline:		return "DW_AT_trampoline";
    case DW_AT_call_column:		return "DW_AT_call_column";
    case DW_AT_call_file:		return "DW_AT_call_file";
    case DW_AT_call_line:		return "DW_AT_call_line";
      /* SGI/MIPS extensions.  */
    case DW_AT_MIPS_fde:		return "DW_AT_MIPS_fde";
    case DW_AT_MIPS_loop_begin:		return "DW_AT_MIPS_loop_begin";
    case DW_AT_MIPS_tail_loop_begin:	return "DW_AT_MIPS_tail_loop_begin";
    case DW_AT_MIPS_epilog_begin:	return "DW_AT_MIPS_epilog_begin";
    case DW_AT_MIPS_loop_unroll_factor: return "DW_AT_MIPS_loop_unroll_factor";
    case DW_AT_MIPS_software_pipeline_depth:
      return "DW_AT_MIPS_software_pipeline_depth";
    case DW_AT_MIPS_linkage_name:	return "DW_AT_MIPS_linkage_name";
    case DW_AT_MIPS_stride:		return "DW_AT_MIPS_stride";
    case DW_AT_MIPS_abstract_name:	return "DW_AT_MIPS_abstract_name";
    case DW_AT_MIPS_clone_origin:	return "DW_AT_MIPS_clone_origin";
    case DW_AT_MIPS_has_inlines:	return "DW_AT_MIPS_has_inlines";
      /* GNU extensions.  */
    case DW_AT_sf_names:		return "DW_AT_sf_names";
    case DW_AT_src_info:		return "DW_AT_src_info";
    case DW_AT_mac_info:		return "DW_AT_mac_info";
    case DW_AT_src_coords:		return "DW_AT_src_coords";
    case DW_AT_body_begin:		return "DW_AT_body_begin";
    case DW_AT_body_end:		return "DW_AT_body_end";
    case DW_AT_GNU_vector:		return "DW_AT_GNU_vector";
      /* UPC extension.  */
    case DW_AT_upc_threads_scaled:	return "DW_AT_upc_threads_scaled";
    default:
      {
	static char buffer[100];

	snprintf(buffer, sizeof(buffer), _("Unknown AT value: %lx"),
		 attribute);
	return buffer;
      }
    }
}

static unsigned char *
read_and_display_attr(unsigned long attribute, unsigned long form,
		      unsigned char *data, unsigned long cu_offset,
		      unsigned long pointer_size,
                      unsigned long offset_size, int dwarf_version,
		      debug_info *debug_info_p, int do_loc)
{
  if (!do_loc)
    printf("     %-18s:", get_AT_name(attribute));
  data = read_and_display_attr_value(attribute, form, data, cu_offset,
				     pointer_size, offset_size,
				     dwarf_version, debug_info_p,
				     do_loc);
  if (!do_loc)
    printf("\n");
  return data;
}


/* Process the contents of a .debug_info section. If do_loc is non-zero,
 * then we are scanning for location lists, and we do not want to display
 * anything to the user: */
static int
process_debug_info(Elf_Internal_Shdr *section, unsigned char *start,
                   FILE *file, int do_loc)
{
  unsigned char *end = (start + section->sh_size);
  unsigned char *section_begin;
  unsigned int unit;
  unsigned int num_units = 0;

  if ((do_loc || do_debug_loc || do_debug_ranges)
      && (num_debug_info_entries == 0))
    {
      unsigned long length;

      /* First scan the section to get the number of comp units: */
      for (section_begin = start, num_units = 0; section_begin < end;
	   num_units++)
	{
	  /* Read the first 4 bytes.  For a 32-bit DWARF section, this will
           * be the length.  For a 64-bit DWARF section, it will be the
           * escape code 0xffffffff followed by an 8 byte length: */
	  length = byte_get(section_begin, 4);

	  if (length == 0xffffffff)
	    {
	      length = byte_get(section_begin + 4, 8);
	      section_begin += (length + 12);
	    }
	  else
	    section_begin += (length + 4);
	}

      if (num_units == 0)
	{
	  error(_("No comp units in .debug_info section ?"));
	  return 0;
	}

      /* Then allocate an array to hold the information: */
      debug_information = (debug_info *)cmalloc(num_units,
                                                sizeof(* debug_information));
      if (debug_information == NULL)
	{
	  error(_("Not enough memory for a debug info array of %u entries"),
                num_units);
	  return 0;
	}
    }

  if (!do_loc)
    {
      printf(_("The section %s contains:\n\n"),
             SECTION_NAME(section));

      load_debug_str(file);
      load_debug_loc(file);
      load_debug_range(file);
    }

  for (section_begin = start, unit = 0; start < end; unit++)
    {
      DWARF2_Internal_CompUnit compunit;
      unsigned char *hdrptr;
      unsigned char *cu_abbrev_offset_ptr;
      unsigned char *tags;
      int level;
      unsigned long cu_offset;
      int offset_size;
      int initial_length_size;

      hdrptr = start;

      compunit.cu_length = byte_get (hdrptr, 4);
      hdrptr += 4;

      if (compunit.cu_length == 0xffffffff)
	{
	  compunit.cu_length = byte_get (hdrptr, 8);
	  hdrptr += 8;
	  offset_size = 8;
	  initial_length_size = 12;
	}
      else
	{
	  offset_size = 4;
	  initial_length_size = 4;
	}

      compunit.cu_version = byte_get (hdrptr, 2);
      hdrptr += 2;

      cu_offset = start - section_begin;
      start += compunit.cu_length + initial_length_size;

      if (elf_header.e_type == ET_REL
	  && !debug_apply_rela_addends (file, section, offset_size,
					section_begin, hdrptr, start))
	return 0;

      cu_abbrev_offset_ptr = hdrptr;
      compunit.cu_abbrev_offset = byte_get(hdrptr, offset_size);
      hdrptr += offset_size;

      if (cu_abbrev_offset_ptr != NULL) {
        ;
      }

      compunit.cu_pointer_size = byte_get (hdrptr, 1);
      hdrptr += 1;
      if ((do_loc || do_debug_loc || do_debug_ranges)
	  && num_debug_info_entries == 0)
	{
	  debug_information [unit].cu_offset = cu_offset;
	  debug_information [unit].pointer_size
	    = compunit.cu_pointer_size;
	  debug_information [unit].base_address = 0;
	  debug_information [unit].loc_offsets = NULL;
	  debug_information [unit].have_frame_base = NULL;
	  debug_information [unit].max_loc_offsets = 0;
	  debug_information [unit].num_loc_offsets = 0;
	  debug_information [unit].range_lists = NULL;
	  debug_information [unit].max_range_lists= 0;
	  debug_information [unit].num_range_lists = 0;
	}

      tags = hdrptr;

      if (!do_loc)
	{
	  printf (_("  Compilation Unit @ offset 0x%lx:\n"), cu_offset);
	  printf (_("   Length:        %ld\n"), compunit.cu_length);
	  printf (_("   Version:       %d\n"), compunit.cu_version);
	  printf (_("   Abbrev Offset: %ld\n"), compunit.cu_abbrev_offset);
	  printf (_("   Pointer Size:  %d\n"), compunit.cu_pointer_size);
	}

      if (compunit.cu_version != 2 && compunit.cu_version != 3)
	{
	  warn (_("Only version 2 and 3 DWARF debug information is currently supported.\n"));
	  continue;
	}

      free_abbrevs ();

      /* Read in the abbrevs used by this compilation unit.  */
      {
	Elf_Internal_Shdr *sec;
	unsigned char *begin;

	/* Locate the .debug_abbrev section and process it: */
	sec = find_section(".debug_abbrev");
	if (sec == NULL)
	  {
	    warn(_("Unable to locate .debug_abbrev section!\n"));
	    return 0;
	  }

	begin = (unsigned char *)get_data(NULL, file, sec->sh_offset, 1,
                                          sec->sh_size,
                                          _("debug_abbrev section data"));
	if (!begin)
	  return 0;

	process_abbrev_section (begin + compunit.cu_abbrev_offset,
				begin + sec->sh_size);

	free (begin);
      }

      level = 0;
      while (tags < start)
	{
	  unsigned int bytes_read;
	  unsigned long abbrev_number;
	  abbrev_entry *entry;
	  abbrev_attr *attr;

	  abbrev_number = read_leb128 (tags, & bytes_read, 0);
	  tags += bytes_read;

	  /* A null DIE marks the end of a list of children.  */
	  if (abbrev_number == 0)
	    {
	      --level;
	      continue;
	    }

	  /* Scan through the abbreviation list until we reach the
	     correct entry.  */
	  for (entry = first_abbrev;
	       entry && entry->entry != abbrev_number;
	       entry = entry->next)
	    continue;

	  if (entry == NULL)
	    {
	      warn (_("Unable to locate entry %lu in the abbreviation table\n"),
		    abbrev_number);
	      return 0;
	    }

	  if (!do_loc)
	    printf (_(" <%d><%lx>: Abbrev Number: %lu (%s)\n"),
		    level,
		    (unsigned long) (tags - section_begin
				     - bytes_read),
		    abbrev_number,
		    get_TAG_name (entry->tag));

	  switch (entry->tag)
	    {
	    default:
	      need_base_address = 0;
	      break;
	    case DW_TAG_compile_unit:
	      need_base_address = 1;
	      break;
	    case DW_TAG_entry_point:
	    case DW_TAG_inlined_subroutine:
	    case DW_TAG_subprogram:
	      need_base_address = 0;
	      /* Assuming that there is no DW_AT_frame_base.  */
	      have_frame_base = 0;
	      break;
	    }

	  for (attr = entry->first_attr; attr; attr = attr->next)
	    tags = read_and_display_attr (attr->attribute,
					  attr->form,
					  tags, cu_offset,
					  compunit.cu_pointer_size,
					  offset_size,
					  compunit.cu_version,
					  &debug_information [unit],
					  do_loc);

 	  if (entry->children)
 	    ++level;
 	}
    }

  /* Set num_debug_info_entries here so that it can be used to check if
     we need to process .debug_loc and .debug_ranges sections.  */
  if ((do_loc || do_debug_loc || do_debug_ranges)
      && num_debug_info_entries == 0)
    num_debug_info_entries = num_units;

  if (!do_loc)
    {
      free_debug_range ();
      free_debug_str ();
      free_debug_loc ();

      printf ("\n");
    }

  return 1;
}

/* Retrieve the pointer size associated with the given compilation unit.
   Optionally the offset of this unit into the .debug_info section is
   also retutned. If there is no .debug_info section then an error
   message is issued and 0 is returned. If the requested comp unit has
   not been defined in the .debug_info section then a warning message
   is issued and the last know pointer size is returned. This message
   is only issued once per section dumped per file dumped.  */

static unsigned int
get_pointer_size_and_offset_of_comp_unit (unsigned int comp_unit,
					  const char * section_name,
					  unsigned long * offset_return)
{
  unsigned long offset = 0;

  if (num_debug_info_entries == 0)
    error (_("%s section needs a populated .debug_info section\n"),
	   section_name);

  else if (comp_unit >= num_debug_info_entries)
    {
      if (!warned_about_missing_comp_units)
	{
	  warn (_("%s section has more comp units than .debug_info section\n"),
		section_name);
	  warn (_("assuming that the pointer size is %d, from the last comp unit in .debug_info\n\n"),
		last_pointer_size);
	  warned_about_missing_comp_units = TRUE;
	}
    }
  else
    {
      last_pointer_size = debug_information [comp_unit].pointer_size;
      offset = debug_information [comp_unit].cu_offset;
    }

  if (offset_return != NULL)
    * offset_return = offset;

  return last_pointer_size;
}

/* Locate and scan the .debug_info section in the file and record the pointer
   sizes and offsets for the compilation units in it. Usually an executable
   will have just one pointer size, but this is not guaranteed, and so we try
   not to make any assumptions. Returns zero upon failure, or the number of
   compilation units upon success.  */

static unsigned int
get_debug_info (FILE * file)
{
  Elf_Internal_Shdr * section;
  unsigned char *start;
  int ret;

  /* Reset the last pointer size so that we can issue correct error
     messages if we are displaying the contents of more than one section.  */
  last_pointer_size = 0;
  warned_about_missing_comp_units = FALSE;

  /* If we already have the information there is nothing else to do.  */
  if (num_debug_info_entries > 0)
    return num_debug_info_entries;

  section = find_section (".debug_info");
  if (section == NULL)
    return 0;

  start = (unsigned char *)get_data(NULL, file, section->sh_offset, 1,
                                    section->sh_size,
                                    _("extracting information from .debug_info section"));
  if (start == NULL)
    return 0;

  ret = process_debug_info (section, start, file, 1);
  free (start);

  return ret ? num_debug_info_entries : 0;
}

static int
display_debug_lines (Elf_Internal_Shdr *section,
		     unsigned char *start, FILE *file)
{
  unsigned char *data = start;
  unsigned char *end = start + section->sh_size;
  unsigned int comp_unit = 0;

  printf (_("\nDump of debug contents of section %s:\n\n"),
	  SECTION_NAME (section));

  get_debug_info (file);

  while (data < end)
    {
      DWARF2_Internal_LineInfo info;
      unsigned char *standard_opcodes;
      unsigned char *end_of_sequence;
      unsigned char *hdrptr;
      unsigned int pointer_size;
      int initial_length_size;
      int offset_size;
      int i;

      hdrptr = data;

      /* Check the length of the block.  */
      info.li_length = byte_get (hdrptr, 4);
      hdrptr += 4;

      if (info.li_length == 0xffffffff)
	{
	  /* This section is 64-bit DWARF 3.  */
	  info.li_length = byte_get (hdrptr, 8);
	  hdrptr += 8;
	  offset_size = 8;
	  initial_length_size = 12;
	}
      else
	{
	  offset_size = 4;
	  initial_length_size = 4;
	}

      if (info.li_length + initial_length_size > section->sh_size)
	{
	  warn
	    (_("The line info appears to be corrupt - the section is too small\n"));
	  return 0;
	}

      /* Check its version number.  */
      info.li_version = byte_get (hdrptr, 2);
      hdrptr += 2;
      if (info.li_version != 2 && info.li_version != 3)
	{
	  warn (_("Only DWARF version 2 and 3 line info is currently supported.\n"));
	  return 0;
	}

      info.li_prologue_length = byte_get (hdrptr, offset_size);
      hdrptr += offset_size;
      info.li_min_insn_length = byte_get (hdrptr, 1);
      hdrptr++;
      info.li_default_is_stmt = byte_get (hdrptr, 1);
      hdrptr++;
      info.li_line_base = byte_get (hdrptr, 1);
      hdrptr++;
      info.li_line_range = byte_get (hdrptr, 1);
      hdrptr++;
      info.li_opcode_base = byte_get (hdrptr, 1);
      hdrptr++;

      /* Sign extend the line base field.  */
      info.li_line_base <<= 24;
      info.li_line_base >>= 24;

      /* Get the pointer size from the comp unit associated
	 with this block of line number information.  */
      pointer_size = get_pointer_size_and_offset_of_comp_unit
	(comp_unit, ".debug_line", NULL);
      comp_unit ++;

      printf (_("  Length:                      %ld\n"), info.li_length);
      printf (_("  DWARF Version:               %d\n"), info.li_version);
      printf (_("  Prologue Length:             %d\n"), info.li_prologue_length);
      printf (_("  Minimum Instruction Length:  %d\n"), info.li_min_insn_length);
      printf (_("  Initial value of 'is_stmt':  %d\n"), info.li_default_is_stmt);
      printf (_("  Line Base:                   %d\n"), info.li_line_base);
      printf (_("  Line Range:                  %d\n"), info.li_line_range);
      printf (_("  Opcode Base:                 %d\n"), info.li_opcode_base);
      printf (_("  (Pointer size:               %u)%s\n"),
	      pointer_size,
	      warned_about_missing_comp_units ? " [assumed]" : "" );

      end_of_sequence = data + info.li_length + initial_length_size;

      reset_state_machine (info.li_default_is_stmt);

      /* Display the contents of the Opcodes table.  */
      standard_opcodes = hdrptr;

      printf (_("\n Opcodes:\n"));

      for (i = 1; i < info.li_opcode_base; i++)
	printf (_("  Opcode %d has %d args\n"), i, standard_opcodes[i - 1]);

      /* Display the contents of the Directory table.  */
      data = standard_opcodes + info.li_opcode_base - 1;

      if (*data == 0)
	printf (_("\n The Directory Table is empty.\n"));
      else
	{
	  printf (_("\n The Directory Table:\n"));

	  while (*data != 0)
	    {
	      printf (_("  %s\n"), data);

	      data += strlen ((char *) data) + 1;
	    }
	}

      /* Skip the NUL at the end of the table.  */
      data++;

      /* Display the contents of the File Name table.  */
      if (*data == 0)
	printf (_("\n The File Name Table is empty.\n"));
      else
	{
	  printf (_("\n The File Name Table:\n"));
	  printf (_("  Entry\tDir\tTime\tSize\tName\n"));

	  while (*data != 0)
	    {
	      unsigned char *name;
	      unsigned int bytes_read;

	      printf (_("  %d\t"), ++state_machine_regs.last_file_entry);
	      name = data;

	      data += strlen ((char *) data) + 1;

	      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      printf (_("%lu\t"), read_leb128 (data, & bytes_read, 0));
	      data += bytes_read;
	      printf (_("%s\n"), name);
	    }
	}

      /* Skip the NUL at the end of the table.  */
      data++;

      /* Now display the statements.  */
      printf (_("\n Line Number Statements:\n"));

      while (data < end_of_sequence)
	{
	  unsigned char op_code;
	  int adv;
	  unsigned int bytes_read;

	  op_code = *data++;

	  if (op_code >= info.li_opcode_base)
	    {
	      op_code -= info.li_opcode_base;
	      adv      = (op_code / info.li_line_range) * info.li_min_insn_length;
	      state_machine_regs.address += adv;
	      printf (_("  Special opcode %d: advance Address by %d to 0x%lx"),
		      op_code, adv, state_machine_regs.address);
	      adv = (op_code % info.li_line_range) + info.li_line_base;
	      state_machine_regs.line += adv;
	      printf (_(" and Line by %d to %d\n"),
		      adv, state_machine_regs.line);
	    }
	  else switch (op_code)
	    {
	    case DW_LNS_extended_op:
	      if (pointer_size == 0)
		{
		  warn (_("Extend line ops need a valid pointer size, guessing at 4\n"));
		  pointer_size = 4;
		}

	      data += process_extended_line_op (data, info.li_default_is_stmt,
						pointer_size);
	      break;

	    case DW_LNS_copy:
	      printf (_("  Copy\n"));
	      break;

	    case DW_LNS_advance_pc:
	      adv = info.li_min_insn_length * read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      state_machine_regs.address += adv;
	      printf (_("  Advance PC by %d to %lx\n"), adv,
		      state_machine_regs.address);
	      break;

	    case DW_LNS_advance_line:
	      adv = read_leb128 (data, & bytes_read, 1);
	      data += bytes_read;
	      state_machine_regs.line += adv;
	      printf (_("  Advance Line by %d to %d\n"), adv,
		      state_machine_regs.line);
	      break;

	    case DW_LNS_set_file:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      printf (_("  Set File Name to entry %d in the File Name Table\n"),
		      adv);
	      state_machine_regs.file = adv;
	      break;

	    case DW_LNS_set_column:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      printf (_("  Set column to %d\n"), adv);
	      state_machine_regs.column = adv;
	      break;

	    case DW_LNS_negate_stmt:
	      adv = state_machine_regs.is_stmt;
	      adv = ! adv;
	      printf (_("  Set is_stmt to %d\n"), adv);
	      state_machine_regs.is_stmt = adv;
	      break;

	    case DW_LNS_set_basic_block:
	      printf (_("  Set basic block\n"));
	      state_machine_regs.basic_block = 1;
	      break;

	    case DW_LNS_const_add_pc:
	      adv = (((255 - info.li_opcode_base) / info.li_line_range)
		     * info.li_min_insn_length);
	      state_machine_regs.address += adv;
	      printf (_("  Advance PC by constant %d to 0x%lx\n"), adv,
		      state_machine_regs.address);
	      break;

	    case DW_LNS_fixed_advance_pc:
	      adv = byte_get (data, 2);
	      data += 2;
	      state_machine_regs.address += adv;
	      printf (_("  Advance PC by fixed size amount %d to 0x%lx\n"),
		      adv, state_machine_regs.address);
	      break;

	    case DW_LNS_set_prologue_end:
	      printf (_("  Set prologue_end to true\n"));
	      break;

	    case DW_LNS_set_epilogue_begin:
	      printf (_("  Set epilogue_begin to true\n"));
	      break;

	    case DW_LNS_set_isa:
	      adv = read_leb128 (data, & bytes_read, 0);
	      data += bytes_read;
	      printf (_("  Set ISA to %d\n"), adv);
	      break;

	    default:
	      printf (_("  Unknown opcode %d with operands: "), op_code);

	      for (i = standard_opcodes[op_code - 1]; i > 0 ; --i)
		{
		  printf ("0x%lx%s", read_leb128 (data, &bytes_read, 0),
			  i == 1 ? "" : ", ");
		  data += bytes_read;
		}
	      putchar ('\n');
	      break;
	    }
	}
      putchar ('\n');
    }

  return 1;
}

static int
display_debug_pubnames (Elf_Internal_Shdr *section,
			unsigned char *start,
			FILE *file ATTRIBUTE_UNUSED)
{
  DWARF2_Internal_PubNames pubnames;
  unsigned char *end;

  end = start + section->sh_size;

  printf (_("Contents of the %s section:\n\n"), SECTION_NAME (section));

  while (start < end)
    {
      unsigned char *data;
      unsigned long offset;
      int offset_size, initial_length_size;

      data = start;

      pubnames.pn_length = byte_get (data, 4);
      data += 4;
      if (pubnames.pn_length == 0xffffffff)
	{
	  pubnames.pn_length = byte_get (data, 8);
	  data += 8;
	  offset_size = 8;
	  initial_length_size = 12;
	}
      else
	{
	  offset_size = 4;
	  initial_length_size = 4;
	}

      pubnames.pn_version = byte_get (data, 2);
      data += 2;
      pubnames.pn_offset = byte_get (data, offset_size);
      data += offset_size;
      pubnames.pn_size = byte_get (data, offset_size);
      data += offset_size;

      start += pubnames.pn_length + initial_length_size;

      if (pubnames.pn_version != 2 && pubnames.pn_version != 3)
	{
	  static int warned = 0;

	  if (! warned)
	    {
	      warn (_("Only DWARF 2 and 3 pubnames are currently supported\n"));
	      warned = 1;
	    }

	  continue;
	}

      printf (_("  Length:                              %ld\n"),
	      pubnames.pn_length);
      printf (_("  Version:                             %d\n"),
	      pubnames.pn_version);
      printf (_("  Offset into .debug_info section:     %ld\n"),
	      pubnames.pn_offset);
      printf (_("  Size of area in .debug_info section: %ld\n"),
	      pubnames.pn_size);

      printf (_("\n    Offset\tName\n"));

      do
	{
	  offset = byte_get (data, offset_size);

	  if (offset != 0)
	    {
	      data += offset_size;
	      printf ("    %-6ld\t\t%s\n", offset, data);
	      data += strlen ((char *) data) + 1;
	    }
	}
      while (offset != 0);
    }

  printf ("\n");
  return 1;
}

static int
display_debug_macinfo(Elf_Internal_Shdr *section, unsigned char *start,
		      FILE *file ATTRIBUTE_UNUSED)
{
  unsigned char *end = (start + section->sh_size);
  unsigned char *curr = start;
  unsigned int bytes_read;
  enum dwarf_macinfo_record_type op;

  printf(_("Contents of the %s section:\n\n"), SECTION_NAME(section));

  while (curr < end)
    {
      unsigned int lineno;
      const char *string;

      op = (enum dwarf_macinfo_record_type)*curr;
      curr++;

      switch (op)
	{
	case DW_MACINFO_start_file:
	  {
	    unsigned int filenum;

	    lineno = read_leb128(curr, & bytes_read, 0);
	    curr += bytes_read;
	    filenum = read_leb128(curr, & bytes_read, 0);
	    curr += bytes_read;

	    printf(_(" DW_MACINFO_start_file - lineno: %d filenum: %d\n"),
		   lineno, filenum);
	  }
	  break;

	case DW_MACINFO_end_file:
	  printf(_(" DW_MACINFO_end_file\n"));
	  break;

	case DW_MACINFO_define:
	  lineno = read_leb128(curr, & bytes_read, 0);
	  curr += bytes_read;
	  string = (char *)curr;
	  curr += (strlen(string) + 1UL);
	  printf(_(" DW_MACINFO_define - lineno : %d macro : %s\n"),
		 lineno, string);
	  break;

	case DW_MACINFO_undef:
	  lineno = read_leb128(curr, & bytes_read, 0);
	  curr += bytes_read;
	  string = (char *)curr;
	  curr += (strlen(string) + 1UL);
	  printf(_(" DW_MACINFO_undef - lineno : %d macro : %s\n"),
		 lineno, string);
	  break;

	case DW_MACINFO_vendor_ext:
	  {
	    unsigned int constant;

	    constant = read_leb128(curr, & bytes_read, 0);
	    curr += bytes_read;
	    string = (char *)curr;
	    curr += (strlen(string) + 1UL);
	    printf(_(" DW_MACINFO_vendor_ext - constant : %d string : %s\n"),
		   constant, string);
	  }
	  break;

        default:
          break;
	}
    }

  return 1;
}


static int
display_debug_abbrev (Elf_Internal_Shdr *section,
		      unsigned char *start,
		      FILE *file ATTRIBUTE_UNUSED)
{
  abbrev_entry *entry;
  unsigned char *end = start + section->sh_size;

  printf (_("Contents of the %s section:\n\n"), SECTION_NAME (section));

  do
    {
      start = process_abbrev_section (start, end);

      if (first_abbrev == NULL)
	continue;

      printf (_("  Number TAG\n"));

      for (entry = first_abbrev; entry; entry = entry->next)
	{
	  abbrev_attr *attr;

	  printf (_("   %ld      %s    [%s]\n"),
		  entry->entry,
		  get_TAG_name (entry->tag),
		  entry->children ? _("has children") : _("no children"));

	  for (attr = entry->first_attr; attr; attr = attr->next)
	    printf (_("    %-18s %s\n"),
		    get_AT_name (attr->attribute),
		    get_FORM_name (attr->form));
	}

      free_abbrevs ();
    }
  while (start);

  printf ("\n");

  return 1;
}

static int
display_debug_loc (Elf_Internal_Shdr *section,
		   unsigned char *start, FILE *file)
{
  unsigned char *section_end;
  unsigned long bytes;
  unsigned char *section_begin = start;
  unsigned int num_loc_list = 0;
  unsigned long last_offset = 0;
  unsigned int first = 0;
  unsigned int i;
  unsigned int j;
  int seen_first_offset = 0;
  int use_debug_info = 1;
  unsigned char *next;

  bytes = section->sh_size;
  section_end = start + bytes;

  if (bytes == 0)
    {
      printf (_("\nThe .debug_loc section is empty.\n"));
      return 0;
    }

  get_debug_info (file);

  /* Check the order of location list in .debug_info section. If
     offsets of location lists are in the ascending order, we can
     use `debug_information' directly.  */
  for (i = 0; i < num_debug_info_entries; i++)
    {
      unsigned int num;

      num = debug_information [i].num_loc_offsets;
      num_loc_list += num;

      /* Check if we can use `debug_information' directly.  */
      if (use_debug_info && num != 0)
	{
	  if (!seen_first_offset)
	    {
	      /* This is the first location list.  */
	      last_offset = debug_information [i].loc_offsets [0];
	      first = i;
	      seen_first_offset = 1;
	      j = 1;
	    }
	  else
	    j = 0;

	  for (; j < num; j++)
	    {
	      if (last_offset >
		  debug_information [i].loc_offsets [j])
		{
		  use_debug_info = 0;
		  break;
		}
	      last_offset = debug_information [i].loc_offsets [j];
	    }
	}
    }

  if (!use_debug_info)
    /* FIXME: Should we handle this case?  */
    error (_("Location lists in .debug_info section aren't in ascending order!\n"));

  if (!seen_first_offset)
    error (_("No location lists in .debug_info section!\n"));

  if (debug_information [first].loc_offsets [0] != 0)
    warn (_("Location lists in .debug_loc section start at 0x%lx\n"),
	  debug_information [first].loc_offsets [0]);

  printf (_("Contents of the .debug_loc section:\n\n"));
  printf (_("    Offset   Begin    End      Expression\n"));

  seen_first_offset = 0;
  for (i = first; i < num_debug_info_entries; i++)
    {
      unsigned long begin;
      unsigned long end;
      unsigned short length;
      unsigned long offset;
      unsigned int pointer_size;
      unsigned long cu_offset;
      unsigned long base_address;
      int need_frame_base;
      int has_frame_base;

      pointer_size = debug_information [i].pointer_size;
      cu_offset = debug_information [i].cu_offset;

      for (j = 0; j < debug_information [i].num_loc_offsets; j++)
	{
	  has_frame_base = debug_information [i].have_frame_base [j];
	  offset = debug_information [i].loc_offsets [j];
	  next = section_begin + offset;
	  base_address = debug_information [i].base_address;

	  if (!seen_first_offset)
	    seen_first_offset = 1;
	  else
	    {
	      if (start < next)
		warn (_("There is a hole [0x%lx - 0x%lx] in .debug_loc section.\n"),
		      (long)(start - section_begin), (long)(next - section_begin));
	      else if (start > next)
		warn (_("There is an overlap [0x%lx - 0x%lx] in .debug_loc section.\n"),
		      (long)(start - section_begin), (long)(next - section_begin));
	    }
	  start = next;

	  if (offset >= bytes)
	    {
	      warn (_("Offset 0x%lx is bigger than .debug_loc section size.\n"),
		    offset);
	      continue;
	    }

	  while (1)
	    {
	      if (start + 2 * pointer_size > section_end)
		{
		  warn (_("Location list starting at offset 0x%lx is not terminated.\n"),
			offset);
		  break;
		}

	      begin = byte_get (start, pointer_size);
	      start += pointer_size;
	      end = byte_get (start, pointer_size);
	      start += pointer_size;

	      if (begin == 0 && end == 0)
		{
		  printf (_("    %8.8lx <End of list>\n"), offset);
		  break;
		}

	      /* Check base address specifiers.  */
	      if (begin == -1UL && end != -1UL)
		{
		  base_address = end;
		  printf (_("    %8.8lx %8.8lx %8.8lx (base address)\n"),
			  offset, begin, end);
		  continue;
		}

	      if (start + 2 > section_end)
		{
		  warn (_("Location list starting at offset 0x%lx is not terminated.\n"),
			offset);
		  break;
		}

	      length = byte_get (start, 2);
	      start += 2;

	      if (start + length > section_end)
		{
		  warn (_("Location list starting at offset 0x%lx is not terminated.\n"),
			offset);
		  break;
		}

	      printf ("    %8.8lx %8.8lx %8.8lx (",
		      offset, begin + base_address, end + base_address);
	      need_frame_base = decode_location_expression (start,
							    pointer_size,
							    length,
							    cu_offset);
	      putchar (')');

	      if (need_frame_base && !has_frame_base)
		printf (_(" [without DW_AT_frame_base]"));

	      if (begin == end)
		fputs (_(" (start == end)"), stdout);
	      else if (begin > end)
		fputs (_(" (start > end)"), stdout);

	      putchar ('\n');

	      start += length;
	    }
	}
    }
  return 1;
}

static int
display_debug_str (Elf_Internal_Shdr *section,
		   unsigned char *start,
		   FILE *file ATTRIBUTE_UNUSED)
{
  unsigned long bytes;
  bfd_vma addr;

  addr  = section->sh_addr;
  bytes = section->sh_size;

  if (bytes == 0)
    {
      printf (_("\nThe .debug_str section is empty.\n"));
      return 0;
    }

  printf (_("Contents of the .debug_str section:\n\n"));

  while (bytes)
    {
      int j;
      int k;
      int lbytes;

      lbytes = (bytes > 16 ? 16 : bytes);

      printf ("  0x%8.8lx ", (unsigned long) addr);

      for (j = 0; j < 16; j++)
	{
	  if (j < lbytes)
	    printf ("%2.2x", start[j]);
	  else
	    printf ("  ");

	  if ((j & 3) == 3)
	    printf (" ");
	}

      for (j = 0; j < lbytes; j++)
	{
	  k = start[j];
	  if (k >= ' ' && k < 0x80)
	    printf ("%c", k);
	  else
	    printf (".");
	}

      putchar ('\n');

      start += lbytes;
      addr  += lbytes;
      bytes -= lbytes;
    }

  putchar ('\n');

  return 1;
}


static int
display_debug_info (Elf_Internal_Shdr * section,
		    unsigned char * start, FILE * file)
{
  return process_debug_info (section, start, file, 0);
}


static int
display_debug_aranges (Elf_Internal_Shdr *section,
		       unsigned char *start,
		       FILE *file ATTRIBUTE_UNUSED)
{
  unsigned char *end = start + section->sh_size;

  printf (_("The section %s contains:\n\n"), SECTION_NAME (section));

  while (start < end)
    {
      unsigned char *hdrptr;
      DWARF2_Internal_ARange arange;
      unsigned char *ranges;
      unsigned long length;
      unsigned long address;
      int excess;
      int offset_size;
      int initial_length_size;

      hdrptr = start;

      arange.ar_length = byte_get (hdrptr, 4);
      hdrptr += 4;

      if (arange.ar_length == 0xffffffff)
	{
	  arange.ar_length = byte_get (hdrptr, 8);
	  hdrptr += 8;
	  offset_size = 8;
	  initial_length_size = 12;
	}
      else
	{
	  offset_size = 4;
	  initial_length_size = 4;
	}

      arange.ar_version = byte_get (hdrptr, 2);
      hdrptr += 2;

      arange.ar_info_offset = byte_get (hdrptr, offset_size);
      hdrptr += offset_size;

      arange.ar_pointer_size = byte_get (hdrptr, 1);
      hdrptr += 1;

      arange.ar_segment_size = byte_get (hdrptr, 1);
      hdrptr += 1;

      if (arange.ar_version != 2 && arange.ar_version != 3)
	{
	  warn (_("Only DWARF 2 and 3 aranges are currently supported.\n"));
	  break;
	}

      printf (_("  Length:                   %ld\n"), arange.ar_length);
      printf (_("  Version:                  %d\n"), arange.ar_version);
      printf (_("  Offset into .debug_info:  %lx\n"), arange.ar_info_offset);
      printf (_("  Pointer Size:             %d\n"), arange.ar_pointer_size);
      printf (_("  Segment Size:             %d\n"), arange.ar_segment_size);

      printf (_("\n    Address  Length\n"));

      ranges = hdrptr;

      /* Must pad to an alignment boundary that is twice the pointer size.  */
      excess = (hdrptr - start) % (2 * arange.ar_pointer_size);
      if (excess)
	ranges += (2 * arange.ar_pointer_size) - excess;

      for (;;)
	{
	  address = byte_get (ranges, arange.ar_pointer_size);

	  ranges += arange.ar_pointer_size;

	  length  = byte_get (ranges, arange.ar_pointer_size);

	  ranges += arange.ar_pointer_size;

	  /* A pair of zeros marks the end of the list.  */
	  if (address == 0 && length == 0)
	    break;

	  printf ("    %8.8lx %lu\n", address, length);
	}

      start += arange.ar_length + initial_length_size;
    }

  printf ("\n");

  return 1;
}

static int
display_debug_ranges(Elf_Internal_Shdr *section, unsigned char *start,
		     FILE *file)
{
  unsigned char *section_end;
  unsigned long bytes;
  unsigned char *section_begin = start;
  unsigned int num_range_list = 0U;
  unsigned long last_offset = 0UL;
  unsigned int first = 0;
  unsigned int i;
  unsigned int j;
  int seen_first_offset = 0;
  int use_debug_info = 1;
  unsigned char *next;

  bytes = section->sh_size;
  section_end = (start + bytes);

  if (bytes == 0)
    {
      printf(_("\nThe .debug_ranges section is empty.\n"));
      return 0;
    }

  if (section_end != NULL) {
    ;
  }

  get_debug_info(file);

  /* Check the order of range list in .debug_info section. If
     offsets of range lists are in the ascending order, we can
     use `debug_information' directly.  */
  for (i = 0; i < num_debug_info_entries; i++)
    {
      unsigned int num;

      num = debug_information [i].num_range_lists;
      num_range_list += num;

      /* Check if we can use `debug_information' directly.  */
      if (use_debug_info && num != 0)
	{
	  if (!seen_first_offset)
	    {
	      /* This is the first range list.  */
	      last_offset = debug_information [i].range_lists [0];
	      first = i;
	      seen_first_offset = 1;
	      j = 1;
	    }
	  else
	    j = 0;

	  for (; j < num; j++)
	    {
	      if (last_offset >
		  debug_information [i].range_lists [j])
		{
		  use_debug_info = 0;
		  break;
		}
	      last_offset = debug_information [i].range_lists [j];
	    }
	}
    }

  if (!use_debug_info)
    /* FIXME: Should we handle this case?  */
    error (_("Range lists in .debug_info section aren't in ascending order!\n"));

  if (!seen_first_offset)
    error (_("No range lists in .debug_info section!\n"));

  if (debug_information [first].range_lists [0] != 0)
    warn (_("Range lists in .debug_ranges section start at 0x%lx\n"),
	  debug_information [first].range_lists [0]);

  printf (_("Contents of the .debug_ranges section:\n\n"));
  printf (_("    Offset   Begin    End\n"));

  seen_first_offset = 0;
  for (i = first; i < num_debug_info_entries; i++)
    {
      unsigned long begin;
      unsigned long end;
      unsigned long offset;
      unsigned int pointer_size;
      unsigned long base_address;

      pointer_size = debug_information [i].pointer_size;

      for (j = 0; j < debug_information [i].num_range_lists; j++)
	{
	  offset = debug_information [i].range_lists [j];
	  next = section_begin + offset;
	  base_address = debug_information [i].base_address;

	  if (!seen_first_offset)
	    seen_first_offset = 1;
	  else
	    {
	      if (start < next)
		warn (_("There is a hole [0x%lx - 0x%lx] in .debug_ranges section.\n"),
		      (long)(start - section_begin), (long)(next - section_begin));
	      else if (start > next)
		warn (_("There is an overlap [0x%lx - 0x%lx] in .debug_ranges section.\n"),
		      (long)(start - section_begin), (long)(next - section_begin));
	    }
	  start = next;

	  while (1)
	    {
	      begin = byte_get (start, pointer_size);
	      start += pointer_size;
	      end = byte_get (start, pointer_size);
	      start += pointer_size;

	      if (begin == 0 && end == 0)
		{
		  printf (_("    %8.8lx <End of list>\n"), offset);
		  break;
		}

	      /* Check base address specifiers.  */
	      if (begin == -1UL && end != -1UL)
		{
		  base_address = end;
		  printf ("    %8.8lx %8.8lx %8.8lx (base address)\n",
			  offset, begin, end);
		  continue;
		}

	      printf ("    %8.8lx %8.8lx %8.8lx",
		      offset, begin + base_address, end + base_address);

	      if (begin == end)
		fputs (_(" (start == end)"), stdout);
	      else if (begin > end)
		fputs (_(" (start > end)"), stdout);

	      putchar ('\n');
	    }
	}
    }
  putchar ('\n');
  return 1;
}

typedef struct Frame_Chunk
{
  struct Frame_Chunk *next;
  unsigned char *chunk_start;
  int ncols;
  /* DW_CFA_{undefined,same_value,offset,register,unreferenced}  */
  short int *col_type;
  int *col_offset;
  const char *augmentation;
  unsigned int code_factor;
  int data_factor;
  unsigned long pc_begin;
  unsigned long pc_range;
  int cfa_reg;
  int cfa_offset;
  int ra;
  unsigned char fde_encoding; /* FIXME: embiggen? */
  unsigned char cfa_exp;
}
Frame_Chunk;

/* A marker for a col_type that means this column was never referenced
   in the frame info.  */
#define DW_CFA_unreferenced (-1)

static void
frame_need_space(Frame_Chunk *fc, int reg)
{
  int prev = fc->ncols;

  if (reg < fc->ncols)
    return;

  fc->ncols = (reg + 1);
  fc->col_type = (short int *)xcrealloc(fc->col_type, fc->ncols,
                                        sizeof(short int));
  fc->col_offset = (int *)xcrealloc(fc->col_offset, fc->ncols,
                                    sizeof(int));

  while (prev < fc->ncols)
    {
      fc->col_type[prev] = DW_CFA_unreferenced;
      fc->col_offset[prev] = 0;
      prev++;
    }
}

static void
frame_display_row (Frame_Chunk *fc, int *need_col_headers, int *max_regs)
{
  int r;
  char tmp[100];

  if (*max_regs < fc->ncols)
    *max_regs = fc->ncols;

  if (*need_col_headers)
    {
      *need_col_headers = 0;

      printf ("   LOC   CFA      ");

      for (r = 0; r < *max_regs; r++)
	if (fc->col_type[r] != DW_CFA_unreferenced)
	  {
	    if (r == fc->ra)
	      printf ("ra   ");
	    else
	      printf ("r%-4d", r);
	  }

      printf ("\n");
    }

  printf ("%08lx ", fc->pc_begin);
  if (fc->cfa_exp)
    strcpy (tmp, "exp");
  else
    sprintf (tmp, "r%d%+d", fc->cfa_reg, fc->cfa_offset);
  printf ("%-8s ", tmp);

  for (r = 0; r < fc->ncols; r++)
    {
      if (fc->col_type[r] != DW_CFA_unreferenced)
	{
	  switch (fc->col_type[r])
	    {
	    case DW_CFA_undefined:
	      strcpy (tmp, "u");
	      break;
	    case DW_CFA_same_value:
	      strcpy (tmp, "s");
	      break;
	    case DW_CFA_offset:
	      sprintf (tmp, "c%+d", fc->col_offset[r]);
	      break;
	    case DW_CFA_register:
	      sprintf (tmp, "r%d", fc->col_offset[r]);
	      break;
	    case DW_CFA_expression:
	      strcpy (tmp, "exp");
	      break;
	    default:
	      strcpy (tmp, "n/a");
	      break;
	    }
	  printf ("%-5s", tmp);
	}
    }
  printf ("\n");
}

static int
size_of_encoded_value (int encoding)
{
  switch (encoding & 0x7)
    {
    default:	/* ??? */
    case 0:	return eh_addr_size;
    case 2:	return 2;
    case 3:	return 4;
    case 4:	return 8;
    }
}

static bfd_vma
get_encoded_value (unsigned char *data, int encoding)
{
  int size = size_of_encoded_value (encoding);
  if (encoding & DW_EH_PE_signed)
    return byte_get_signed (data, size);
  else
    return byte_get (data, size);
}

#define GET(N)	byte_get (start, N); start += N
#define LEB()	read_leb128 (start, & length_return, 0); start += length_return
#define SLEB()	read_leb128 (start, & length_return, 1); start += length_return

/* */
static int
display_debug_frames(Elf_Internal_Shdr *section, unsigned char *start,
		     FILE *file)
{
  unsigned char *end = (start + section->sh_size);
  unsigned char *section_start = start;
  Frame_Chunk *chunks = (Frame_Chunk *)0;
  Frame_Chunk *remembered_state = (Frame_Chunk *)0;
  Frame_Chunk *rs;
  int is_eh = streq(SECTION_NAME(section), ".eh_frame");
  unsigned int length_return;
  int max_regs = 0;

  printf(_("The section %s contains:\n"), SECTION_NAME(section));

  while (start < end)
    {
      unsigned char *saved_start;
      unsigned char *block_end;
      unsigned long length;
      unsigned long cie_id;
      Frame_Chunk *fc;
      Frame_Chunk *cie;
      int need_col_headers = 1;
      unsigned char *augmentation_data = NULL;
      unsigned long augmentation_data_len = 0UL;
      int encoded_ptr_size = (int)eh_addr_size;
      int offset_size;
      int initial_length_size;

      saved_start = start;
      length = byte_get(start, 4); start += 4;

      if (length == 0)
	{
	  printf("\n%08lx ZERO terminator\n\n",
		 (unsigned long)(saved_start - section_start));
	  return 1;
	}

      if (length == 0xffffffff)
	{
	  length = byte_get (start, 8);
	  start += 8;
	  offset_size = 8;
	  initial_length_size = 12;
	}
      else
	{
	  offset_size = 4;
	  initial_length_size = 4;
	}

      block_end = saved_start + length + initial_length_size;
      cie_id = byte_get (start, offset_size); start += offset_size;

      if (elf_header.e_type == ET_REL
	  && !debug_apply_rela_addends(file, section, offset_size,
				       section_start, start, block_end))
	return 0;

      if (is_eh ? (cie_id == 0) : (cie_id == DW_CIE_ID))
	{
	  int version;

	  fc = (Frame_Chunk *)xmalloc(sizeof(Frame_Chunk));
	  memset(fc, 0, sizeof(Frame_Chunk));

	  fc->next = chunks;
	  chunks = fc;
	  fc->chunk_start = saved_start;
	  fc->ncols = 0;
	  fc->col_type = (short int *)xmalloc(sizeof(short int));
	  fc->col_offset = (int *)xmalloc(sizeof(int));
	  frame_need_space(fc, (max_regs - 1));

	  version = *start++;

	  fc->augmentation = (char *)start;
	  start = (unsigned char *)strchr((char *)start, '\0') + 1;

	  if (fc->augmentation[0] == 'z')
	    {
	      fc->code_factor = LEB();
	      fc->data_factor = SLEB();
	      if (version == 1)
		{
		  fc->ra = GET(1);
		}
	      else
		{
		  fc->ra = LEB ();
		}
	      augmentation_data_len = LEB ();
	      augmentation_data = start;
	      start += augmentation_data_len;
	    }
	  else if (streq (fc->augmentation, "eh"))
	    {
	      start += eh_addr_size;
	      fc->code_factor = LEB ();
	      fc->data_factor = SLEB ();
	      if (version == 1)
		{
		  fc->ra = GET (1);
		}
	      else
		{
		  fc->ra = LEB ();
		}
	    }
	  else
	    {
	      fc->code_factor = LEB ();
	      fc->data_factor = SLEB ();
	      if (version == 1)
		{
		  fc->ra = GET (1);
		}
	      else
		{
		  fc->ra = LEB ();
		}
	    }
	  cie = fc;

	  if (do_debug_frames_interp)
	    printf ("\n%08lx %08lx %08lx CIE \"%s\" cf=%d df=%d ra=%d\n",
		    (unsigned long)(saved_start - section_start), length, cie_id,
		    fc->augmentation, fc->code_factor, fc->data_factor,
		    fc->ra);
	  else
	    {
	      printf ("\n%08lx %08lx %08lx CIE\n",
		      (unsigned long)(saved_start - section_start), length, cie_id);
	      printf ("  Version:               %d\n", version);
	      printf ("  Augmentation:          \"%s\"\n", fc->augmentation);
	      printf ("  Code alignment factor: %u\n", fc->code_factor);
	      printf ("  Data alignment factor: %d\n", fc->data_factor);
	      printf ("  Return address column: %d\n", fc->ra);

	      if (augmentation_data_len)
		{
		  unsigned long i;
		  printf ("  Augmentation data:    ");
		  for (i = 0; i < augmentation_data_len; ++i)
		    printf (" %02x", augmentation_data[i]);
		  putchar ('\n');
		}
	      putchar ('\n');
	    }

	  if (augmentation_data_len)
	    {
	      unsigned char *p, *q;
	      p = (unsigned char *) fc->augmentation + 1;
	      q = augmentation_data;

	      while (1)
		{
		  if (*p == 'L')
		    q++;
		  else if (*p == 'P')
		    q += 1 + size_of_encoded_value (*q);
		  else if (*p == 'R')
		    fc->fde_encoding = *q++; /* FIXME: -Wstringop-overflow */
		  else
		    break;
		  p++;
		}

	      if (fc->fde_encoding)
		encoded_ptr_size = size_of_encoded_value (fc->fde_encoding);
	    }

	  frame_need_space (fc, fc->ra);
	}
      else
	{
	  unsigned char *look_for;
	  static Frame_Chunk fde_fc;

	  fc = & fde_fc;
	  memset (fc, 0, sizeof (Frame_Chunk));

	  look_for = is_eh ? start - 4 - cie_id : section_start + cie_id;

	  for (cie = chunks; cie ; cie = cie->next)
	    if (cie->chunk_start == look_for)
	      break;

	  if (!cie)
	    {
	      warn("Invalid CIE pointer %08lx in FDE at %p\n",
                   cie_id, (void *)saved_start);
	      start = block_end;
	      fc->ncols = 0;
	      fc->col_type = (short int *)xmalloc(sizeof(short int));
	      fc->col_offset = (int *)xmalloc(sizeof(int));
	      frame_need_space(fc, (max_regs - 1));
	      cie = fc;
	      fc->augmentation = "";
	      fc->fde_encoding = 0;
	    }
	  else
	    {
	      fc->ncols = cie->ncols;
	      fc->col_type = (short int *)xcmalloc(fc->ncols,
                                                   sizeof(short int));
	      fc->col_offset = (int *)xcmalloc(fc->ncols, sizeof(int));
	      memcpy(fc->col_type, cie->col_type,
                     ((size_t)fc->ncols * sizeof(short int)));
	      memcpy(fc->col_offset, cie->col_offset,
                     ((size_t)fc->ncols * sizeof(int)));
	      fc->augmentation = cie->augmentation;
	      fc->code_factor = cie->code_factor;
	      fc->data_factor = cie->data_factor;
	      fc->cfa_reg = cie->cfa_reg;
	      fc->cfa_offset = cie->cfa_offset;
	      fc->ra = cie->ra;
	      frame_need_space(fc, (max_regs - 1));
	      fc->fde_encoding = cie->fde_encoding;
	    }

	  if (fc->fde_encoding)
	    encoded_ptr_size = size_of_encoded_value(fc->fde_encoding);

	  fc->pc_begin = get_encoded_value(start, fc->fde_encoding);
	  if ((fc->fde_encoding & 0x70) == DW_EH_PE_pcrel
	      /* Do NOT adjust for ET_REL since there is invariably a pcrel
	       * reloc here, which we have NOT applied.  */
	      && elf_header.e_type != ET_REL)
	    fc->pc_begin += section->sh_addr + (start - section_start);
	  start += encoded_ptr_size;
	  fc->pc_range = byte_get(start, encoded_ptr_size);
	  start += encoded_ptr_size;

	  if (cie->augmentation[0] == 'z')
	    {
	      augmentation_data_len = LEB();
	      augmentation_data = start;
	      start += augmentation_data_len;
	    }

	  printf ("\n%08lx %08lx %08lx FDE cie=%08lx pc=%08lx..%08lx\n",
		  (unsigned long)(saved_start - section_start), length, cie_id,
		  (unsigned long)(cie->chunk_start - section_start),
		  fc->pc_begin, fc->pc_begin + fc->pc_range);
	  if (! do_debug_frames_interp && augmentation_data_len)
	    {
	      unsigned long i;

	      printf ("  Augmentation data:    ");
	      for (i = 0; i < augmentation_data_len; ++i)
		printf (" %02x", augmentation_data[i]);
	      putchar ('\n');
	      putchar ('\n');
	    }
	}

      /* At this point, fc is the current chunk, cie (if any) is set, and
       * we are about to interpret instructions for the chunk.  */
      /* ??? At present we need to do this always, since this sizes the
       * fc->col_type and fc->col_offset arrays, which we write into always.
       * We should probably split the interpreted and non-interpreted bits
       * into two different routines, since there is so much that does NOT
       * really overlap between them.  */
      if (1 || do_debug_frames_interp)
	{
	  /* Start by making a pass over the chunk, allocating storage
	     and taking note of what registers are used.  */
	  unsigned char *tmp_string = start;

	  while (start < block_end)
	    {
	      unsigned op, opa;
	      unsigned long reg, tmp_l;

	      op = *start++;
	      opa = (op & 0x3f);
	      if (op & 0xc0) {
		op &= 0xc0;
              }

	      /* Warning: if you add any more cases to this switch, be
	       * sure to add them to the corresponding switch below.  */
	      switch (op)
		{
		case DW_CFA_advance_loc:
		  break;
		case DW_CFA_offset:
		  LEB();
		  frame_need_space(fc, opa);
		  fc->col_type[opa] = DW_CFA_undefined;
		  break;
		case DW_CFA_restore:
		  frame_need_space(fc, opa);
		  fc->col_type[opa] = DW_CFA_undefined;
		  break;
		case DW_CFA_set_loc:
		  start += encoded_ptr_size;
		  break;
		case DW_CFA_advance_loc1:
		  start += 1;
		  break;
		case DW_CFA_advance_loc2:
		  start += 2;
		  break;
		case DW_CFA_advance_loc4:
		  start += 4;
		  break;
		case DW_CFA_offset_extended:
		  reg = LEB(); LEB();
		  frame_need_space(fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_restore_extended:
		  reg = LEB();
		  frame_need_space(fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_undefined:
		  reg = LEB();
		  frame_need_space(fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_same_value:
		  reg = LEB();
		  frame_need_space(fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_register:
		  reg = LEB(); LEB();
		  frame_need_space (fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_def_cfa:
		  LEB(); LEB();
		  break;
		case DW_CFA_def_cfa_register:
		  LEB();
		  break;
		case DW_CFA_def_cfa_offset:
		  LEB();
		  break;
		case DW_CFA_def_cfa_expression:
		  tmp_l = LEB();
		  start += tmp_l;
		  break;
		case DW_CFA_expression:
		  reg = LEB();
		  tmp_l = LEB();
		  start += tmp_l;
		  frame_need_space(fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_offset_extended_sf:
		  reg = LEB(); SLEB();
		  frame_need_space(fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  break;
		case DW_CFA_def_cfa_sf:
		  LEB(); SLEB();
		  break;
		case DW_CFA_def_cfa_offset_sf:
		  SLEB();
		  break;
		case DW_CFA_MIPS_advance_loc8:
		  start += 8;
		  break;
		case DW_CFA_GNU_args_size:
		  LEB();
		  break;
		case DW_CFA_GNU_negative_offset_extended:
		  reg = LEB(); LEB();
		  frame_need_space(fc, reg);
		  fc->col_type[reg] = DW_CFA_undefined;
		  ATTRIBUTE_FALLTHROUGH; /* ...I guess? */
		default:
		  break;
		}
	    }
	  start = tmp_string;
	}

      /* Now we know what registers are used, make a second pass over
       * the chunk, this time actually printing out the info.  */

      while (start < block_end)
	{
	  unsigned int op, opa;
	  unsigned long ul, reg, roffs;
	  long l, ofs;
	  bfd_vma vma;

	  op = *start++;
	  opa = (op & 0x3f);
	  if (op & 0xc0)
	    op &= 0xc0;

	  /* Warning: if you add any more cases to this switch, be
	     sure to add them to the corresponding switch above.  */
	  switch (op)
	    {
	    case DW_CFA_advance_loc:
	      if (do_debug_frames_interp)
		frame_display_row(fc, &need_col_headers, &max_regs);
	      else
		printf("  DW_CFA_advance_loc: %d to %08lx\n",
                       (opa * fc->code_factor),
                       (fc->pc_begin + (opa * fc->code_factor)));
	      fc->pc_begin += (opa * fc->code_factor);
	      break;

	    case DW_CFA_offset:
	      roffs = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_offset: r%d at cfa%+ld\n",
			opa, roffs * fc->data_factor);
	      fc->col_type[opa] = DW_CFA_offset;
	      fc->col_offset[opa] = roffs * fc->data_factor;
	      break;

	    case DW_CFA_restore:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_restore: r%d\n", opa);
	      fc->col_type[opa] = cie->col_type[opa];
	      fc->col_offset[opa] = cie->col_offset[opa];
	      break;

	    case DW_CFA_set_loc:
	      vma = get_encoded_value (start, fc->fde_encoding);
	      if ((fc->fde_encoding & 0x70) == DW_EH_PE_pcrel
		  && elf_header.e_type != ET_REL)
		vma += section->sh_addr + (start - section_start);
	      start += encoded_ptr_size;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_set_loc: %08lx\n", (unsigned long)vma);
	      fc->pc_begin = vma;
	      break;

	    case DW_CFA_advance_loc1:
	      ofs = byte_get (start, 1); start += 1;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_advance_loc1: %ld to %08lx\n",
			ofs * fc->code_factor,
			fc->pc_begin + ofs * fc->code_factor);
	      fc->pc_begin += ofs * fc->code_factor;
	      break;

	    case DW_CFA_advance_loc2:
	      ofs = byte_get (start, 2); start += 2;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_advance_loc2: %ld to %08lx\n",
			ofs * fc->code_factor,
			fc->pc_begin + ofs * fc->code_factor);
	      fc->pc_begin += ofs * fc->code_factor;
	      break;

	    case DW_CFA_advance_loc4:
	      ofs = byte_get (start, 4); start += 4;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_advance_loc4: %ld to %08lx\n",
			ofs * fc->code_factor,
			fc->pc_begin + ofs * fc->code_factor);
	      fc->pc_begin += ofs * fc->code_factor;
	      break;

	    case DW_CFA_offset_extended:
	      reg = LEB ();
	      roffs = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_offset_extended: r%ld at cfa%+ld\n",
			reg, roffs * fc->data_factor);
	      fc->col_type[reg] = DW_CFA_offset;
	      fc->col_offset[reg] = roffs * fc->data_factor;
	      break;

	    case DW_CFA_restore_extended:
	      reg = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_restore_extended: r%ld\n", reg);
	      fc->col_type[reg] = cie->col_type[reg];
	      fc->col_offset[reg] = cie->col_offset[reg];
	      break;

	    case DW_CFA_undefined:
	      reg = LEB();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_undefined: r%ld\n", reg);
	      fc->col_type[reg] = DW_CFA_undefined;
	      fc->col_offset[reg] = 0;
	      break;

	    case DW_CFA_same_value:
	      reg = LEB();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_same_value: r%ld\n", reg);
	      fc->col_type[reg] = DW_CFA_same_value;
	      fc->col_offset[reg] = 0;
	      break;

	    case DW_CFA_register:
	      reg = LEB();
	      roffs = LEB();
	      if (! do_debug_frames_interp)
		printf("  DW_CFA_register: r%ld in r%ld\n", reg, roffs);
	      fc->col_type[reg] = DW_CFA_register;
	      fc->col_offset[reg] = roffs;
	      break;

	    case DW_CFA_remember_state:
	      if (! do_debug_frames_interp)
		printf("  DW_CFA_remember_state\n");
	      rs = (Frame_Chunk *)xmalloc(sizeof(Frame_Chunk));
	      rs->ncols = fc->ncols;
	      rs->col_type = (short int *)xcmalloc(rs->ncols,
                                                   sizeof(short int));
	      rs->col_offset = (int *)xcmalloc(rs->ncols, sizeof(int));
	      memcpy(rs->col_type, fc->col_type, rs->ncols);
	      memcpy(rs->col_offset, fc->col_offset,
                     (rs->ncols * sizeof(int)));
	      rs->next = remembered_state;
	      remembered_state = rs;
	      break;

	    case DW_CFA_restore_state:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_restore_state\n");
	      rs = remembered_state;
	      if (rs)
		{
		  remembered_state = rs->next;
		  frame_need_space (fc, rs->ncols-1);
		  memcpy (fc->col_type, rs->col_type, rs->ncols);
		  memcpy (fc->col_offset, rs->col_offset,
			  rs->ncols * sizeof (int));
		  free (rs->col_type);
		  free (rs->col_offset);
		  free (rs);
		}
	      else if (do_debug_frames_interp)
		printf ("Mismatched DW_CFA_restore_state\n");
	      break;

	    case DW_CFA_def_cfa:
	      fc->cfa_reg = LEB ();
	      fc->cfa_offset = LEB ();
	      fc->cfa_exp = 0;
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa: r%d ofs %d\n",
			fc->cfa_reg, fc->cfa_offset);
	      break;

	    case DW_CFA_def_cfa_register:
	      fc->cfa_reg = LEB ();
	      fc->cfa_exp = 0;
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa_reg: r%d\n", fc->cfa_reg);
	      break;

	    case DW_CFA_def_cfa_offset:
	      fc->cfa_offset = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa_offset: %d\n", fc->cfa_offset);
	      break;

	    case DW_CFA_nop:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_nop\n");
	      break;

	    case DW_CFA_def_cfa_expression:
	      ul = LEB ();
	      if (! do_debug_frames_interp)
		{
		  printf ("  DW_CFA_def_cfa_expression (");
		  decode_location_expression (start, eh_addr_size, ul, 0);
		  printf (")\n");
		}
	      fc->cfa_exp = 1;
	      start += ul;
	      break;

	    case DW_CFA_expression:
	      reg = LEB ();
	      ul = LEB ();
	      if (! do_debug_frames_interp)
		{
		  printf ("  DW_CFA_expression: r%ld (", reg);
		  decode_location_expression (start, eh_addr_size, ul, 0);
		  printf (")\n");
		}
	      fc->col_type[reg] = DW_CFA_expression;
	      start += ul;
	      break;

	    case DW_CFA_offset_extended_sf:
	      reg = LEB ();
	      l = SLEB ();
	      frame_need_space (fc, reg);
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_offset_extended_sf: r%ld at cfa%+ld\n",
			reg, l * fc->data_factor);
	      fc->col_type[reg] = DW_CFA_offset;
	      fc->col_offset[reg] = l * fc->data_factor;
	      break;

	    case DW_CFA_def_cfa_sf:
	      fc->cfa_reg = LEB ();
	      fc->cfa_offset = SLEB ();
	      fc->cfa_offset = fc->cfa_offset * fc->data_factor;
	      fc->cfa_exp = 0;
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa_sf: r%d ofs %d\n",
			fc->cfa_reg, fc->cfa_offset);
	      break;

	    case DW_CFA_def_cfa_offset_sf:
	      fc->cfa_offset = SLEB ();
	      fc->cfa_offset = fc->cfa_offset * fc->data_factor;
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_def_cfa_offset_sf: %d\n", fc->cfa_offset);
	      break;

	    case DW_CFA_MIPS_advance_loc8:
	      ofs = byte_get (start, 8); start += 8;
	      if (do_debug_frames_interp)
		frame_display_row (fc, &need_col_headers, &max_regs);
	      else
		printf ("  DW_CFA_MIPS_advance_loc8: %ld to %08lx\n",
			ofs * fc->code_factor,
			fc->pc_begin + ofs * fc->code_factor);
	      fc->pc_begin += ofs * fc->code_factor;
	      break;

	    case DW_CFA_GNU_window_save:
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_GNU_window_save\n");
	      break;

	    case DW_CFA_GNU_args_size:
	      ul = LEB ();
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_GNU_args_size: %ld\n", ul);
	      break;

	    case DW_CFA_GNU_negative_offset_extended:
	      reg = LEB ();
	      l = - LEB ();
	      frame_need_space (fc, reg);
	      if (! do_debug_frames_interp)
		printf ("  DW_CFA_GNU_negative_offset_extended: r%ld at cfa%+ld\n",
			reg, l * fc->data_factor);
	      fc->col_type[reg] = DW_CFA_offset;
	      fc->col_offset[reg] = l * fc->data_factor;
	      break;

	    default:
	      warn (_("unsupported or unknown DW_CFA_%d\n"), op);
	      start = block_end;
	    }
	}

      if (do_debug_frames_interp)
	frame_display_row (fc, &need_col_headers, &max_regs);

      start = block_end;
    }

  printf ("\n");

  return 1;
}

#undef GET
#undef LEB
#undef SLEB

static int
display_debug_not_supported (Elf_Internal_Shdr *section,
			     unsigned char *start ATTRIBUTE_UNUSED,
			     FILE *file ATTRIBUTE_UNUSED)
{
  printf (_("Displaying the debug contents of section %s is not yet supported.\n"),
	    SECTION_NAME (section));

  return 1;
}

/* A structure containing the name of a debug section
   and a pointer to a function that can decode it.  */
static struct {
  const char *const name;
  int (*display)(Elf_Internal_Shdr *, unsigned char *, FILE *);
} debug_displays[] = {
  { ".debug_abbrev",		display_debug_abbrev },
  { ".debug_aranges",		display_debug_aranges },
  { ".debug_frame",		display_debug_frames },
  { ".debug_info",		display_debug_info },
  { ".debug_line",		display_debug_lines },
  { ".debug_pubnames",		display_debug_pubnames },
  { ".eh_frame",		display_debug_frames },
  { ".debug_macinfo",		display_debug_macinfo },
  { ".debug_str",		display_debug_str },
  { ".debug_loc",		display_debug_loc },
  { ".debug_pubtypes",		display_debug_pubnames },
  { ".debug_ranges",		display_debug_ranges },
  { ".debug_static_func",	display_debug_not_supported },
  { ".debug_static_vars",	display_debug_not_supported },
  { ".debug_types",		display_debug_not_supported },
  { ".debug_weaknames",		display_debug_not_supported }
};

static int
display_debug_section(Elf_Internal_Shdr *section, FILE *file)
{
  const char *name = SECTION_NAME(section);
  bfd_size_type length;
  int result = 1;
  int i;

  length = section->sh_size;
  if (length == 0)
    {
      printf(_("\nSection '%s' has no debugging data.\n"), name);
      return 0;
    }

  if (strneq(name, ".gnu.linkonce.wi.", 17))
    name = ".debug_info";

  /* See if we know how to display the contents of this section: */
  for (i = NUM_ELEM(debug_displays); i--;)
    if (streq(debug_displays[i].name, name))
      {
	unsigned char *start;

	start = ((unsigned char *)
                 get_data(NULL, file, (long)section->sh_offset, 1UL,
                          (size_t)length, _("debug section data")));
	if (start == NULL)
	  {
	    result = 0;
	    break;
	  }

	result &= debug_displays[i].display(section, start, file);
	free(start);

	/* If we loaded in the abbrev section
	   at some point, we must release it here.  */
	free_abbrevs();

	break;
      }

  if (i == -1)
    {
      printf(_("Unrecognized debug section: %s\n"), name);
      result = 0;
    }

  return result;
}

static void
process_section_contents(FILE *file)
{
  Elf_Internal_Shdr *section;
  unsigned int i;

  if (! do_dump)
    return;

  for (i = 0, section = section_headers;
       (i < elf_header.e_shnum) && (i < num_dump_sects);
       i++, section++)
    {
#ifdef SUPPORT_DISASSEMBLY
      if (dump_sects[i] & DISASS_DUMP)
	disassemble_section (section, file);
#endif /* SUPPORT_DISASSEMBLY */
      if (dump_sects[i] & HEX_DUMP)
	dump_section(section, file);

      if (dump_sects[i] & DEBUG_DUMP)
	display_debug_section(section, file);
    }

  /* Check to see if the user requested a
     dump of a section that does not exist.  */
  while (i++ < num_dump_sects)
    if (dump_sects[i])
      warn(_("Section %d was not dumped because it does not exist!\n"), i);
}

static void
process_mips_fpe_exception(int mask)
{
  if (mask)
    {
      int first = 1;
      if (mask & OEX_FPU_INEX) fputs("INEX", stdout), first = 0;
      if (mask & OEX_FPU_UFLO)
        printf("%sUFLO", (first ? "" : "|")), first = 0;
      if (mask & OEX_FPU_OFLO)
	printf("%sOFLO", (first ? "" : "|")), first = 0;
      if (mask & OEX_FPU_DIV0)
	printf("%sDIV0", (first ? "" : "|")), first = 0;
      if (mask & OEX_FPU_INVAL) printf("%sINVAL", first ? "" : "|");
    }
  else
    fputs("0", stdout);
}

static int
process_mips_specific(FILE *file)
{
  Elf_Internal_Dyn *entry;
  size_t liblist_offset = 0UL;
  size_t liblistno = 0UL;
  size_t conflictsno = 0UL;
  size_t options_offset = 0UL;
  size_t conflicts_offset = 0UL;

  /* We have a lot of special sections; thanks SGI!  */
  if (dynamic_section == NULL)
    /* No information available: */
    return 0;

  for (entry = dynamic_section; entry->d_tag != DT_NULL; ++entry)
    switch (entry->d_tag)
      {
      case DT_MIPS_LIBLIST:
	liblist_offset =
          ((size_t)
           offset_from_vma(file, entry->d_un.d_val,
                           (liblistno * sizeof(Elf32_External_Lib))));
	break;
      case DT_MIPS_LIBLISTNO: liblistno = (size_t)entry->d_un.d_val; break;
      case DT_MIPS_OPTIONS:
	options_offset =
          (size_t)offset_from_vma(file, entry->d_un.d_val, 0);
        break;
      case DT_MIPS_CONFLICT:
	conflicts_offset =
          ((size_t)
           offset_from_vma(file, entry->d_un.d_val,
                           (conflictsno
                            * sizeof(Elf32_External_Conflict))));
	break;
      case DT_MIPS_CONFLICTNO:
        conflictsno = (size_t)entry->d_un.d_val; break;
      default: break;
      }

  if ((liblist_offset != 0) && (liblistno != 0) && do_dynamic)
    {
      Elf32_External_Lib *elib;
      size_t cnt;

      elib = ((Elf32_External_Lib *)
              get_data(NULL, file, (long)liblist_offset, liblistno,
                       sizeof(Elf32_External_Lib), _("liblist")));
      if (elib)
	{
	  printf("\nSection '.liblist' contains %lu entries:\n",
		 (unsigned long)liblistno);
	  fputs("     Library              Time Stamp          Checksum   Version Flags\n",
                stdout);

	  for (cnt = 0; cnt < liblistno; ++cnt)
	    {
	      Elf32_Lib liblist;
	      time_t time;
	      char timebuf[66]; /* big enough for -Wformat-length */
	      struct tm *tmp;

	      liblist.l_name = (unsigned long)BYTE_GET(elib[cnt].l_name);
	      time = (time_t)BYTE_GET(elib[cnt].l_time_stamp);
	      liblist.l_checksum = (unsigned long)BYTE_GET(elib[cnt].l_checksum);
	      liblist.l_version = (unsigned long)BYTE_GET(elib[cnt].l_version);
	      liblist.l_flags = (unsigned long)BYTE_GET(elib[cnt].l_flags);

	      tmp = gmtime(&time);
	      snprintf(timebuf, sizeof(timebuf),
                       "%04u-%02u-%02uT%02u:%02u:%02u",
                       tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
                       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	      printf("%3lu: ", (unsigned long)cnt);
	      if (VALID_DYNAMIC_NAME(liblist.l_name))
		print_symbol(20, GET_DYNAMIC_NAME(liblist.l_name));
	      else
		printf("<corrupt: %9ld>", liblist.l_name);
	      printf(" %s %#10lx %-7ld", timebuf, liblist.l_checksum,
		     liblist.l_version);

	      if (liblist.l_flags == 0)
		puts(" NONE");
	      else
		{
		  static const struct {
		    const char *name;
		    int bit;
		  } l_flags_vals[] = {
		    { " EXACT_MATCH", LL_EXACT_MATCH },
		    { " IGNORE_INT_VER", LL_IGNORE_INT_VER },
		    { " REQUIRE_MINOR", LL_REQUIRE_MINOR },
		    { " EXPORTS", LL_EXPORTS },
		    { " DELAY_LOAD", LL_DELAY_LOAD },
		    { " DELTA", LL_DELTA }
		  };
		  int flags = (int)liblist.l_flags;
		  size_t fcnt;

		  for (fcnt = 0;
		       fcnt < sizeof(l_flags_vals) / sizeof(l_flags_vals[0]);
		       ++fcnt)
		    if ((flags & l_flags_vals[fcnt].bit) != 0)
		      {
			fputs(l_flags_vals[fcnt].name, stdout);
			flags ^= l_flags_vals[fcnt].bit;
		      }
		  if (flags != 0)
		    printf(" %#x", (unsigned int)flags);

		  puts("");
		}
	    }

	  free(elib);
	}
    }

  if (options_offset != 0)
    {
      Elf_External_Options *eopt;
      Elf_Internal_Shdr *sect = section_headers;
      Elf_Internal_Options *iopt;
      Elf_Internal_Options *option = NULL;
      size_t offset;
      int cnt;

      /* Find the section header so that we get the size: */
      while (sect->sh_type != SHT_MIPS_OPTIONS) {
	++sect;
      }

      eopt = ((Elf_External_Options *)
              get_data(NULL, file, (long)options_offset, 1UL,
                       (size_t)sect->sh_size, _("options")));
      if (eopt)
	{
	  iopt = ((Elf_Internal_Options *)
                  cmalloc(((size_t)sect->sh_size / sizeof(eopt)),
                          sizeof(*iopt)));
	  if (iopt == NULL)
	    {
	      error(_("Out of memory"));
	      return 0;
	    }

          cnt = 0;
	  offset = (size_t)cnt;
	  option = iopt;

	  while (offset < sect->sh_size)
	    {
	      Elf_External_Options *eoption;

	      eoption = (Elf_External_Options *)((char *)eopt + offset);

	      option->kind = (unsigned char)BYTE_GET(eoption->kind);
	      option->size = (unsigned char)BYTE_GET(eoption->size);
	      option->section = (unsigned short)BYTE_GET(eoption->section);
	      option->info = (unsigned long)BYTE_GET(eoption->info);

	      offset += option->size;

	      ++option;
	      ++cnt;
	    }

	  printf(_("\nSection '%s' contains %d entries:\n"),
		 SECTION_NAME(sect), cnt);

	  option = iopt;

	  while (cnt-- > 0)
	    {
	      size_t len;

	      switch (option->kind)
		{
		case ODK_NULL:
		  /* This should NOT happen: */
		  printf(" NULL       %d %lx", option->section, option->info);
		  break;
		case ODK_REGINFO:
		  printf(" REGINFO    ");
		  if (elf_header.e_machine == EM_MIPS)
		    {
		      /* 32bit form.  */
		      Elf32_External_RegInfo *ereg;
		      Elf32_RegInfo reginfo;

		      ereg = (Elf32_External_RegInfo *)(option + 1);
		      reginfo.ri_gprmask = (unsigned long)BYTE_GET(ereg->ri_gprmask);
		      reginfo.ri_cprmask[0] = (unsigned long)BYTE_GET(ereg->ri_cprmask[0]);
		      reginfo.ri_cprmask[1] = (unsigned long)BYTE_GET(ereg->ri_cprmask[1]);
		      reginfo.ri_cprmask[2] = (unsigned long)BYTE_GET(ereg->ri_cprmask[2]);
		      reginfo.ri_cprmask[3] = (unsigned long)BYTE_GET(ereg->ri_cprmask[3]);
		      reginfo.ri_gp_value = (long)BYTE_GET(ereg->ri_gp_value);

		      printf("GPR %08lx  GP 0x%lx\n",
			     reginfo.ri_gprmask,
			     (unsigned long)reginfo.ri_gp_value);
		      printf("            CPR0 %08lx  CPR1 %08lx  CPR2 %08lx  CPR3 %08lx\n",
			     reginfo.ri_cprmask[0], reginfo.ri_cprmask[1],
			     reginfo.ri_cprmask[2], reginfo.ri_cprmask[3]);
		    }
		  else
		    {
		      /* 64 bit form.  */
		      Elf64_External_RegInfo *ereg;
		      Elf64_Internal_RegInfo reginfo;

		      ereg = (Elf64_External_RegInfo *)(option + 1);
		      reginfo.ri_gprmask = (unsigned long)BYTE_GET(ereg->ri_gprmask);
		      reginfo.ri_cprmask[0] = (unsigned long)BYTE_GET(ereg->ri_cprmask[0]);
		      reginfo.ri_cprmask[1] = (unsigned long)BYTE_GET(ereg->ri_cprmask[1]);
		      reginfo.ri_cprmask[2] = (unsigned long)BYTE_GET(ereg->ri_cprmask[2]);
		      reginfo.ri_cprmask[3] = (unsigned long)BYTE_GET(ereg->ri_cprmask[3]);
		      reginfo.ri_gp_value = BYTE_GET(ereg->ri_gp_value);

		      printf("GPR %08lx  GP 0x",
			     reginfo.ri_gprmask);
		      printf_vma(reginfo.ri_gp_value);
		      printf("\n");

		      printf("            CPR0 %08lx  CPR1 %08lx  CPR2 %08lx  CPR3 %08lx\n",
			     reginfo.ri_cprmask[0], reginfo.ri_cprmask[1],
			     reginfo.ri_cprmask[2], reginfo.ri_cprmask[3]);
		    }
		  ++option;
		  continue;
		case ODK_EXCEPTIONS:
		  fputs(" EXCEPTIONS fpe_min(", stdout);
		  process_mips_fpe_exception(option->info & OEX_FPU_MIN);
		  fputs(") fpe_max(", stdout);
		  process_mips_fpe_exception((int)((option->info & OEX_FPU_MAX) >> 8));
		  fputs(")", stdout);

		  if (option->info & OEX_PAGE0)
		    fputs(" PAGE0", stdout);
		  if (option->info & OEX_SMM)
		    fputs(" SMM", stdout);
		  if (option->info & OEX_FPDBUG)
		    fputs(" FPDBUG", stdout);
		  if (option->info & OEX_DISMISS)
		    fputs(" DISMISS", stdout);
		  break;
		case ODK_PAD:
		  fputs(" PAD       ", stdout);
		  if (option->info & OPAD_PREFIX)
		    fputs(" PREFIX", stdout);
		  if (option->info & OPAD_POSTFIX)
		    fputs(" POSTFIX", stdout);
		  if (option->info & OPAD_SYMBOL)
		    fputs(" SYMBOL", stdout);
		  break;
		case ODK_HWPATCH:
		  fputs(" HWPATCH   ", stdout);
		  if (option->info & OHW_R4KEOP)
		    fputs(" R4KEOP", stdout);
		  if (option->info & OHW_R8KPFETCH)
		    fputs(" R8KPFETCH", stdout);
		  if (option->info & OHW_R5KEOP)
		    fputs(" R5KEOP", stdout);
		  if (option->info & OHW_R5KCVTL)
		    fputs(" R5KCVTL", stdout);
		  break;
		case ODK_FILL:
		  fputs(" FILL       ", stdout);
		  /* XXX: Print content of info word?  */
		  break;
		case ODK_TAGS:
		  fputs(" TAGS       ", stdout);
		  /* XXX: Print content of info word?  */
		  break;
		case ODK_HWAND:
		  fputs(" HWAND     ", stdout);
		  if (option->info & OHWA0_R4KEOP_CHECKED)
		    fputs(" R4KEOP_CHECKED", stdout);
		  if (option->info & OHWA0_R4KEOP_CLEAN)
		    fputs(" R4KEOP_CLEAN", stdout);
		  break;
		case ODK_HWOR:
		  fputs(" HWOR      ", stdout);
		  if (option->info & OHWA0_R4KEOP_CHECKED)
		    fputs(" R4KEOP_CHECKED", stdout);
		  if (option->info & OHWA0_R4KEOP_CLEAN)
		    fputs(" R4KEOP_CLEAN", stdout);
		  break;
		case ODK_GP_GROUP:
		  printf(" GP_GROUP  %#06lx  self-contained %#06lx",
			 (option->info & OGP_GROUP),
			 ((option->info & OGP_SELF) >> 16));
		  break;
		case ODK_IDENT:
		  printf(" IDENT     %#06lx  self-contained %#06lx",
			 (option->info & OGP_GROUP),
			 ((option->info & OGP_SELF) >> 16));
		  break;
		default:
		  /* This should NOT happen: */
		  printf(" %3d ???     %d %lx",
			 option->kind, option->section, option->info);
		  break;
		}

	      len = sizeof(*eopt);
	      while ((len < option->size) && ((char *)option != NULL))
		if ((((char *)option)[len] >= ' ')
		    && (((char *)option)[len] < 0x7f))
		  printf("%c", ((char *)option)[len++]);
		else
		  printf("\\%03o", ((char *)option)[len++]);

	      fputs("\n", stdout);
	      ++option;
	    }

	  free(eopt);
	}
    }

  if ((conflicts_offset != 0) && (conflictsno != 0))
    {
      Elf32_Conflict *iconf;
      size_t cnt;

      if (dynamic_symbols == NULL)
	{
	  error(_("conflict list found without a dynamic symbol table"));
	  return 0;
	}

      iconf = (Elf32_Conflict *)cmalloc(conflictsno, sizeof(*iconf));
      if (iconf == (Elf32_Conflict *)NULL)
	{
	  error(_("Out of memory"));
	  return 0;
	}

      if (is_32bit_elf)
	{
	  Elf32_External_Conflict *econf32;

	  econf32 = ((Elf32_External_Conflict *)
                     get_data(NULL, file, (long)conflicts_offset,
                              conflictsno, sizeof(*econf32),
                              _("conflict")));
	  if (!econf32)
            {
              free(iconf);
	      return 0;
            }

	  for (cnt = 0; cnt < conflictsno; ++cnt)
	    iconf[cnt] = (Elf32_Conflict)BYTE_GET(econf32[cnt]);

	  free(econf32);
	}
      else
	{
	  Elf64_External_Conflict *econf64;

	  econf64 = ((Elf64_External_Conflict *)
                     get_data(NULL, file, (long)conflicts_offset,
                              conflictsno, sizeof(*econf64),
                              _("conflict")));
	  if (!econf64)
            {
              free(iconf);
	      return 0;
            }

	  for (cnt = 0; cnt < conflictsno; ++cnt)
	    iconf[cnt] = (Elf32_Conflict)BYTE_GET(econf64[cnt]);

	  free(econf64);
	}

      printf(_("\nSection '.conflict' contains %lu entries:\n"),
	     (unsigned long)conflictsno);
      puts(_("  Num:    Index       Value  Name"));

      for (cnt = 0; cnt < conflictsno; ++cnt)
	{
	  Elf_Internal_Sym *psym = &dynamic_symbols[iconf[cnt]];

	  printf("%5lu: %8lu  ", (unsigned long)cnt, iconf[cnt]);
	  print_vma(psym->st_value, FULL_HEX);
	  putchar(' ');
	  if (VALID_DYNAMIC_NAME(psym->st_name))
	    print_symbol(25, GET_DYNAMIC_NAME(psym->st_name));
	  else
	    printf("<corrupt: %14ld>", psym->st_name);
	  putchar('\n');
	}

      free(iconf);
    }

  return 1;
}

static int
process_gnu_liblist(FILE *file)
{
  Elf_Internal_Shdr *section, *string_sec;
  Elf32_External_Lib *elib;
  char *strtab;
  size_t strtab_size;
  size_t cnt;
  unsigned i;

  if (! do_arch) {
    return 0;
  }

  for (i = 0, section = section_headers;
       i < elf_header.e_shnum;
       i++, section++)
    {
      switch (section->sh_type)
	{
	case SHT_GNU_LIBLIST:
	  if (SECTION_HEADER_INDEX(section->sh_link) >= elf_header.e_shnum)
	    break;

	  elib = ((Elf32_External_Lib *)
                  get_data(NULL, file, (long)section->sh_offset, 1UL,
                           (size_t)section->sh_size, _("liblist")));

          if (elib == NULL) {
	    break;
          }
	  string_sec = SECTION_HEADER(section->sh_link);

	  strtab = (char *)get_data(NULL, file,
                                    (long)string_sec->sh_offset, 1UL,
                                    (size_t)string_sec->sh_size,
                                    _("liblist string table"));
	  strtab_size = (size_t)string_sec->sh_size;

	  if (strtab == NULL
	      || section->sh_entsize != sizeof(Elf32_External_Lib))
	    {
	      free(elib);
	      break;
	    }

	  printf(_("\nLibrary list section '%s' contains %lu entries:\n"),
		 SECTION_NAME(section),
		 (long)(section->sh_size / sizeof (Elf32_External_Lib)));

	  puts("     Library              Time Stamp          Checksum   Version Flags");

	  for (cnt = 0; cnt < (section->sh_size / sizeof(Elf32_External_Lib));
	       ++cnt)
	    {
	      Elf32_Lib liblist;
	      time_t time;
	      char timebuf[66]; /* big enough for -Wformat-length */
	      struct tm *tmp;

	      liblist.l_name = (unsigned long)BYTE_GET(elib[cnt].l_name);
	      time = (time_t)BYTE_GET(elib[cnt].l_time_stamp);
	      liblist.l_checksum = (unsigned long)BYTE_GET(elib[cnt].l_checksum);
	      liblist.l_version = (unsigned long)BYTE_GET(elib[cnt].l_version);
	      liblist.l_flags = (unsigned long)BYTE_GET(elib[cnt].l_flags);

	      tmp = gmtime(&time);
	      snprintf(timebuf, sizeof(timebuf),
                       "%04u-%02u-%02uT%02u:%02u:%02u",
                       tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
                       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	      printf("%3lu: ", (unsigned long)cnt);
	      if (do_wide)
		printf("%-20s", ((liblist.l_name < strtab_size)
                                 ? strtab + liblist.l_name : "<corrupt>"));
	      else
		printf("%-20.20s", ((liblist.l_name < strtab_size)
                                    ? (strtab + liblist.l_name)
                                    : "<corrupt>"));
	      printf(" %s %#010lx %-7ld %-7ld\n", timebuf,
                     liblist.l_checksum, liblist.l_version, liblist.l_flags);
	    }

	  free(elib);
	  ATTRIBUTE_FALLTHROUGH; /* ...I guess? */
        default:;
	}
    }

  return 1;
}

static const char *
get_note_type(unsigned e_type)
{
  static char buff[64];

  if (elf_header.e_type == ET_CORE)
    switch (e_type)
      {
      case NT_AUXV: return _("NT_AUXV (auxiliary vector)");
      case NT_PRSTATUS: return _("NT_PRSTATUS (prstatus structure)");
      case NT_FPREGSET: return _("NT_FPREGSET (floating point registers)");
      case NT_PRPSINFO: return _("NT_PRPSINFO (prpsinfo structure)");
      case NT_TASKSTRUCT: return _("NT_TASKSTRUCT (task structure)");
      case NT_PRXFPREG: return _("NT_PRXFPREG (user_xfpregs structure)");
      case NT_PSTATUS: return _("NT_PSTATUS (pstatus structure)");
      case NT_FPREGS: return _("NT_FPREGS (floating point registers)");
      case NT_PSINFO: return _("NT_PSINFO (psinfo structure)");
      case NT_LWPSTATUS: return _("NT_LWPSTATUS (lwpstatus_t structure)");
      case NT_LWPSINFO: return _("NT_LWPSINFO (lwpsinfo_t structure)");
      case NT_WIN32PSTATUS: return _("NT_WIN32PSTATUS (win32_pstatus structure)");
      default: break;
      }
  else
    switch (e_type)
      {
      case NT_VERSION: return _("NT_VERSION (version)");
      case NT_ARCH: return _("NT_ARCH (architecture)");
      default: break;
      }

  snprintf(buff, sizeof(buff), _("Unknown note type: (0x%08x)"), e_type);
  return buff;
}

static const char *
get_netbsd_elfcore_note_type(unsigned e_type)
{
  static char buff[64];

  if (e_type == NT_NETBSDCORE_PROCINFO)
    {
      /* NetBSD core "procinfo" structure: */
      return _("NetBSD procinfo structure");
    }

  /* As of Jan 2002 there are no other machine-independent notes
     defined for NetBSD core files. If the note type is less
     than the start of the machine-dependent note types, we don't
     understand it.  */

  if (e_type < NT_NETBSDCORE_FIRSTMACH)
    {
      snprintf(buff, sizeof(buff), _("Unknown note type: (0x%08x)"), e_type);
      return buff;
    }

  switch (elf_header.e_machine)
    {
    /* On the Alpha, SPARC (32-bit and 64-bit), PT_GETREGS == mach+0
       and PT_GETFPREGS == mach+2.  */
    case EM_OLD_ALPHA:
    case EM_ALPHA:
    case EM_SPARC:
    case EM_SPARC32PLUS:
    case EM_SPARCV9:
      switch (e_type)
	{
	case NT_NETBSDCORE_FIRSTMACH+0: return _("PT_GETREGS (reg structure)");
	case NT_NETBSDCORE_FIRSTMACH+2: return _("PT_GETFPREGS (fpreg structure)");
	default: break;
	}
      break;

    /* On all other arch's, PT_GETREGS == mach+1 and
       PT_GETFPREGS == mach+3.  */
    default:
      switch (e_type)
	{
	case NT_NETBSDCORE_FIRSTMACH+1: return _("PT_GETREGS (reg structure)");
	case NT_NETBSDCORE_FIRSTMACH+3: return _("PT_GETFPREGS (fpreg structure)");
	default: break;
	}
    }

  snprintf(buff, sizeof(buff), _("PT_FIRSTMACH+%d"),
	   (e_type - NT_NETBSDCORE_FIRSTMACH));
  return buff;
}

/* Note that by the ELF standard, the name field is already null byte
   terminated, and namesz includes the terminating null byte.
   I.E. the value of namesz for the name "FSF" is 4.

   If the value of namesz is zero, there is no name present.  */
static int
process_note(Elf_Internal_Note *pnote)
{
  const char *nt;

  if (pnote->namesz == 0)
    /* If there is no note name, then use the default set of
     * note type strings: */
    nt = get_note_type(pnote->type);
  else if (strneq(pnote->namedata, "NetBSD-CORE", 11))
    /* NetBSD-specific core file notes: */
    nt = get_netbsd_elfcore_note_type(pnote->type);
  else
    /* Do NOT recognize this note name; just use the default set of
     * note type strings.  */
    nt = get_note_type(pnote->type);

  printf("  %s\t\t0x%08lx\t%s\n",
	 (pnote->namesz ? pnote->namedata : "(NONE)"),
	 pnote->descsz, nt);
  return 1;
}


static int
process_corefile_note_segment(FILE *file, bfd_vma offset, bfd_vma length)
{
  Elf_External_Note *pnotes;
  Elf_External_Note *external;
  int res = 1;

  if (length <= 0) {
    return 0;
  }

  pnotes = (Elf_External_Note *)get_data(NULL, file, (long)offset, 1UL,
                                         (size_t)length, _("notes"));
  if (!pnotes) {
    return 0;
  }

  external = pnotes;

  printf(_("\nNotes at offset 0x%08lx with length 0x%08lx:\n"),
         (unsigned long)offset, (unsigned long)length);
  printf(_("  Owner\t\tData size\tDescription\n"));

  while (external < (Elf_External_Note *)((char *) pnotes + length))
    {
      Elf_External_Note *next;
      Elf_Internal_Note inote;
      char *temp = NULL;

      inote.type = (unsigned long)BYTE_GET(external->type);
      inote.namesz = (unsigned long)BYTE_GET(external->namesz);
      inote.namedata = external->name;
      inote.descsz = (unsigned long)BYTE_GET(external->descsz);
      inote.descdata = (inote.namedata + align_power(inote.namesz, 2));
      inote.descpos = (offset + (bfd_vma)(inote.descdata - (char*)pnotes));

      next = (Elf_External_Note *)(inote.descdata + align_power(inote.descsz, 2));

      if (((char *)next) > (((char *)pnotes) + length))
	{
	  warn(_("corrupt note found at offset %lx into core notes\n"),
               (long)((char *)external - (char *)pnotes));
	  warn(_(" type: %lx, namesize: %08lx, descsize: %08lx\n"),
               inote.type, inote.namesz, inote.descsz);
	  break;
	}

      external = next;

      /* Verify that name is null terminated. It appears that at least
       * one version of Linux (RedHat 6.0) generates corefiles that do NOT
       * comply with the ELF spec by failing to include the null byte in
       * namesz.  */
      if (inote.namedata[inote.namesz] != '\0')
	{
	  temp = (char *)malloc(inote.namesz + 1UL);

	  if (temp == NULL)
	    {
	      error(_("Out of memory\n"));
	      res = 0;
	      break;
	    }

	  strncpy(temp, inote.namedata, inote.namesz);
	  temp[inote.namesz] = 0;

#ifdef DEBUG
	  warn(_("'%s' NOTE name not properly null terminated\n"), temp);
#endif /* DEBUG */
	  inote.namedata = temp;
	}

      res &= process_note(&inote);

      if (temp != NULL)
	{
	  free(temp);
	  temp = NULL;
	}
    }

  free(pnotes);

  return res;
}

static int
process_corefile_note_segments(FILE *file)
{
  Elf_Internal_Phdr *segment;
  unsigned int i;
  int res = 1;

  if (! get_program_headers(file))
      return 0;

  for (i = 0, segment = program_headers; i < elf_header.e_phnum;
       i++, segment++)
    {
      if (segment->p_type == PT_NOTE)
	res &= process_corefile_note_segment(file,
					     (bfd_vma)segment->p_offset,
					     (bfd_vma)segment->p_filesz);
    }

  return res;
}

static int
process_note_sections(FILE *file)
{
  Elf_Internal_Shdr *section;
  unsigned long i;
  int res = 1;

  for (i = 0, section = section_headers; i < elf_header.e_shnum;
       i++, section++)
    if (section->sh_type == SHT_NOTE)
      res &= process_corefile_note_segment(file,
                                           (bfd_vma)section->sh_offset,
					   (bfd_vma)section->sh_size);

  return res;
}

static int
process_notes(FILE *file)
{
  /* If we have not been asked to display the notes, then do nothing: */
  if (! do_notes)
    return 1;

  if (elf_header.e_type != ET_CORE)
    return process_note_sections(file);

  /* No program headers means no NOTE segment: */
  if (elf_header.e_phnum > 0)
    return process_corefile_note_segments(file);

  printf(_("No note segments present in the core file.\n"));
  return 1;
}

static int
process_arch_specific (FILE *file)
{
  if (! do_arch)
    return 1;

  switch (elf_header.e_machine)
    {
    case EM_MIPS:
    case EM_MIPS_RS3_LE: return process_mips_specific(file); break;
    default: break;
    }
  return 1;
}

static int
get_file_header(FILE *file)
{
  /* Read in the identity array: */
  if (fread(elf_header.e_ident, EI_NIDENT, 1, file) != 1)
    return 0;

  /* Determine how to read the rest of the header: */
  switch (elf_header.e_ident[EI_DATA])
    {
    default: /* fall through */
    case ELFDATANONE: /* fall through */
    case ELFDATA2LSB:
      byte_get = byte_get_little_endian;
      byte_put = byte_put_little_endian;
      break;
    case ELFDATA2MSB:
      byte_get = byte_get_big_endian;
      byte_put = byte_put_big_endian;
      break;
    }

  /* For now we only support 32 bit and 64 bit ELF files: */
  is_32bit_elf = (elf_header.e_ident[EI_CLASS] != ELFCLASS64);

  /* Read in the rest of the header: */
  if (is_32bit_elf)
    {
      Elf32_External_Ehdr ehdr32;

      if (fread(ehdr32.e_type, (sizeof(ehdr32) - EI_NIDENT), 1, file) != 1)
	return 0;

      elf_header.e_type = (unsigned short)BYTE_GET(ehdr32.e_type);
      elf_header.e_machine = (unsigned short)BYTE_GET(ehdr32.e_machine);
      elf_header.e_version = (unsigned long)BYTE_GET(ehdr32.e_version);
      elf_header.e_entry = BYTE_GET(ehdr32.e_entry); /* already bfd_vma */
      elf_header.e_phoff = (bfd_size_type)BYTE_GET(ehdr32.e_phoff);
      elf_header.e_shoff = (bfd_size_type)BYTE_GET(ehdr32.e_shoff);
      elf_header.e_flags = (unsigned long)BYTE_GET(ehdr32.e_flags);
      elf_header.e_ehsize = (unsigned int)BYTE_GET(ehdr32.e_ehsize);
      elf_header.e_phentsize = (unsigned int)BYTE_GET(ehdr32.e_phentsize);
      elf_header.e_phnum = (unsigned int)BYTE_GET(ehdr32.e_phnum);
      elf_header.e_shentsize = (unsigned int)BYTE_GET(ehdr32.e_shentsize);
      elf_header.e_shnum = (unsigned int)BYTE_GET(ehdr32.e_shnum);
      elf_header.e_shstrndx = (unsigned int)BYTE_GET(ehdr32.e_shstrndx);
    }
  else
    {
      Elf64_External_Ehdr ehdr64;

      /* If we have been compiled with sizeof (bfd_vma) == 4, then
	 we will not be able to cope with the 64bit data found in
	 64 ELF files. Detect this now and abort before we start
	 overwriting things.  */
      if (sizeof(bfd_vma) < 8UL)
	{
	  error(_("This instance of readelf has been built without support for a\n\
64 bit data type and so it cannot read 64 bit ELF files.\n"));
	  return 0;
	}

      if (fread(ehdr64.e_type, (sizeof(ehdr64) - EI_NIDENT), 1, file) != 1)
	return 0;

      elf_header.e_type = (unsigned short)BYTE_GET(ehdr64.e_type);
      elf_header.e_machine = (unsigned short)BYTE_GET(ehdr64.e_machine);
      elf_header.e_version = (unsigned long)BYTE_GET(ehdr64.e_version);
      elf_header.e_entry = BYTE_GET(ehdr64.e_entry); /* already bfd_vma */
      elf_header.e_phoff = (bfd_size_type)BYTE_GET(ehdr64.e_phoff);
      elf_header.e_shoff = (bfd_size_type)BYTE_GET(ehdr64.e_shoff);
      elf_header.e_flags = (unsigned long)BYTE_GET(ehdr64.e_flags);
      elf_header.e_ehsize = (unsigned int)BYTE_GET(ehdr64.e_ehsize);
      elf_header.e_phentsize = (unsigned int)BYTE_GET(ehdr64.e_phentsize);
      elf_header.e_phnum = (unsigned int)BYTE_GET(ehdr64.e_phnum);
      elf_header.e_shentsize = (unsigned int)BYTE_GET(ehdr64.e_shentsize);
      elf_header.e_shnum = (unsigned int)BYTE_GET(ehdr64.e_shnum);
      elf_header.e_shstrndx = (unsigned int)BYTE_GET(ehdr64.e_shstrndx);
    }

  if (elf_header.e_shoff)
    {
      /* There may be some extensions in the first section header. Do NOT
       * bomb if we cannot read it.  */
      if (is_32bit_elf)
	get_32bit_section_headers (file, 1);
      else
	get_64bit_section_headers (file, 1);
    }

  return 1;
}

/* Process one ELF object file according to the command line options.
 * This file may actually be stored in an archive.  The file is positioned
 * at the start of the ELF object: */
static int
process_object(char *file_name, FILE *file)
{
  unsigned int i;

  if (! get_file_header(file))
    {
      error(_("%s: Failed to read file header\n"), file_name);
      return 1;
    }

  /* Initialize per file variables: */
  for (i = NUM_ELEM(version_info); i--;) {
    version_info[i] = 0;
  }

  for (i = NUM_ELEM(dynamic_info); i--;) {
    dynamic_info[i] = 0;
  }

  /* Process the file: */
  if (show_name) {
    printf(_("\nFile: %s\n"), file_name);
  }

  /* Initialize the dump_sects array from the cmdline_dump_sects array.
   * Note we do this even if cmdline_dump_sects is empty because we must
   * make sure that the dump_sets array is zeroed out before each object
   * file is processed: */
  if (num_dump_sects > num_cmdline_dump_sects) {
    memset(dump_sects, 0, num_dump_sects);
  }

  if (num_cmdline_dump_sects > 0)
    {
      if (num_dump_sects == 0) {
	/* A sneaky way of allocating the dump_sects array: */
	request_dump (num_cmdline_dump_sects, 0);
      }

      assert(num_dump_sects >= num_cmdline_dump_sects);
      memcpy(dump_sects, cmdline_dump_sects, num_cmdline_dump_sects);
    }

  if (! process_file_header()) {
    return 1;
  }

  if (! process_section_headers(file))
    {
      /* Without loaded section headers, we cannot process lots of
       * things: */
      do_unwind = do_version = do_dump = do_arch = 0;

      if (! do_using_dynamic) {
	do_syms = do_reloc = 0;
      }
    }

  if (! process_section_groups(file))
    {
      /* Without loaded section groups, we cannot process unwind: */
      do_unwind = 0;
    }

  if (process_program_headers(file)) {
    process_dynamic_section(file);
  }

  process_relocs(file);

  process_unwind(file);

  process_symbol_table(file);

  process_syminfo(file);

  process_version_sections(file);

  process_section_contents(file);

  process_notes(file);

  process_gnu_liblist(file);

  process_arch_specific(file);

  if (program_headers)
    {
      free(program_headers);
      program_headers = NULL;
    }

  if (section_headers)
    {
      free(section_headers);
      section_headers = NULL;
    }

  if (string_table)
    {
      free(string_table);
      string_table = NULL;
      string_table_length = 0;
    }

  if (dynamic_strings)
    {
      free(dynamic_strings);
      dynamic_strings = NULL;
      dynamic_strings_length = 0;
    }

  if (dynamic_symbols)
    {
      free (dynamic_symbols);
      dynamic_symbols = NULL;
      num_dynamic_syms = 0;
    }

  if (dynamic_syminfo)
    {
      free(dynamic_syminfo);
      dynamic_syminfo = NULL;
    }

  if (section_headers_groups)
    {
      free(section_headers_groups);
      section_headers_groups = NULL;
    }

  if (section_groups)
    {
      struct group_list *g, *next;

      for (i = 0; i < group_count; i++)
	{
	  for (g = section_groups[i].root; g != NULL; g = next)
	    {
	      next = g->next;
	      free(g);
	    }
	}

      free(section_groups);
      section_groups = NULL;
    }

  if (debug_information)
    {
      for (i = 0; i < num_debug_info_entries; i++)
	{
	  if (!debug_information[i].max_loc_offsets)
	    {
	      free(debug_information[i].loc_offsets);
	      free(debug_information[i].have_frame_base);
	    }
	  if (!debug_information[i].max_range_lists)
	    free(debug_information[i].range_lists);
	}
      free(debug_information);
      debug_information = NULL;
      num_debug_info_entries = 0;
    }

  return 0;
}

/* Process an ELF archive. The file is positioned just after the
 * ARMAG string.  */
static int
process_archive(char *file_name, FILE *file)
{
  struct ar_hdr arhdr;
  size_t got;
  unsigned long size;
  char *longnames = NULL;
  unsigned long longnames_size = 0UL;
  size_t file_name_size;
  int ret;

  show_name = 1;

  got = fread(&arhdr, 1, sizeof(arhdr), file);
  if (got != sizeof(arhdr))
    {
      if (got == 0) {
	return 0;
      }

      error(_("%s: failed to read archive header\n"), file_name);
      return 1;
    }

  if (memcmp(arhdr.ar_name, "/               ", 16UL) == 0)
    {
      /* This is the archive symbol table. Skip it.
       * FIXME: We should have an option to dump it.  */
      size = strtoul(arhdr.ar_size, NULL, 10);
      if (fseek(file, (long)(size + (size & 1L)), SEEK_CUR) != 0)
	{
	  error(_("%s: failed to skip archive symbol table\n"), file_name);
	  return 1;
	}

      got = fread(&arhdr, 1UL, sizeof(arhdr), file);
      if (got != sizeof(arhdr))
	{
	  if (got == 0) {
	    return 0;
          }

	  error(_("%s: failed to read archive header\n"), file_name);
	  return 1;
	}
    }

  if (memcmp(arhdr.ar_name, "//              ", 16) == 0)
    {
      /* This is the archive string table holding long member names: */
      longnames_size = strtoul(arhdr.ar_size, NULL, 10);

      longnames = (char *)malloc(longnames_size);
      if (longnames == NULL)
	{
	  error(_("Out of memory\n"));
	  return 1;
	}

      if (fread(longnames, longnames_size, 1UL, file) != 1)
	{
	  free(longnames);
	  error(_("%s: failed to read string table\n"), file_name);
	  return 1;
	}

      if ((longnames_size & 1) != 0)
	getc(file);

      got = fread(&arhdr, 1UL, sizeof(arhdr), file);
      if (got != sizeof(arhdr))
	{
	  free(longnames);

	  if (got == 0)
	    return 0;

	  error(_("%s: failed to read archive header\n"), file_name);
	  return 1;
	}
    }

  file_name_size = strlen(file_name);
  ret = 0;

  while (1)
    {
      char *name;
      char *nameend;
      char *namealc;

      if (arhdr.ar_name[0] == '/')
	{
	  unsigned long off;

	  off = strtoul(arhdr.ar_name + 1, NULL, 10);
	  if (off >= longnames_size)
	    {
	      error(_("%s: invalid archive string table offset %lu\n"),
                    file_name, off);
	      ret = 1;
	      break;
	    }

	  name = (longnames + off);
	  nameend = (char *)memchr(name, '/', longnames_size - off);
	}
      else
	{
	  name = arhdr.ar_name;
	  nameend = (char *)memchr(name, '/', 16);
	}

      if (nameend == NULL)
	{
	  error(_("%s: bad archive file name\n"), file_name);
	  ret = 1;
	  break;
	}

      namealc = (char *)malloc(file_name_size + ((size_t)(nameend - name))
                               + 3UL);
      if (namealc == NULL)
	{
	  error(_("Out of memory\n"));
	  ret = 1;
	  break;
	}

      memcpy(namealc, file_name, file_name_size);
      namealc[file_name_size] = '(';
      memcpy(namealc + file_name_size + 1, name, (size_t)(nameend - name));
      namealc[file_name_size + 1UL + (size_t)(nameend - name)] = ')';
      namealc[file_name_size + 2UL + (size_t)(nameend - name)] = '\0';

      archive_file_offset = ftell(file);
      archive_file_size = strtoul(arhdr.ar_size, NULL, 10);

      ret |= process_object(namealc, file);

      free(namealc);

      if (fseek(file, (archive_file_offset + (long)archive_file_size
                       + ((long)archive_file_size & 1L)), SEEK_SET) != 0)
	{
	  error(_("%s: failed to seek to next archive header\n"),
                file_name);
	  ret = 1;
	  break;
	}

      got = fread(&arhdr, 1UL, sizeof(arhdr), file);
      if (got != sizeof(arhdr))
	{
	  if (got == 0) {
	    break;
          }

	  error(_("%s: failed to read archive header\n"), file_name);
	  ret = 1;
	  break;
	}
    }

  if (longnames != 0)
    free(longnames);

  return ret;
}

static int process_file(char *file_name)
{
  FILE *file;
  struct stat statbuf;
  char armag[SARMAG];
  int ret;

  if (stat(file_name, &statbuf) < 0)
    {
      if (errno == ENOENT)
	error(_("'%s': No such file\n"), file_name);
      else
	error(_("Could not locate '%s'.  System error message: %s\n"),
              file_name, xstrerror(errno));
      return 1;
    }

  if (! S_ISREG (statbuf.st_mode))
    {
      error(_("'%s' is not an ordinary file\n"), file_name);
      return 1;
    }

  file = fopen(file_name, "rb");
  if (file == NULL)
    {
      error(_("Input file '%s' is not readable.\n"), file_name);
      return 1;
    }

  if (fread(armag, SARMAG, 1, file) != 1)
    {
      error(_("%s: Failed to read file header\n"), file_name);
      fclose(file);
      return 1;
    }

  if (memcmp(armag, ARMAG, SARMAG) == 0) {
      ret = process_archive(file_name, file);
  } else {
      rewind(file);
      archive_file_offset = 0L;
      archive_file_size = (unsigned long)archive_file_offset;
      ret = process_object(file_name, file);
  }

  fclose(file);

  return ret;
}

#ifdef SUPPORT_DISASSEMBLY
/* Needed by the i386 disassembler. For extra credit, someone could
 * fix this so that we insert symbolic addresses here, esp. for GOT/PLT
 * symbols. */
void print_address(unsigned int addr, FILE *outfile)
{
  fprintf(outfile,"0x%8.8x", addr);
}

/* Needed by the i386 disassembler: */
void db_task_printsym(unsigned int addr)
{
  print_address (addr, stderr);
}
#endif /* SUPPORT_DISASSEMBLY */

int main(int argc, char **argv)
{
  int err;

#if defined(HAVE_SETLOCALE) && defined(HAVE_LC_MESSAGES)
  setlocale(LC_MESSAGES, "");
#endif /* HAVE_SETLOCALE && HAVE_LC_MESSAGES */
#if defined(HAVE_SETLOCALE)
  setlocale(LC_CTYPE, "");
#endif /* HAVE_SETLOCALE */
#if !defined(HAVE_BINDTEXTDOMAIN) && !defined(bindtextdomain)
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "bindtextdomain is needed."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* !HAVE_BINDTEXTDOMAIN && !bindtextdomain */
  bindtextdomain(PACKAGE, LOCALEDIR);
#if !defined(HAVE_TEXTDOMAIN) && !defined(textdomain)
# if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#  warning "textdomain is needed."
# endif /* __GNUC__ && !__STRICT_ANSI__ */
#endif /* !HAVE_TEXTDOMAIN && !textdomain */
  textdomain(PACKAGE);

  parse_args(argc, argv);

  if (num_dump_sects > 0) {
      /* Make a copy of the dump_sects array.  */
      cmdline_dump_sects = (char *)malloc(num_dump_sects);
      if (cmdline_dump_sects == NULL) {
	  error(_("Out of memory allocating dump request table."));
      } else {
	  memcpy(cmdline_dump_sects, dump_sects, num_dump_sects);
	  num_cmdline_dump_sects = num_dump_sects;
      }
  }

  if (optind < (argc - 1))
    show_name = 1;

  err = 0;
  while (optind < argc) {
      err |= process_file(argv[optind++]);
  }

  if (dump_sects != NULL) {
      free(dump_sects);
  }
  if (cmdline_dump_sects != NULL) {
      free(cmdline_dump_sects);
  }

  return err;
}

/* silence '-Wunused-macros': */
#ifdef BFD64
# undef BFD64
#endif /* BFD64 */
#ifdef DISASS_DUMP
# undef DISASS_DUMP
#endif /* DISASS_DUMP */

/* (this file is way too long) */
/* EOF */
