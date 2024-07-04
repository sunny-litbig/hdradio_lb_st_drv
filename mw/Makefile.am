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
##############################################

lib_LTLIBRARIES = libtcradio.la

libtcradio_la_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/conf \
	-I$(TOP)/src/service \
	-I$(TOP)/src/sound \
	-I$(TOP)/src/rds \
	-I$(TOP)/src/util \
	-I$(TOP)/src/hd

libtcradio_la_SOURCES = \
	src/tcradio_api.c \
	src/conf/tcradio_callback.c \
	src/conf/tcradio_config.c \
	src/service/tcradio_service.c \
	src/hd/tcradio_hdr_if.c \
	src/sound/tcradio_sound.c \
	src/rds/tcradio_rds.c \
	src/rds/tcradio_rds_if.c \
	src/rds/tcradio_rds_sync.c \
	src/rds/tcradio_rds_parser.c \
	src/util/tcradio_memory.c \
	src/util/tcradio_msgq.c \
	src/util/tcradio_temp.c \
	src/util/tcradio_thread.c \
	src/util/tcradio_utils.c

libtcradio_la_LIBADD = \
	-ltcradiohal \
	-ltcradiohalif

if RADIO_IS_HDRADIO
libtcradio_la_LIBADD += \
	-ltchdradio \
	-ltchdradioif \
	-lHDRadio
endif

libtcradio_la_LDFLAGS = \
	-version-number 2:0:0 \
	-shared

bin_PROGRAMS = TcRadioCui

TcRadioCui_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/rds \
	-I$(TOP)/src/hd

TcRadioCui_SOURCES = \
	src/cui/tcradio_cui.c

TcRadioCui_LDADD = \
	$(top_builddir)/libtcradio.la

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
	-I$(TOP)/src \
	-I$(TOP)/src/rds \
	-I$(TOP)/src/hd

TcRadioSubManager_SOURCES = \
	src/submgr/tcradio_sub_manager.c \
	src/submgr/dev_sub_ipc.c

TcRadioSubManager_LDADD = \
	$(top_builddir)/libtcradio.la

include_HEADERS = \
	$(TOP)/src/tcradio_api.h \
	$(TOP)/src/rds/tcradio_rds_api.h \
	$(TOP)/src/hd/tcradio_hdr_if.h

#defines for hdradio
if RADIO_IS_HDRADIO
DEFS += -DUSE_HDRADIO
endif
