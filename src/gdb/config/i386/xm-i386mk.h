/* xm-i386mk.h
   Definitions to make GDB run on Mach 3 OSF 1/MK on an Intel 386
   Copyright 1992, 1993, 1998 Free Software Foundation, Inc.

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

#define HAVE_TERMIO	1

#define EMULATOR_BASE	0xa0000000
#define EMULATOR_END	0xa0040000

#include "i386/xm-i386m3.h"
