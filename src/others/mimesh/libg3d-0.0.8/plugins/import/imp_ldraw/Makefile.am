pluginimpdir = $(pkglibdir)/plugins/import

pluginimp_LTLIBRARIES = imp_ldraw.la

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

imp_ldraw_la_SOURCES = imp_ldraw.c imp_ldraw_misc.h imp_ldraw_types.h \
	imp_ldraw_part.c imp_ldraw_part.h \
	imp_ldraw_library.c imp_ldraw_library.h \
	imp_ldraw_color.c imp_ldraw_color.h \
	imp_ldraw_mpd.c imp_ldraw_mpd.h
imp_ldraw_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
