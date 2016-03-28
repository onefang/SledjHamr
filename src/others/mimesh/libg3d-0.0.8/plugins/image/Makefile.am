pluginimgdir = $(pkglibdir)/plugins/image

pluginimg_LTLIBRARIES = \
	img_bmp.la \
	img_sgi.la \
	img_dds.la

if HAVE_GDKPIXBUF
pluginimg_LTLIBRARIES += img_gdkpixbuf.la
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

img_bmp_la_SOURCES = img_bmp.c
img_bmp_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
img_sgi_la_SOURCES = img_sgi.c
img_sgi_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@
img_dds_la_SOURCES = img_dds.c
img_dds_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@

img_gdkpixbuf_la_CFLAGS = ${AM_CFLAGS} @GDKPIXBUF_CFLAGS@
img_gdkpixbuf_la_SOURCES = img_gdkpixbuf.c
img_gdkpixbuf_la_LIBADD = $(top_builddir)/src/libg3d.la @GLIB_LIBS@ \
                          @GDKPIXBUF_LIBS@
