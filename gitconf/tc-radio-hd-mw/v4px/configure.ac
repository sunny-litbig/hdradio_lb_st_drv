#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tc-radio-mw], [2.0.0], [jkhwang@telechips.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/tcradio_api.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LN_S

AC_ARG_ENABLE([aarch64], AC_HELP_STRING([--enable-aarch64], [Enable 64bit System]))
AS_IF([test "x$enable_aarch64" = "xyes"], [SYSTEM_IS_AARCH64=1])
AM_CONDITIONAL([SYSTEM_IS_AARCH64], [test "x$SYSTEM_IS_AARCH64" = "x1"])

AC_ARG_ENABLE([hdradio], AC_HELP_STRING([--enable-hdradio], [Enable HDRadio Function]))
AS_IF([test "x$enable_hdradio" = "xyes"], [RADIO_IS_HDRADIO=1])
AM_CONDITIONAL([RADIO_IS_HDRADIO], [test "x$RADIO_IS_HDRADIO" = "x1"])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([asound], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])

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
