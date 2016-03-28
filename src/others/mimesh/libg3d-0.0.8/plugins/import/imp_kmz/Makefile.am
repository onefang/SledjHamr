pluginimpdir = $(pkglibdir)/plugins/import

if HAVE_LIBXML2
if HAVE_LIBGSF
pluginimp_LTLIBRARIES = imp_kmz.la
endif
endif

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-DG_LOG_DOMAIN=\"LibG3D\" \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@ @LIBXML2_CFLAGS@ \
	@LIBGSF_CFLAGS@

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include

AM_LDFLAGS = \
	-no-undefined -module \
	@PLUGINS_LDFLAGS@

LIBTOOL += --quiet

imp_kmz_la_SOURCES = imp_kmz.c
imp_kmz_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@ @LIBGSF_LIBS@ \
                    @LIBXML2_LIBS@
