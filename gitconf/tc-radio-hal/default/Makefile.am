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

lib_LTLIBRARIES = libtcradiohalif.la

libtcradiohalif_la_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/conf \
	-I$(TOP)/src/audio

libtcradiohalif_la_SOURCES = \
	src/conf/tcradio_hal_config.c \
	src/audio/dev_audio.c
	
libtcradiohalif_la_LDFLAGS = \
	-version-number 2:0:0 \
	-shared

include_HEADERS = \
	$(TOP)/src/tcradio_hal.h \
	$(TOP)/src/conf/tcradio_hal_config.h \
	$(TOP)/src/fifo/tcradio_hal_fifo.h

#defines for TARGET BOARD
if BOARD_IS_TCC897X_LCN20
DEFS += -DTCC897X_LCN20_BOARD
endif
if BOARD_IS_TCC897X_LCN30
DEFS += -DTCC897X_LCN30_BOARD
endif
if BOARD_IS_TCC802X
DEFS += -DTCC802X_BOARD
endif
if BOARD_IS_TCC802X_EVM20
DEFS += -DTCC802X_EVM21_BOARD
endif
if BOARD_IS_TCC802X_EVM21
DEFS += -DTCC802X_EVM21_BOARD
endif
if BOARD_IS_TCC802X_EVB21
DEFS += -DTCC802X_EVM21_BOARD
endif
if BOARD_IS_TCC8030
DEFS += -DTCC8030_BOARD
endif
if BOARD_IS_TCC8031
DEFS += -DTCC8031_BOARD
endif

#defines for audio
if AUDIO_IS_PULSEAUDIO
DEFS += -DUSE_PULSEAUDIO
endif

#defines for tuners
if TUNER_IS_S0
DEFS += -DUSE_S0_TUNER
endif
if TUNER_IS_X0
DEFS += -DUSE_X0_TUNER
endif
if TUNER_IS_M0
DEFS += -DUSE_M0_TUNER
endif

#defines for hdradio
if RADIO_IS_HDRADIO
DEFS += -DUSE_HDRADIO
endif

#defines for AARCH64
if SYSTEM_IS_AARCH64
TOP_LIB = $(TOP)/lib64
else
TOP_LIB = $(TOP)/lib
endif

bin_PROGRAMS = TcRadioHalCui

TcRadioHalCui_CPPFLAGS = \
	-I$(TOP)/src

TcRadioHalCui_SOURCES = \
	src/cui/tcradio_hal_cui.c

TcRadioHalCui_LDADD = \
	$(TOP_LIB)/libtcradiohal.so.*.*.* \
	$(top_builddir)/libtcradiohalif.la

addlibdir = ${libdir}
addlib_DATA  = \
		$(TOP_LIB)/libtcradiohal.so.*.*.*

install-data-hook:
		for word in $(addlib_DATA); do \
			filename=`basename $$word`; \
			soname=`echo $$filename | cut -d. -f1-3`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$soname; \
			libname=`echo $$filename | cut -d. -f1-2`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$libname; \
		done
