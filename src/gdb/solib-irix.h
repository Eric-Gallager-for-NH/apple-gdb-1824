/* solib-irix.h: Handle shared libraries for GDB, the GNU Debugger.

   Copyright (C) 2007-2013 Free Software Foundation, Inc.

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

#ifndef SOLIB_IRIX_H
#define SOLIB_IRIX_H

struct target_so_ops;
extern struct target_so_ops irix_so_ops;

extern struct lm_info fetch_lm_info(CORE_ADDR);

#endif /* solib-irix.h */

/* EOF */
