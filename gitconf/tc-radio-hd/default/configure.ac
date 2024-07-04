#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tc-radio-hd], [1.6.0], [jkhwang@telechips.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/api/tchdr_api.h])
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

AC_ARG_ENABLE([tcevb], AC_HELP_STRING([--enable-tcevb], [Enable Telechips Evaluation Board]))
AS_IF([test "x$enable_tcevb" = "xyes"], [TARGET_IS_TC_EVB=1])
AM_CONDITIONAL([TARGET_IS_TC_EVB], [test "x$TARGET_IS_TC_EVB" = "x1"])

AC_ARG_ENABLE([hdrcui], AC_HELP_STRING([--enable-hdrcui], [Enable HD Radio example CUI build]))
AS_IF([test "x$enable_hdrcui" = "xyes"], [BUILD_EXAMPLE_CUI=1])
AM_CONDITIONAL([BUILD_EXAMPLE_CUI], [test "x$BUILD_EXAMPLE_CUI" = "x1"])

# Checks for libraries.
AC_CHECK_LIB([pthread], [main])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
