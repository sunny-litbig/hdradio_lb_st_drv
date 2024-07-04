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

#defines for AARCH64
if SYSTEM_IS_AARCH64
TOP_LIB = $(TOP)/lib64
else
TOP_LIB = $(TOP)/lib
endif

bin_PROGRAMS = TcRadioCui

TcRadioCui_CPPFLAGS = \
		-I$(TOP)/src

TcRadioCui_SOURCES = \
		src/cui/tcradio_cui.c

TcRadioCui_LDADD = \
		$(TOP_LIB)/libtcradio.so.*.*.*

include_HEADERS = \
		src/tcradio_api.h \
		src/tcradio_rds_api.h \
		src/tcradio_hdr_if.h

#defines for hdradio
if RADIO_IS_HDRADIO
DEFS += -DUSE_HDRADIO
endif

addlibdir = ${libdir}
addlib_DATA  = \
		$(TOP_LIB)/libtcradio.so.*.*.*

install-data-hook:
		for word in $(addlib_DATA); do \
			filename=`basename $$word`; \
			soname=`echo $$filename | cut -d. -f1-3`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$soname; \
			libname=`echo $$filename | cut -d. -f1-2`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$libname; \
		done
