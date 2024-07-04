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

#defines for MACHINE
if BOARD_IS_TCC8059_MAIN
SEL_CORE = a72
endif
if BOARD_IS_TCC8059_SUB
SEL_CORE = a53
endif
if BOARD_IS_TCC8053_MAIN
SEL_CORE = a72
endif
if BOARD_IS_TCC8053_SUB
SEL_CORE = a53
endif
if BOARD_IS_TCC8050_MAIN
SEL_CORE = a72
endif
if BOARD_IS_TCC8050_SUB
SEL_CORE = a53
endif
if BOARD_IS_TCC8031P_MAIN
SEL_CORE = a53
endif
if BOARD_IS_TCC8034P_MAIN
SEL_CORE = a53
endif
if BOARD_IS_TCC8070_MAIN
SEL_CORE = a72
endif

#defines for AARCH64
if SYSTEM_IS_AARCH64
TOP_LIB = $(TOP)/lib64/$(SEL_CORE)
else
TOP_LIB = $(TOP)/lib/$(SEL_CORE)
endif

bin_PROGRAMS = TcRadioCui

TcRadioCui_CPPFLAGS = \
		-I$(TOP)/src

TcRadioCui_SOURCES = \
		src/cui/tcradio_cui.c

TcRadioCui_LDADD = \
		$(TOP_LIB)/libtcradio.so.*.*.*

if BOARD_IS_TCC8059_SUB
bin_PROGRAMS += TcRadioSubManager
endif
if BOARD_IS_TCC8053_SUB
bin_PROGRAMS += TcRadioSubManager
endif
if BOARD_IS_TCC8050_SUB
bin_PROGRAMS += TcRadioSubManager
endif

TcRadioSubManager_CPPFLAGS = \
	    -I$(TOP)/src

TcRadioSubManager_SOURCES = \
	    src/submgr/tcradio_sub_manager.c \
     	src/submgr/dev_sub_ipc.c

TcRadioSubManager_LDADD = \
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
