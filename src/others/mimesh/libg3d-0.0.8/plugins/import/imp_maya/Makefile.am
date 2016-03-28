pluginimpdir = $(pkglibdir)/plugins/import

if EXPERIMENTAL
pluginimp_LTLIBRARIES = imp_maya.la
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

imp_maya_la_SOURCES = imp_maya.c imp_maya_chunks.h \
	imp_maya_callbacks.c imp_maya_callbacks.h \
	imp_maya_obj.c imp_maya_obj.h \
	imp_maya_var.c imp_maya_var.h
imp_maya_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
