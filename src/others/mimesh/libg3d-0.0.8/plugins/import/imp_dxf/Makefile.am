pluginimpdir = $(pkglibdir)/plugins/import

pluginimp_LTLIBRARIES = \
	imp_dxf.la

LIBTOOL += --quiet

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-DG_LOG_DOMAIN=\"LibG3D\" \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@

INCLUDES = -I$(top_srcdir)/include

AM_LDFLAGS = \
	-no-undefined -module \
	@PLUGINS_LDFLAGS@

imp_dxf_la_SOURCES = imp_dxf.c imp_dxf.h \
	imp_dxf_types.h imp_dxf_def.h imp_dxf_vars.h \
	imp_dxf_entities.h imp_dxf_chunks.h \
	imp_dxf_callbacks.c imp_dxf_callbacks.h \
	imp_dxf_section.c imp_dxf_section.h \
	imp_dxf_color.c imp_dxf_color.h \
	imp_dxf_prop.c imp_dxf_prop.h
imp_dxf_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@

