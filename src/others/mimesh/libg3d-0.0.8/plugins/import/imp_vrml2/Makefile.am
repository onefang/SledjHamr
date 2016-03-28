pluginimpdir = $(pkglibdir)/plugins/import

if EXPERIMENTAL
pluginimp_LTLIBRARIES = imp_vrml2.la
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

imp_vrml2_la_SOURCES = imp_vrml2.c imp_vrml2.h \
	imp_vrml_read.c imp_vrml_read.h \
	imp_vrml_types.h \
	imp_vrml2_types.c imp_vrml2_types.h \
	imp_vrml2_nodes.h
imp_vrml2_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
