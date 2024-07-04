##############################################
AUTOMAKE_OPTIONS = foreign
TOP=$(top_srcdir)
DESTLIB_PATH = $(DESTDIR)$(libdir)
##############################################
#defines for AARCH64
if SYSTEM_IS_AARCH64
TOP_LIB = $(TOP)/lib64
else
TOP_LIB = $(TOP)/lib
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
