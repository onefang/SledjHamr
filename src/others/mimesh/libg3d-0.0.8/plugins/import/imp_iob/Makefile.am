pluginimpdir = $(pkglibdir)/plugins/import

pluginimp_LTLIBRARIES = imp_iob.la

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

imp_iob_la_SOURCES = imp_iob.c imp_iob.h imp_iob_chunks.h \
	imp_iob_callbacks.c imp_iob_callbacks.h
imp_iob_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
