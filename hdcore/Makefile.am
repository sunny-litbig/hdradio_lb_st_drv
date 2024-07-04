##############################################
AUTOMAKE_OPTIONS = foreign
CC = @CC@
CPP = @CPP@
AM_CFLAGS = @CFLAGS@ $(TCAM_CFLAGS) -I$(top_srcdir)/include
AM_CPPFLAGS = $(AM_CFLAGS)
LIBS = @LIBS@ $(TCAM_LIBS)
DEFS = @DEFS@
##############################################
TOP = $(top_srcdir)
DESTLIB_PATH = $(DESTDIR)$(libdir)
##############################################
lib_LTLIBRARIES = libHDRadio.la

libHDRadio_la_CPPFLAGS = \
	-I$(TOP)/src/inc

libHDRadio_la_SOURCES = \
	src/hdradioDummy.c

libHDRadio_la_LDFLAGS = \
	-version-number 2:6:10 \
	-avoid-version \
	-shared

if SYSTEM_IS_AARCH64
DEFS += -DSYSTEM_IS_AARCH64
endif

