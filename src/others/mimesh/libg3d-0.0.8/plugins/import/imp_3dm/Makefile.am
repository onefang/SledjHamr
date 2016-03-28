pluginimpdir = $(pkglibdir)/plugins/import

if EXPERIMENTAL
pluginimp_LTLIBRARIES = imp_3dm.la
endif

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-DG_LOG_DOMAIN=\"LibG3D\" \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include

AM_LDFLAGS = \
	-no-undefined -module \
	@PLUGINS_LDFLAGS@

LIBTOOL += --quiet

imp_3dm_la_SOURCES = imp_3dm.c imp_3dm.h \
	imp_3dm_callbacks.h imp_3dm_callbacks.c \
	imp_3dm_object.c imp_3dm_object.h \
	imp_3dm_chunks.h imp_3dm_object_types.h imp_3dm_types.h
imp_3dm_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@

