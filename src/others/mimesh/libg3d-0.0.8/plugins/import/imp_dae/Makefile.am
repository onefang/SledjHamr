pluginimpdir = $(pkglibdir)/plugins/import

if HAVE_LIBXML2
pluginimp_LTLIBRARIES = imp_dae.la
endif

AM_CFLAGS = \
	-Wall -ansi -pedantic \
	-DG_LOG_DOMAIN=\"LibG3D\" \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@ @LIBXML2_CFLAGS@

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include

AM_LDFLAGS = \
	-no-undefined -module \
	@PLUGINS_LDFLAGS@

LIBTOOL += --quiet

imp_dae_la_SOURCES = imp_dae.c \
	imp_dae_xml.h imp_dae_xml.c \
	imp_dae_library.h imp_dae_library.c \
	imp_dae_cb.h imp_dae_cb.c imp_dae_chunks.h
imp_dae_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@ @LIBXML2_LIBS@
