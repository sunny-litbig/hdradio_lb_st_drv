##############################################
AUTOMAKE_OPTIONS = foreign
TOP=$(top_srcdir)
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

addlibdir = ${libdir}
addlib_DATA  = \
		$(TOP_LIB)/libtda770xdriver.so.*.*.*

install-data-hook:
		for word in $(addlib_DATA); do \
			filename=`basename $$word`; \
			soname=`echo $$filename | cut -d. -f1-3`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$soname; \
			libname=`echo $$filename | cut -d. -f1-2`; \
			$(LN_S)f $$filename $(DESTLIB_PATH)/$$libname; \
		done
