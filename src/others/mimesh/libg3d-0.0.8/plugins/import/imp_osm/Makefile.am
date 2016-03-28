pluginimpdir = $(pkglibdir)/plugins/import

if EXPERIMENTAL
if HAVE_LIBXML2
pluginimp_LTLIBRARIES = imp_osm.la
endif
endif

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-DG_LOG_DOMAIN=\"LibG3D\" \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@ @LIBXML2_CFLAGS@

INCLUDES = -I$(top_srcdir)/include

AM_LDFLAGS = \
	-no-undefined -module \
	@PLUGINS_LDFLAGS@

LIBTOOL += --quiet

imp_osm_la_SOURCES = imp_osm.c
imp_osm_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@ @LIBXML2_LIBS@
