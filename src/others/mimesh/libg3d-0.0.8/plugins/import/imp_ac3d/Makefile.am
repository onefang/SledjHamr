pluginimpdir = $(pkglibdir)/plugins/import

pluginimp_LTLIBRARIES = imp_ac3d.la

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

imp_ac3d_la_SOURCES = imp_ac3d.c
imp_ac3d_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
