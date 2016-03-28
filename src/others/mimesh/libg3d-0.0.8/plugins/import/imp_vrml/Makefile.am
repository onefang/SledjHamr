pluginimpdir = $(pkglibdir)/plugins/import

if OS_WIN32
else
pluginimp_LTLIBRARIES = imp_vrml.la
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

AM_LFLAGS = -o${LEX_OUTPUT_ROOT}.c --header-file=${LEX_OUTPUT_ROOT}.h -R
LIBTOOL += --quiet

imp_vrml_la_SOURCES = imp_vrml_v1.l imp_vrml_v1.h imp_vrml.c
imp_vrml_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@ ${LEXLIB}

EXTRA_DIST = imp_vrml_v1.c imp_vrml_v1.h
