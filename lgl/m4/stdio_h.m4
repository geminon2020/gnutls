# stdio_h.m4 serial 3
dnl Copyright (C) 2007 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_STDIO_H],
[
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  gl_ABSOLUTE_HEADER([stdio.h])
  ABSOLUTE_STDIO_H=\"$gl_cv_absolute_stdio_h\"
  AC_SUBST([ABSOLUTE_STDIO_H])
])

AC_DEFUN([gl_STDIO_MODULE_INDICATOR],
[
  dnl Use AC_REQUIRE here, so that the default settings are expanded once only.
  AC_REQUIRE([gl_STDIO_H_DEFAULTS])
  GNULIB_[]m4_translit([$1],[abcdefghijklmnopqrstuvwxyz./-],[ABCDEFGHIJKLMNOPQRSTUVWXYZ___])=1
])

AC_DEFUN([gl_STDIO_H_DEFAULTS],
[
  GNULIB_FPRINTF_POSIX=0;  AC_SUBST([GNULIB_FPRINTF_POSIX])
  GNULIB_PRINTF_POSIX=0;   AC_SUBST([GNULIB_PRINTF_POSIX])
  GNULIB_SNPRINTF=0;       AC_SUBST([GNULIB_SNPRINTF])
  GNULIB_SPRINTF_POSIX=0;  AC_SUBST([GNULIB_SPRINTF_POSIX])
  GNULIB_VFPRINTF_POSIX=0; AC_SUBST([GNULIB_VFPRINTF_POSIX])
  GNULIB_VPRINTF_POSIX=0;  AC_SUBST([GNULIB_VPRINTF_POSIX])
  GNULIB_VSNPRINTF=0;      AC_SUBST([GNULIB_VSNPRINTF])
  GNULIB_VSPRINTF_POSIX=0; AC_SUBST([GNULIB_VSPRINTF_POSIX])
  GNULIB_VASPRINTF=0;      AC_SUBST([GNULIB_VASPRINTF])
  GNULIB_FFLUSH=0;         AC_SUBST([GNULIB_FFLUSH])
  dnl Assume proper GNU behavior unless another module says otherwise.
  REPLACE_FPRINTF=0;       AC_SUBST([REPLACE_FPRINTF])
  REPLACE_VFPRINTF=0;      AC_SUBST([REPLACE_VFPRINTF])
  REPLACE_PRINTF=0;        AC_SUBST([REPLACE_PRINTF])
  REPLACE_VPRINTF=0;       AC_SUBST([REPLACE_VPRINTF])
  REPLACE_SNPRINTF=0;      AC_SUBST([REPLACE_SNPRINTF])
  HAVE_DECL_SNPRINTF=1;    AC_SUBST([HAVE_DECL_SNPRINTF])
  REPLACE_VSNPRINTF=0;     AC_SUBST([REPLACE_VSNPRINTF])
  HAVE_DECL_VSNPRINTF=1;   AC_SUBST([HAVE_DECL_VSNPRINTF])
  REPLACE_SPRINTF=0;       AC_SUBST([REPLACE_SPRINTF])
  REPLACE_VSPRINTF=0;      AC_SUBST([REPLACE_VSPRINTF])
  HAVE_VASPRINTF=1;        AC_SUBST([HAVE_VASPRINTF])
  REPLACE_VASPRINTF=0;     AC_SUBST([REPLACE_VASPRINTF])
  REPLACE_FFLUSH=0;        AC_SUBST([REPLACE_FFLUSH])
])
