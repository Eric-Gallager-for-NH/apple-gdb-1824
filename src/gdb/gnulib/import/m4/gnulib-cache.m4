# Copyright (C) 2002-2016 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file.  If not, see <http://www.gnu.org/licenses/>.
#
# As a special exception to the GNU General Public License,
# this file may be distributed as part of a program that
# contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the specification of how gnulib-tool is used.
# It acts as a cache: It is written and read by gnulib-tool.
# In projects that use version control, this file is meant to be put under
# version control, like the configure.ac and various Makefile.am files.


# Specification in the form of a command-line invocation:
#   gnulib-tool --import --lib=libgnu --source-base=import --m4-base=import/m4 --doc-base=doc --tests-base=tests --aux-dir=import/extra --with-obsolete --avoid=lock --avoid=msvc-nothrow --avoid=threadlib --no-conditional-dependencies --no-libtool --macro-prefix=gl --no-vc-files absolute-header alignof alloca alloca-opt ansi-c++-opt assert assert-h assure autobuild bcopy bison-i18n btowc c-ctype c-strcase c-strcasestr chdir-long closedir configmake connect dirent dirent-safer dirfd dosname double-slash-root dup2 environ errno error exitfail extensions extern-inline fcntl fcntl-h fcntl-safer fileblocks flexmember float fnmatch fnmatch-gnu fpieee fpucw frexp frexpl fstatat fts getcwd getcwd-lgpl getpagesize gettext-h gettimeofday git-version-gen gitlog-to-changelog gnu-make gpl-2.0 havelib host-cpu-c-abi host-os ignore-value include_next inline intprops inttypes inttypes-incomplete isnand-nolibm isnanl-nolibm iswctype largefile ldd localcharset locale lstat malloc-gnu malloc-posix manywarnings math mbrtowc mbsinit mbsrtowcs memchr memcmp memmem memmem-simple mempcpy memrchr mkdtemp multiarch nextafter no-c++ nocrash obstack openmp pathmax pclose popen putenv readdir readlink realloc-gnu realloc-posix recv regex regex-quote regexprops-generic rmdir send sig2str sigaction signal signal-h sigpipe sigpipe-die sigprocmask snippet/_Noreturn snippet/arg-nonnull snippet/c++defs snippet/link-warning snippet/warn-on-use ssize_t stat stat-macros stat-size stat-time stdbool stddef stdint stdlib streq strerror strerror-override strerror_r-posix string strncat strndup strnlen strnlen1 strstr strstr-simple strtok_r sys_select sys_stat sys_time sys_types sys_wait tempname time time_r time_rz timegm unistd unistd-safer unlink unlink-busy update-copyright usleep vc-list-files verify warnings wchar wcsncasecmp wctype-h winsz-ioctl winsz-termios xalloc xalloc-die xalloc-oversized xmalloca xmemdup0 xstrndup

# Specification in the form of a few gnulib-tool.m4 macro invocations:
gl_LOCAL_DIR([])
gl_MODULES([
  absolute-header
  alignof
  alloca
  alloca-opt
  ansi-c++-opt
  assert
  assert-h
  assure
  autobuild
  bcopy
  bison-i18n
  btowc
  c-ctype
  c-strcase
  c-strcasestr
  chdir-long
  closedir
  configmake
  connect
  dirent
  dirent-safer
  dirfd
  dosname
  double-slash-root
  dup2
  environ
  errno
  error
  exitfail
  extensions
  extern-inline
  fcntl
  fcntl-h
  fcntl-safer
  fileblocks
  flexmember
  float
  fnmatch
  fnmatch-gnu
  fpieee
  fpucw
  frexp
  frexpl
  fstatat
  fts
  getcwd
  getcwd-lgpl
  getpagesize
  gettext-h
  gettimeofday
  git-version-gen
  gitlog-to-changelog
  gnu-make
  gpl-2.0
  havelib
  host-cpu-c-abi
  host-os
  ignore-value
  include_next
  inline
  intprops
  inttypes
  inttypes-incomplete
  isnand-nolibm
  isnanl-nolibm
  iswctype
  largefile
  ldd
  localcharset
  locale
  lstat
  malloc-gnu
  malloc-posix
  manywarnings
  math
  mbrtowc
  mbsinit
  mbsrtowcs
  memchr
  memcmp
  memmem
  memmem-simple
  mempcpy
  memrchr
  mkdtemp
  multiarch
  nextafter
  no-c++
  nocrash
  obstack
  openmp
  pathmax
  pclose
  popen
  putenv
  readdir
  readlink
  realloc-gnu
  realloc-posix
  recv
  regex
  regex-quote
  regexprops-generic
  rmdir
  send
  sig2str
  sigaction
  signal
  signal-h
  sigpipe
  sigpipe-die
  sigprocmask
  snippet/_Noreturn
  snippet/arg-nonnull
  snippet/c++defs
  snippet/link-warning
  snippet/warn-on-use
  ssize_t
  stat
  stat-macros
  stat-size
  stat-time
  stdbool
  stddef
  stdint
  stdlib
  streq
  strerror
  strerror-override
  strerror_r-posix
  string
  strncat
  strndup
  strnlen
  strnlen1
  strstr
  strstr-simple
  strtok_r
  sys_select
  sys_stat
  sys_time
  sys_types
  sys_wait
  tempname
  time
  time_r
  time_rz
  timegm
  unistd
  unistd-safer
  unlink
  unlink-busy
  update-copyright
  usleep
  vc-list-files
  verify
  warnings
  wchar
  wcsncasecmp
  wctype-h
  winsz-ioctl
  winsz-termios
  xalloc
  xalloc-die
  xalloc-oversized
  xmalloca
  xmemdup0
  xstrndup
])
gl_WITH_OBSOLETE
gl_AVOID([ lock msvc-nothrow threadlib])
gl_SOURCE_BASE([import])
gl_M4_BASE([import/m4])
gl_PO_BASE([])
gl_DOC_BASE([doc])
gl_TESTS_BASE([tests])
gl_LIB([libgnu])
gl_MAKEFILE_NAME([])
gl_MACRO_PREFIX([gl])
gl_PO_DOMAIN([])
gl_WITNESS_C_MACRO([])
gl_VC_FILES([false])
