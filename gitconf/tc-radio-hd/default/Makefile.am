##############################################
AUTOMAKE_OPTIONS = foreign
CC = @CC@
CPP = @CPP@
AM_CFLAGS = @CFLAGS@ $(TCAM_CFLAGS) -I$(top_srcdir)/include
AM_CPPFLAGS = $(AM_CFLAGS)
LIBS = @LIBS@ $(TCAM_LIBS)
DEFS = @DEFS@
##############################################
TOP=$(top_srcdir)
DESTLIB_PATH = $(DESTDIR)$(libdir)
##############################################
lib_LTLIBRARIES = libtchdradioif.la

libtchdradioif_la_CPPFLAGS = \
		-I$(TOP)/src/api

libtchdradioif_la_SOURCES = \
    	src/api/tchdr_callback.c

libtchdradioif_la_LDFLAGS = \
	    -version-number 1:0:2 \
	    -shared \
	    -fPIC

include_HEADERS = \
	  	$(TOP)/src/api/tchdr_types.h \
		$(TOP)/src/api/tchdr_callback.h \
		$(TOP)/src/api/tchdr_psd.h \
		$(TOP)/src/api/tchdr_sis.h \
		$(TOP)/src/api/tchdr_aas.h \
		$(TOP)/src/api/tchdr_sig.h \
		$(TOP)/src/api/tchdr_alert.h \
		$(TOP)/src/api/tchdr_api.h


#defines for AARCH64
if SYSTEM_IS_AARCH64
TOP_LIB = $(TOP)/lib64
else
TOP_LIB = $(TOP)/lib
endif

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
	-L$(TOP_LIB) -ltchdradio \
	$(top_builddir)/libtchdradioif.la \
	-L$(TOP_LIB) -lHDRadio \
	-ltcradiohal \
	-ltcradiohalif \
	-ltcradioperi
endif
endif

addlibdir = ${libdir}
addlib_DATA  = \
		$(TOP_LIB)/libtchdradio.so.*.*.* \
		$(TOP_LIB)/libHDRadio.so.*.*.*

install-data-hook:
		for word in $(addlib_DATA); do \
			filename=`basename $$word`; \
			soname=`echo $$filename | cut -d. -f1-3`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$soname; \
			libname=`echo $$filename | cut -d. -f1-2`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$libname; \
		done
