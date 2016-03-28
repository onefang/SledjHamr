pluginimpdir = $(pkglibdir)/plugins/import

if EXPERIMENTAL
pluginimp_LTLIBRARIES = imp_acf.la
endif

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-DG_LOG_DOMAIN=\"LibG3D\" \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@

INCLUDES = -I$(top_srcdir)/include

AM_LDFLAGS = \
	-no-undefined -module \
	@PLUGINS_LDFLAGS@

LIBTOOL += --quiet

imp_acf_la_SOURCES = imp_acf.c imp_acf.h \
	imp_acf_detect.c imp_acf_detect.h \
	imp_acf_airfoil.c imp_acf_airfoil.h \
	imp_acf_wing.c imp_acf_wing.h \
	imp_acf_def.c imp_acf_def.h imp_acf_def_proto.h \
	imp_acf_def_acf603.h imp_acf_def_acf625.h imp_acf_def_acf630.h \
	imp_acf_def_acf640.h imp_acf_def_acf651.h \
	imp_acf_def_acf700.h imp_acf_def_acf740.h
imp_acf_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
