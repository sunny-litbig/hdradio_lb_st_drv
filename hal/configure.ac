#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tc-radio-hal], [2.0.0], [jkhwang@telechips.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/tcradio_hal.c])
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

AC_ARG_ENABLE([pulseaudio], AC_HELP_STRING([--enable-pulseaudio], [Enable PulseAudio Function]))
AS_IF([test "x$enable_pulseaudio" = "xyes"], [AUDIO_IS_PULSEAUDIO=1])
AM_CONDITIONAL([AUDIO_IS_PULSEAUDIO], [test "x$AUDIO_IS_PULSEAUDIO" = "x1"])

AC_ARG_ENABLE([hdradio], AC_HELP_STRING([--enable-hdradio], [Enable HDRadio Function]))
AS_IF([test "x$enable_hdradio" = "xyes"], [RADIO_IS_HDRADIO=1])
AM_CONDITIONAL([RADIO_IS_HDRADIO], [test "x$RADIO_IS_HDRADIO" = "x1"])

AC_ARG_ENABLE([s0tuner], AC_HELP_STRING([--enable-s0tuner], [Enable S0 Tuner Function]))
AS_IF([test "x$enable_s0tuner" = "xyes"], [TUNER_IS_S0=1])
AM_CONDITIONAL([TUNER_IS_S0], [test "x$TUNER_IS_S0" = "x1"])

AC_ARG_ENABLE([x0tuner], AC_HELP_STRING([--enable-x0tuner], [Enable X0 Tuner Function]))
AS_IF([test "x$enable_x0tuner" = "xyes"], [TUNER_IS_X0=1])
AM_CONDITIONAL([TUNER_IS_X0], [test "x$TUNER_IS_X0" = "x1"])

AC_ARG_ENABLE([m0tuner], AC_HELP_STRING([--enable-m0tuner], [Enable M0 Tuner Function]))
AS_IF([test "x$enable_m0tuner" = "xyes"], [TUNER_IS_M0=1])
AM_CONDITIONAL([TUNER_IS_M0], [test "x$TUNER_IS_M0" = "x1"])

AC_ARG_ENABLE([t0tuner], AC_HELP_STRING([--enable-t0tuner], [Enable T0 Tuner Function]))
AS_IF([test "x$enable_t0tuner" = "xyes"], [TUNER_IS_T0=1])
AM_CONDITIONAL([TUNER_IS_T0], [test "x$TUNER_IS_T0" = "x1"])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([asound], [main])

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile])

#telechips processor setup
AC_ARG_VAR(PROCESSOR, telechips processor setting (tcc807x, tcc805x, tcc803x, tcc802x, tcc892x, tcc8925s, tcc893x, tcc8935s, tcc896x, tcc897x, tcc898x or simulate))
AS_IF([test "x$PROCESSOR" = x], [PROCESSOR="tcc897x"])
AM_CONDITIONAL(IS_TCC807X, test "x$PROCESSOR" = "xtcc807x")
AM_CONDITIONAL(IS_TCC803X, test "x$PROCESSOR" = "xtcc803x")
AM_CONDITIONAL(IS_TCC805X, test "x$PROCESSOR" = "xtcc805x")
AM_CONDITIONAL(IS_TCC802X, test "x$PROCESSOR" = "xtcc802x")
AM_CONDITIONAL(IS_TCC898X, test "x$PROCESSOR" = "xtcc898x")
AM_CONDITIONAL(IS_TCC897X, test "x$PROCESSOR" = "xtcc897x")
AM_CONDITIONAL(IS_TCC896X, test "x$PROCESSOR" = "xtcc896x")
AM_CONDITIONAL(IS_TCC893X, test "x$PROCESSOR" = "xtcc893x")
AM_CONDITIONAL(IS_TCC8935S, test "x$PROCESSOR" = "xtcc8935s")
AM_CONDITIONAL(IS_TCC892X, test "x$PROCESSOR" = "xtcc892x")
AM_CONDITIONAL(IS_TCC8925S, test "x$PROCESSOR" = "xtcc8925s")

#telechips machine setup
AC_ARG_VAR(MACHINE, telechips machine setting (tcc8971-lcn-2.0, tcc8971-lcn-2.0a or tcc8021, tcc8030, tcc8031, tcc8059-main, tcc8059-sub, tcc8053-main, tcc8053-sub, tcc8050-main, tcc8050-sub, tcc8031p-main, tcc8034p-main, tcc8030-main, tcc8070-main))
AS_IF([test "x$MACHINE" = x], [MACHINE="tcc8971-lcn-2.0"])
AS_IF([test "x$MACHINE" = "xtcc8971-lcn-2.0a"], [MACHINE="tcc8971-lcn-2.0"])
AS_IF([test "x$MACHINE" = "xtcc8030-main"], [MACHINE="tcc8031p-main"])
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
AM_CONDITIONAL(BOARD_IS_TCC8031P_MAIN,  test "x$MACHINE" = "xtcc8031p-main")
AM_CONDITIONAL(BOARD_IS_TCC8034P_MAIN,  test "x$MACHINE" = "xtcc8034p-main")
AM_CONDITIONAL(BOARD_IS_TCC8070_MAIN,  test "x$MACHINE" = "xtcc8070-main")
AC_OUTPUT
