#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tcradio-mw], [2.0.0], [jkhwang@telechips.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/tcradio_api.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL

AC_ARG_ENABLE([hdradio], AC_HELP_STRING([--enable-hdradio], [Enable HDRadio Function]))
AS_IF([test "x$enable_hdradio" = "xyes"], [RADIO_IS_HDRADIO=1])
AM_CONDITIONAL([RADIO_IS_HDRADIO], [test "x$RADIO_IS_HDRADIO" = "x1"])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([asound], [main])

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CONFIG_FILES([Makefile])

#telechips machine setup
AC_ARG_VAR(MACHINE, telechips machine setting (tcc8971-lcn-2.0, tcc8971-lcn-2.0a or tcc8021, tcc8030, tcc8031, tcc8059-main, tcc8059-sub, tcc8053-main, tcc8053-sub, tcc8050-main, tcc8050-sub, tcc8070-main))
AS_IF([test "x$MACHINE" = x], [MACHINE="tcc8971-lcn-2.0"])
AS_IF([test "x$MACHINE" = "xtcc8971-lcn-2.0a"], [MACHINE="tcc8971-lcn-2.0"])
AM_CONDITIONAL(BOARD_IS_TCC897X_LCN20, test "x$MACHINE" = "xtcc8971-lcn-2.0")
AM_CONDITIONAL(BOARD_IS_TCC897X_LCN30, test "x$MACHINE" = "xtcc8971-lcn-3.0")
AM_CONDITIONAL(BOARD_IS_TCC802X,       test "x$MACHINE" = "xtcc8021")
AM_CONDITIONAL(BOARD_IS_TCC802X_EVM20, test "x$MACHINE" = "xtcc8021-evm-2.0")
AM_CONDITIONAL(BOARD_IS_TCC802X_EVM21, test "x$MACHINE" = "xtcc8021-evm-2.1")
AM_CONDITIONAL(BOARD_IS_TCC802X_EVB21, test "x$MACHINE" = "xtcc8021-evb-2.1")
AM_CONDITIONAL(BOARD_IS_TCC8030,       test "x$MACHINE" = "xtcc8030")
AM_CONDITIONAL(BOARD_IS_TCC8031,       test "x$MACHINE" = "xtcc8031")
AM_CONDITIONAL(BOARD_IS_TCC8059_MAIN,  test "x$MACHINE" = "xtcc8059-main")
AM_CONDITIONAL(BOARD_IS_TCC8059_SUB,   test "x$MACHINE" = "xtcc8059-sub")
AM_CONDITIONAL(BOARD_IS_TCC8053_MAIN,  test "x$MACHINE" = "xtcc8053-main")
AM_CONDITIONAL(BOARD_IS_TCC8053_SUB,   test "x$MACHINE" = "xtcc8053-sub")
AM_CONDITIONAL(BOARD_IS_TCC8050_MAIN,  test "x$MACHINE" = "xtcc8050-main")
AM_CONDITIONAL(BOARD_IS_TCC8050_SUB,   test "x$MACHINE" = "xtcc8050-sub")
AM_CONDITIONAL(BOARD_IS_TCC8070_MAIN,  test "x$MACHINE" = "xtcc8070-main")
AC_OUTPUT
