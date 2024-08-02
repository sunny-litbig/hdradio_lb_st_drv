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

lib_LTLIBRARIES = libtcradiohal.la \
				  libtcradiohalif.la

libtcradiohal_la_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/conf \
	-I$(TOP)/src/util \
	-I$(TOP)/src/audio \
	-I$(TOP)/src/fifo

libtcradiohal_la_SOURCES = \
	src/DMBLog.c \
	src/tcradio_hal.c \
	src/util/tcradio_hal_utils.c \
	src/util/tcradio_hal_mutex.c \
	src/util/tcradio_hal_rbuf.c \
	src/fifo/tcradio_hal_fifo.c

libtcradiohal_la_LIBADD =

if TUNER_IS_S0
libtcradiohal_la_LIBADD += \
	-lsi4796xdriver
endif
if TUNER_IS_X0
libtcradiohal_la_LIBADD += \
	-lx0tunerdriver
endif
if TUNER_IS_M0
libtcradiohal_la_LIBADD += \
	-lmax2175driver
endif
if TUNER_IS_T0
libtcradiohal_la_LIBADD += \
	-ltda770xdriver
endif
libtcradiohal_la_LIBADD += \
	-ltcradioperi

libtcradiohal_la_LIBADD += \
	-ldlt

libtcradiohal_la_LDFLAGS = \
	-version-number 2:0:0 \
	-shared

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

bin_PROGRAMS = TcRadioHalCui

TcRadioHalCui_CPPFLAGS = \
	-I$(TOP)/src

TcRadioHalCui_SOURCES = \
	src/cui/tcradio_hal_cui.c

TcRadioHalCui_LDADD = \
	$(top_builddir)/libtcradiohal.la \
	$(top_builddir)/libtcradiohalif.la

include_HEADERS = \
	$(TOP)/src/DMBLog.h \
	$(TOP)/src/tcradio_hal.h \
	$(TOP)/src/fifo/tcradio_hal_fifo.h \
	$(TOP)/src/conf/tcradio_hal_config.h

#defines for PROCESSOR
if IS_TCC897X
DEFS += -DUSE_TCC897X
endif
if IS_TCC802X
DEFS += -DUSE_TCC802X
endif
if IS_TCC803X
DEFS += -DUSE_TCC803X
endif
if IS_TCC805X
DEFS += -DUSE_TCC805X
endif
if IS_TCC807X
DEFS += -DUSE_TCC807X
endif

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
if BOARD_IS_TCC8059_MAIN
DEFS += -DTCC8059_MAIN_BOARD
endif
if BOARD_IS_TCC8059_SUB
DEFS += -DTCC8059_SUB_BOARD
endif
if BOARD_IS_TCC8053_MAIN
DEFS += -DTCC8053_MAIN_BOARD
endif
if BOARD_IS_TCC8053_SUB
DEFS += -DTCC8053_SUB_BOARD
endif
if BOARD_IS_TCC8050_MAIN
DEFS += -DTCC8050_MAIN_BOARD
endif
if BOARD_IS_TCC8050_SUB
DEFS += -DTCC8050_SUB_BOARD
endif
if BOARD_IS_TCC8070_MAIN
DEFS += -DTCC8070_MAIN_BOARD
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
if TUNER_IS_T0
DEFS += -DUSE_T0_TUNER
endif

#defines for hdradio
if RADIO_IS_HDRADIO
DEFS += -DUSE_HDRADIO
endif
