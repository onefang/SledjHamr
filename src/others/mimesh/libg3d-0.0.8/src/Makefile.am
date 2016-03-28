lib_LTLIBRARIES = libg3d.la

libg3d_la_SOURCES = \
	context.c \
	plugins.c \
	model.c \
	object.c \
	face.c \
	material.c \
	texture.c \
	stream.c stream_read.c stream_file.c stream_buffer.c \
	read.c \
	iff.c \
	primitive.c \
	vector.c \
	matrix.c \
	quat.c

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include

libg3d_la_CFLAGS = \
	-Wall -ansi -pedantic \
	-fstrict-aliasing -Wstrict-aliasing \
	-DG_LOG_DOMAIN=\"LibG3D\" \
	@DBG_CFLAGS@ -DDEBUG=@DEBUG@ \
	@GLIB_CFLAGS@

libg3d_la_LDFLAGS = \
	-no-undefined -version-info @LTVERSION@

libg3d_la_LIBADD = \
	@GLIB_LIBS@ \
	@MAGIC_LIBS@ \
	@MATH_LIBS@

if HAVE_LIBGSF
libg3d_la_CFLAGS += @LIBGSF_CFLAGS@
libg3d_la_LIBADD += @LIBGSF_LIBS@
libg3d_la_SOURCES += stream_gsf.c stream_gsf_class.c stream_gsf_class.h
endif
if HAVE_ZLIB
libg3d_la_CFLAGS += @Z_CFLAGS@
libg3d_la_LIBADD += @Z_LIBS@
libg3d_la_SOURCES += stream_zlib.c
endif
