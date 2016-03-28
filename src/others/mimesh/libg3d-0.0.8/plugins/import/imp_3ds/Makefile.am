pluginimpdir = $(pkglibdir)/plugins/import

pluginimp_LTLIBRARIES = imp_3ds.la

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

imp_3ds_la_SOURCES = imp_3ds.c imp_3ds.h imp_3ds_chunks.h \
	imp_3ds_callbacks.c imp_3ds_callbacks.h
imp_3ds_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
