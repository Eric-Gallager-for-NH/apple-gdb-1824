# strftime-fixes.m4 serial 1
dnl Copyright (C) 2017 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_STRFTIME],
[
  AC_REQUIRE([gl_HEADER_TIME_H_DEFAULTS])
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw*) REPLACE_STRFTIME=1 ;;
    *)      REPLACE_STRFTIME=0 ;;
  esac
])
