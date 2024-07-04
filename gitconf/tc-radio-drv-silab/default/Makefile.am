##############################################
AUTOMAKE_OPTIONS = foreign
TOP=$(top_srcdir)
DESTLIB_PATH = $(DESTDIR)$(libdir)
##############################################

addbindir = ${bindir}
addbin_DATA = \
		$(TOP)/bin/si4795x_amfmwb_sw1p2_v3p0.bin \
		$(TOP)/bin/si4796x_amfmwb_sw1p2_v3p0.bin \
		$(TOP)/bin/si4795x_amfmdab_sw1p2_v3p0.bin \
		$(TOP)/bin/si4796x_amfmdab_sw1p2_v3p0.bin \
		$(TOP)/bin/si4795x_amfmdab_sw1p2p5p11_v4p1.bin \
		$(TOP)/bin/si4796x_amfmdab_sw1p2p5p11_v4p1.bin \
		$(TOP)/bin/si4795x_amfmwb_sw1p2p11_v4p1.bin \
		$(TOP)/bin/si4796x_amfmwb_sw1p2p11_v4p1.bin

#defines for AARCH64
if SYSTEM_IS_AARCH64
TOP_LIB = $(TOP)/lib64
else
TOP_LIB = $(TOP)/lib
endif

addlibdir = ${libdir}
addlib_DATA  = \
		$(TOP_LIB)/libsi4796xdriver.so.*.*.*

install-data-hook:
		for word in $(addlib_DATA); do \
			filename=`basename $$word`; \
			soname=`echo $$filename | cut -d. -f1-3`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$soname; \
			libname=`echo $$filename | cut -d. -f1-2`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$libname; \
		done
