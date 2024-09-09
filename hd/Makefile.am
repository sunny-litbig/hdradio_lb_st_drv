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
CORE_HD_LIB = libHDRadio.so.*.*.*
##############################################
lib_LTLIBRARIES = libtchdradio.la \
				  libtchdradioif.la

libtchdradio_la_CPPFLAGS = \
	-I$(TOP)/src/api \
	-I$(TOP)/src/mw \
	-I$(TOP)/src/cmdproc \
	-I$(TOP)/src/support \
	-I$(TOP)/src/framework \
	-I$(TOP)/src/core

libtchdradio_la_SOURCES = \
	src/DMBLog.c \
	src/mw/tchdr_common.c \
	src/mw/tchdr_callback_conf.c \
	src/mw/tchdr_msg.c \
	src/mw/tchdr_service.c \
	src/mw/tchdr_debug.c \
	src/framework/tchdr_audio.c \
	src/framework/tchdr_bbinput.c \
	src/framework/tchdr_hdlibcb.c \
	src/framework/tchdr_framework.c \
	src/api/tchdr_aas.c \
	src/api/tchdr_alert.c \
	src/api/tchdr_psd.c \
	src/api/tchdr_sig.c \
	src/api/tchdr_sis.c \
	src/api/tchdr_api.c \
	src/cmdproc/tchdr_cmdcallbacks.c \
	src/cmdproc/tchdr_cmdaas.c \
	src/cmdproc/tchdr_cmdiboc.c \
	src/cmdproc/tchdr_cmdproc.c \
	src/cmdproc/tchdr_cmdsispsd.c \
	src/cmdproc/tchdr_cmdsys.c \
	src/cmdproc/tchdr_cmdtune.c \
	src/support/tchdr_bytestream.c \
	src/support/tchdr_cbuffer.c \
	src/support/tchdr_ethernet.c \
	src/support/tchdr_fader.c \
	src/support/tchdr_std.c

libtchdradio_la_LDFLAGS = \
	-version-number 1:6:1 \
	-shared \
	-fPIC

libtchdradioif_la_CPPFLAGS = \
	-I$(TOP)/src/api

libtchdradioif_la_SOURCES = \
	src/api/tchdr_callback.c

libtchdradioif_la_LDFLAGS = \
	-version-number 1:0:2 \
	-shared \
	-fPIC

include_HEADERS = \
	$(TOP)/src/DMBLog.h \
	$(TOP)/src/api/tchdr_types.h \
	$(TOP)/src/api/tchdr_callback.h \
	$(TOP)/src/api/tchdr_psd.h \
	$(TOP)/src/api/tchdr_sis.h \
	$(TOP)/src/api/tchdr_aas.h \
	$(TOP)/src/api/tchdr_sig.h \
	$(TOP)/src/api/tchdr_alert.h \
	$(TOP)/src/api/tchdr_api.h

#defines for TARGET BOARD
if BOARD_IS_TCC897X_LCN20
SEL_LIB_DIR = v2px
endif
if BOARD_IS_TCC897X_LCN30
SEL_LIB_DIR = v2px
endif
if BOARD_IS_TCC802X
SEL_LIB_DIR = v2px
endif
if BOARD_IS_TCC802X_EVM20
SEL_LIB_DIR = v2px
endif
if BOARD_IS_TCC802X_EVM21
SEL_LIB_DIR = v2px
endif
if BOARD_IS_TCC802X_EVB21
SEL_LIB_DIR = v2px
endif
if BOARD_IS_TCC8030
SEL_LIB_DIR = v3px
endif
if BOARD_IS_TCC8031
SEL_LIB_DIR = v3px
endif
if BOARD_IS_TCC8059_MAIN
SEL_LIB_DIR = v4px/a72
endif
if BOARD_IS_TCC8059_SUB
SEL_LIB_DIR = v4px/a53
endif
if BOARD_IS_TCC8053_MAIN
SEL_LIB_DIR = v4px/a72
endif
if BOARD_IS_TCC8053_SUB
SEL_LIB_DIR = v4px/a53
endif
if BOARD_IS_TCC8050_MAIN
SEL_LIB_DIR = v4px/a72
endif
if BOARD_IS_TCC8050_SUB
SEL_LIB_DIR = v4px/a53
endif
if BOARD_IS_TCC8031P_MAIN
SEL_LIB_DIR = v4px/a53
endif
if BOARD_IS_TCC8034P_MAIN
SEL_LIB_DIR = v4px/a53
endif
if BOARD_IS_TCC8070_MAIN
SEL_LIB_DIR = v4px/a72
endif

#define for AARCH64
if SYSTEM_IS_AARCH64
HDRADIO_LIB = $(TOP)/lib64/$(SEL_LIB_DIR)
else
if SYSTEM_IS_ARCH_HARDFP
HDRADIO_LIB = $(TOP)/lib/$(SEL_LIB_DIR)
else
HDRADIO_LIB = $(TOP)/lib/$(SEL_LIB_DIR)
endif
endif

libtchdradio_la_LIBADD = \
	-L$(HDRADIO_LIB) -ltcresampler \
	-L$(HDRADIO_LIB) -ltcprotection

libtchdradio_la_LIBADD += \
	-ldlt

if TARGET_IS_TC_EVB
DEFS += -DUSE_TELECHIPS_EVB
if BUILD_EXAMPLE_CUI
DEFS += -DBUILD_HDR_EXAMPLE_CUI
bin_PROGRAMS = TcHdrCui

TcHdrCui_CFLAGS = \
	-I$(TOP)/src/api \
	-I$(TOP)/src/example

TcHdrCui_SOURCES = \
	src/example/tchdr_cui.c \
	src/example/tchdr_cui_if.c \
	src/example/tchdr_cui_audio.c

TcHdrCui_LDADD = \
	$(top_builddir)/libtchdradio.la \
	$(top_builddir)/libtchdradioif.la \
	-L$(HDRADIO_LIB) -lHDRadio \
	-ltcradiohal \
	-ltcradiohalif \
	-ltcradioperi
endif
endif

addlibdir = ${libdir}
addlib_DATA = \
	$(HDRADIO_LIB)/$(CORE_HD_LIB)

install-data-hook:
		for word in $(addlib_DATA); do \
			filename=`basename $$word`; \
			soname=`echo $$filename | cut -d. -f1-3`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$soname; \
			libname=`echo $$filename | cut -d. -f1-2`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$libname; \
		done
