pluginimpdir = $(pkglibdir)/plugins/import

pluginimp_LTLIBRARIES = imp_cob.la

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

imp_cob_la_SOURCES = imp_cob.c
imp_cob_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
