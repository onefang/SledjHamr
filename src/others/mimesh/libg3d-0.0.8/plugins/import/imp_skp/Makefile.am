pluginimpdir = $(pkglibdir)/plugins/import

if EXPERIMENTAL
pluginimp_LTLIBRARIES = imp_skp.la
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

imp_skp_la_SOURCES = imp_skp.c imp_skp.h imp_skp_types.h \
	imp_skp_chunks.h imp_skp_callbacks.h imp_skp_callbacks.c \
	imp_skp_read.c imp_skp_read.h
imp_skp_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
