#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([hdradioDummy], [2.6.10], [jkhwang@telechips.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/hdradioDummy.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_LN_S

AC_ARG_ENABLE([aarch64], AC_HELP_STRING([--enable-aarch64], [Enable 64bit System]))
AS_IF([test "x$enable_aarch64" = "xyes"], [SYSTEM_IS_AARCH64=1])
AM_CONDITIONAL([SYSTEM_IS_AARCH64], [test "x$SYSTEM_IS_AARCH64" = "x1"])

AC_ARG_ENABLE([arch_soft], AC_HELP_STRING([--enable-arch-softfp], [Enable 32bit soft-fp System]))
AS_IF([test "x$enable_arch_softfp" = "xyes"], [SYSTEM_IS_ARCH_SOFTFP=1])
AM_CONDITIONAL([SYSTEM_IS_ARCH_SOFTFP], [test "x$SYSTEM_IS_ARCH_SOFTFP" = "x1"])

AC_ARG_ENABLE([arch_hard], AC_HELP_STRING([--enable-arch-hardfp], [Enable 32bit hard-fp System]))
AS_IF([test "x$enable_arch_hardfp" = "xyes"], [SYSTEM_IS_ARCH_HARDFP=1])
AM_CONDITIONAL([SYSTEM_IS_ARCH_HARDFP], [test "x$SYSTEM_IS_ARCH_HARDFP" = "x1"])

# Checks for libraries.
AC_CHECK_LIB([main])

# Checks for header files.
AC_CHECK_HEADERS([stdio.h string.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
