pluginimpdir = $(pkglibdir)/plugins/import

pluginimp_LTLIBRARIES = imp_lwo.la

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

imp_lwo_la_SOURCES = imp_lwo.c imp_lwo.h imp_lwo_chunks.h \
	imp_lwo_callbacks.c imp_lwo_callbacks.h
imp_lwo_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
