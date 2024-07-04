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
#telechips processor setup
AC_ARG_VAR(PROCESSOR, telechips processor setting (tcc807x, tcc805x or simulate))
AS_IF([test "x$PROCESSOR" = x], [PROCESSOR="tcc805x"])
AM_CONDITIONAL(IS_TCC805X, test "x$PROCESSOR" = "xtcc805x")
AM_CONDITIONAL(IS_TCC807X, test "x$PROCESSOR" = "xtcc807x")

#telechips machine setup
AC_ARG_VAR(MACHINE, telechips machine setting (tcc8059-main, tcc8059-sub, tcc8053-main, tcc8053-sub, tcc8050-main, tcc8050-sub, tcc8031p-main, tcc8034p-main, tcc8030-main, tcc8070-main))
AS_IF([test "x$MACHINE" = x], [MACHINE="tcc8059-sub"])
AS_IF([test "x$MACHINE" = "xtcc8030-main"], [MACHINE="tcc8031p-main"])
AM_CONDITIONAL(BOARD_IS_TCC8059_MAIN,  test "x$MACHINE" = "xtcc8059-main")
AM_CONDITIONAL(BOARD_IS_TCC8059_SUB,   test "x$MACHINE" = "xtcc8059-sub")
AM_CONDITIONAL(BOARD_IS_TCC8053_MAIN,  test "x$MACHINE" = "xtcc8053-main")
AM_CONDITIONAL(BOARD_IS_TCC8053_SUB,   test "x$MACHINE" = "xtcc8053-sub")
AM_CONDITIONAL(BOARD_IS_TCC8050_MAIN,  test "x$MACHINE" = "xtcc8050-main")
AM_CONDITIONAL(BOARD_IS_TCC8050_SUB,   test "x$MACHINE" = "xtcc8050-sub")
AM_CONDITIONAL(BOARD_IS_TCC8031P_MAIN,  test "x$MACHINE" = "xtcc8031p-main")
AM_CONDITIONAL(BOARD_IS_TCC8034P_MAIN,  test "x$MACHINE" = "xtcc8034p-main")
AM_CONDITIONAL(BOARD_IS_TCC8070_MAIN,  test "x$MACHINE" = "xtcc8070-main")
AC_OUTPUT
