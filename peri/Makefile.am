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

lib_LTLIBRARIES = libtcradioperi.la

libtcradioperi_la_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/gpio \
	-I$(TOP)/src/i2c \
	-I$(TOP)/src/i2s \
	-I$(TOP)/src/spi \
	-I$(TOP)/src/tuner \
	-I$(TOP)/src/conf

libtcradioperi_la_SOURCES = \
	src/gpio/dev_gpio.c \
	src/i2c/dev_i2c.c \
	src/i2s/dev_iq_i2s.c \
	src/i2s/dev_blend_audio_i2s.c \
	src/spi/dev_spi.c \
	src/conf/tcradio_peri_config.c

include_HEADERS = \
	$(TOP)/src/tcradio_types.h \
	$(TOP)/src/i2s/dev_iq_i2s.h \
	$(TOP)/src/i2s/dev_blend_audio_i2s.h \
	$(TOP)/src/conf/tcradio_peri_config.h \
	$(TOP)/src/tuner/tcradio_drv.h

libtcradioperi_la_LDFLAGS = \
	-version-number 2:0:0 \
	-shared

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
if BOARD_IS_TCC8031P_MAIN
DEFS += -DTCC8031P_MAIN_BOARD
endif
if BOARD_IS_TCC8034P_MAIN
DEFS += -DTCC8034P_MAIN_BOARD
endif
if BOARD_IS_TCC8070_MAIN
DEFS += -DTCC8070_MAIN_BOARD
endif
