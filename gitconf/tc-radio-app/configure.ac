#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tc-radio-app], [2.0.0], [jkhwang@telechips.com])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_ARG_ENABLE([aarch64], AC_HELP_STRING([--enable-aarch64], [Enable 64bit System]))
AS_IF([test "x$enable_aarch64" = "xyes"], [SYSTEM_IS_AARCH64=1])
AM_CONDITIONAL([SYSTEM_IS_AARCH64], [test "x$SYSTEM_IS_AARCH64" = "x1"])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
