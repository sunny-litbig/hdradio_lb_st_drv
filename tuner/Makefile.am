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

lib_LTLIBRARIES = libx0tunerdriver.la \
				  libsi4796xdriver.la \
				  libmax2175driver.la \
				  libtda770xdriver.la

libx0tunerdriver_la_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/x0tuner

libx0tunerdriver_la_SOURCES = \
	src/x0tuner/x0tuner_core.c \
	src/x0tuner/x0tuner_hal.c \
	src/x0tuner/x0tuner_patch.c

libx0tunerdriver_la_LIBADD = \
	-ltcradioperi

libx0tunerdriver_la_LDFLAGS = \
	-version-number 2:0:0 \
	-shared

libsi4796xdriver_la_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/si479xx

libsi4796xdriver_la_SOURCES = \
	src/si479xx/si479xx_core.c \
	src/si479xx/si479xx_hal.c

libsi4796xdriver_la_LIBADD = \
	-ltcradioperi

libsi4796xdriver_la_LDFLAGS = \
	-version-number 2:0:0 \
	-shared

libtda770xdriver_la_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/star

libtda770xdriver_la_SOURCES = \
	src/star/star_gpio.c \
	src/star/star_i2c.c \
	src/star/star_protocol.c \
	src/star/star_driver.c

libtda770xdriver_la_LIBADD = \
	-ltcradioperi

libtda770xdriver_la_LDFLAGS = \
	-version-number 2:0:0 \
	-shared

libmax2175driver_la_CPPFLAGS = \
	-I$(TOP)/src \
	-I$(TOP)/src/max2175

libmax2175driver_la_SOURCES = \
	src/max2175/max2175_core.c \
	src/max2175/max2175_hal.c \
	src/max2175/max2175_regfunc.c \
	src/max2175/max2175_tables.c

libmax2175driver_la_LIBADD = \
	-ltcradioperi

libmax2175driver_la_LDFLAGS = \
	-version-number 2:0:0 \
	-shared

include_HEADERS = \
	$(TOP)/src/si479xx/si479xx_core.h \
	$(TOP)/src/x0tuner/x0tuner_core.h \
	$(TOP)/src/max2175/max2175_core.h \
	$(TOP)/src/star/star_public.h \
	$(TOP)/src/star/star_driver.h

addbindir = ${bindir}
addbin_DATA = \
	$(TOP)/src/si479xx/bin/si4795x_amfmwb_sw1p2_v3p0.bin \
	$(TOP)/src/si479xx/bin/si4795x_amfmdab_sw1p2_v3p0.bin \
	$(TOP)/src/si479xx/bin/si4796x_amfmwb_sw1p2_v3p0.bin \
	$(TOP)/src/si479xx/bin/si4796x_amfmdab_sw1p2_v3p0.bin \
	$(TOP)/src/si479xx/bin/si4795x_amfmdab_sw1p2p5p11_v4p1.bin \
	$(TOP)/src/si479xx/bin/si4796x_amfmdab_sw1p2p5p11_v4p1.bin \
	$(TOP)/src/si479xx/bin/si4795x_amfmwb_sw1p2p11_v4p1.bin \
	$(TOP)/src/si479xx/bin/si4796x_amfmwb_sw1p2p11_v4p1.bin \
	$(TOP)/src/si479xx/bin/si4795x_amfmdab_sw1p2p5p11_v5p2.bin \
	$(TOP)/src/si479xx/bin/si4796x_amfmdab_sw1p2p5p11_v5p2.bin \
	$(TOP)/src/si479xx/bin/si4795x_amfmwb_sw1p2p11_v5p2.bin \
	$(TOP)/src/si479xx/bin/si4796x_amfmwb_sw1p2p11_v5p2.bin

