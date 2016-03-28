/* $Id:$ */

/*
    libg3d - 3D object loading library

    Copyright (C) 2005-2009  Markus Dahms <mad@automagically.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _G3D_STREAM_H
#define _G3D_STREAM_H

#include <glib.h>
#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:stream
 * @short_description: I/O abstraction layer for plugins
 * @see_also: #G3DStream
 * @include: g3d/stream.h
 *
 * A stream is an abstraction for data input. It enables plugins to read
 * data from a file, a memory buffer, a container file or some other medium.
 */

enum {
	/* shift offsets */
	G3D_STREAM_SEEKABLE = 0x00,
	G3D_STREAM_READABLE = 0x01,
	G3D_STREAM_WRITABLE = 0x02
};

/**
 * G3DStreamReadFunc:
 * @ptr: buffer to read bytes into
 * @size: number of bytes to read
 * @data: opaque stream data
 *
 * Callback function for g3d_stream_read().
 *
 * Returns: number of bytes actually read.
 */
typedef gsize (* G3DStreamReadFunc)(gpointer ptr, gsize size, gpointer data);

/**
 * G3DStreamReadLineFunc:
 * @buf: buffer to read bytes into
 * @size: maximum size of buffer
 * @data: opaque stream data
 *
 * Callback function for g3d_stream_read_line().
 *
 * Returns: The line buffer or NULL in case of an error.
 */
typedef gchar *(* G3DStreamReadLineFunc)(gchar *buf, gsize size,
	gpointer data);

/**
 * G3DStreamSeekFunc:
 * @data: opaque stream data
 * @offset: seek offset
 * @whence: seek type
 *
 * Callback function for g3d_stream_seek().
 *
 * Returns: 0 on success, -1 else.
 */
typedef gint (*G3DStreamSeekFunc)(gpointer data, goffset offset,
	GSeekType whence);

/**
 * G3DStreamTellFunc:
 * @data: opaque stream data
 *
 * Callback function for g3d_stream_tell().
 *
 * Returns: current stream position.
 */
typedef goffset (*G3DStreamTellFunc)(gpointer data);

/**
 * G3DStreamSizeFunc:
 * @data: opaque stream data
 *
 * Callback function for g3d_stream_size().
 *
 * Returns: size of stream.
 */
typedef goffset (*G3DStreamSizeFunc)(gpointer data);

/**
 * G3DStreamEofFunc:
 * @data: opaque stream data
 *
 * Callback function for g3d_stream_eof().
 *
 * Returns: TRUE on stream end-of-file, FALSE else.
 */
typedef gboolean (*G3DStreamEofFunc)(gpointer data);

/**
 * G3DStreamCloseFunc:
 * @data: opaque stream data
 *
 * Callback function for g3d_stream_close().
 *
 * Returns: 0 on success, -1 else.
 */
typedef gint (*G3DStreamCloseFunc)(gpointer data);

/**
 * G3DStream:
 *
 * An abstraction of input handling.
 */
struct _G3DStream {
	/*< private >*/
	guint32 flags;
	gchar *uri;
	G3DStreamReadFunc read;
	G3DStreamReadLineFunc readline;
	G3DStreamSeekFunc seek;
	G3DStreamTellFunc tell;
	G3DStreamSizeFunc size;
	G3DStreamEofFunc eof;
	G3DStreamCloseFunc close;
	gpointer data;
	guint32 linecount;
	struct _G3DStream  *zip_container;
};

/* public interface */

/**
 * g3d_stream_is_seekable:
 * @stream: the stream
 *
 * Get information whether it is possible to seek in a stream.
 *
 * Returns: TRUE if seekable, FALSE else
 */
EAPI
gboolean g3d_stream_is_seekable(G3DStream *stream);

/**
 * g3d_stream_get_uri:
 * @stream: the stream
 *
 * Get the URI of a stream
 *
 * Returns: a non-NULL, zero-terminated string containing the URI of the
 * string. This return value should not be freed.
 */
EAPI
gchar *g3d_stream_get_uri(G3DStream *stream);

/**
 * g3d_stream_read_int8:
 * @stream: the stream to read from
 *
 * Read a 1 byte signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
gint32 g3d_stream_read_int8(G3DStream *stream);

/**
 * g3d_stream_read_int16_be:
 * @stream: the stream to read from
 *
 * Read a 2 byte big-endian signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
gint32 g3d_stream_read_int16_be(G3DStream *stream);

/**
 * g3d_stream_read_int16_le:
 * @stream: the stream to read from
 *
 * Read a 2 byte little-endian signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
gint32 g3d_stream_read_int16_le(G3DStream *stream);

/**
 * g3d_stream_read_int32_be:
 * @stream: the stream to read from
 *
 * Read a 4 byte big-endian signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
gint32 g3d_stream_read_int32_be(G3DStream *stream);

/**
 * g3d_stream_read_int32_le:
 * @stream: the stream to read from
 *
 * Read a 4 byte little-endian signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
gint32 g3d_stream_read_int32_le(G3DStream *stream);

/**
 * g3d_stream_read_float_be:
 * @stream: the stream to read from
 *
 * Read a 4 byte big-endian floating point number from file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
G3DFloat g3d_stream_read_float_be(G3DStream *stream);

/**
 * g3d_stream_read_float_le:
 * @stream: the stream to read from
 *
 * Read a 4 byte little-endian floating point number from file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
G3DFloat g3d_stream_read_float_le(G3DStream *stream);

/**
 * g3d_stream_read_double_be:
 * @stream: the stream to read from
 *
 * Read a 8 byte big-endian double-precision floating point number from file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
G3DDouble g3d_stream_read_double_be(G3DStream *stream);

/**
 * g3d_stream_read_double_le:
 * @stream: the stream to read from
 *
 * Read a 8 byte little-endian double-precision floating point number from
 * file.
 *
 * Returns: The read value, 0 in case of error
 */
EAPI
G3DDouble g3d_stream_read_double_le(G3DStream *stream);

/**
 * g3d_stream_read_cstr:
 * @stream: the stream to read from
 * @buffer: the buffer to fill
 * @max_len: maximum number to read from stream
 *
 * Read a string (terminated by '\0') from stream
 *
 * Returns: number of bytes read from stream
 */
EAPI
gint32 g3d_stream_read_cstr(G3DStream *stream, gchar *buffer, gint32 max_len);

/**
 * g3d_stream_read:
 * @stream: the stream to read from
 * @ptr: pointer to memory storage
 * @size: number of bytes to read
 *
 * Reads a number of bytes from the stream.
 *
 * Returns: number of bytes successfully read.
 */
EAPI
gsize g3d_stream_read(G3DStream *stream, gpointer ptr, gsize size);

/**
 * g3d_stream_read_line:
 * @stream: stream to read a line from
 * @buf: an allocated buffer to be filled
 * @size: maximum length of line including terminating zero
 *
 * Read a line (terminated by a newline character or end of file) from a
 * stream.
 *
 * Returns: the read line or NULL in case of an error.
 */
EAPI
gchar *g3d_stream_read_line(G3DStream *stream, gchar *buf, gsize size);

/**
 * g3d_stream_skip:
 * @stream: stream to skip bytes from
 * @offset: number of bytes to skip
 *
 * Skip a number of bytes (>= 0) in stream even if it does not support
 * seeking.
 *
 * Returns: 0 on success, -1 else
 */
EAPI
gint g3d_stream_skip(G3DStream *stream, goffset offset);

/**
 * g3d_stream_seek:
 * @stream: stream to seek in
 * @offset: number of bytes to seek
 * @whence: seek type
 *
 * Moves around the current position in the stream.
 *
 * Returns: 0 on success, -1 else
 */
EAPI
gint g3d_stream_seek(G3DStream *stream, goffset offset, GSeekType whence);

/**
 * g3d_stream_tell:
 * @stream: stream to get position from
 *
 * Tells the current position in the stream.
 *
 * Returns: current stream position
 */
EAPI
goffset g3d_stream_tell(G3DStream *stream);

/**
 * g3d_stream_line:
 * @stream: stream to get line from
 *
 * Get the current line number from stream. This only works if line are
 * consequently read with g3d_stream_read_line(), so it's only applicable
 * for text streams.
 *
 * Returns: current line number, may be 0
 */
EAPI
guint32 g3d_stream_line(G3DStream *stream);

/**
 * g3d_stream_size:
 * @stream: stream to get size from
 *
 * Get the size in bytes of a stream.
 *
 * Returns: size of stream in bytes
 */
EAPI
goffset g3d_stream_size(G3DStream *stream);

/**
 * g3d_stream_eof:
 * @stream: the stream
 *
 * Checks whether the stream has reached its end.
 *
 * Returns: TRUE if no more data can be read, FALSE else.
 */
EAPI
gboolean g3d_stream_eof(G3DStream *stream);

/**
 * g3d_stream_close:
 * @stream: the stream
 *
 * Closes an open stream.
 *
 * Returns: 0 on success.
 */
EAPI
gint g3d_stream_close(G3DStream *stream);

/**
 * g3d_stream_new_custom:
 * @flags: stream capability flags
 * @uri: URI of new stream, must not be NULL
 * @readfunc: read callback function
 * @readlinefunc: read line callback function, may be NULL in which case
 * line reading is emulated with g3d_stream_read()
 * @seekfunc: seek callback function
 * @tellfunc: tell callback function
 * @sizefunc: size callback function
 * @eoffunc: end-of-file callback function
 * @closefunc: close callback function
 * @data: opaque data for all callback functions
 * @zip_container: original zip container
 *
 * Creates a new #G3DStream with custom callback functions.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error.
 */
EAPI
G3DStream *g3d_stream_new_custom(guint32 flags, const gchar *uri,
	G3DStreamReadFunc readfunc, G3DStreamReadLineFunc readlinefunc,
	G3DStreamSeekFunc seekfunc, G3DStreamTellFunc tellfunc,
	G3DStreamSizeFunc sizefunc,
	G3DStreamEofFunc eoffunc, G3DStreamCloseFunc closefunc, gpointer data,
	G3DStream *zip_container);

/**
 * g3d_stream_open_file:
 * @filename: the name of the file to open
 * @mode: the mode to open the file, as given to fopen()
 *
 * Opens a file with the C stdio routines.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error.
 */
EAPI
G3DStream *g3d_stream_open_file(const gchar *filename, const gchar *mode);

/**
 * g3d_stream_from_buffer:
 * @buffer: memory buffer to use
 * @size: size of buffer
 * @title: optional title of stream, may be NULL
 * @free_buffer: whether to free the memory with g_free() on g3d_stream_close()
 *
 * Use a buffer in memory as #G3DStream.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error.
 */
EAPI
G3DStream *g3d_stream_from_buffer(guint8 *buffer, gsize size,
	const gchar *title, gboolean free_buffer);

#ifdef HAVE_ZLIB

/**
 * g3d_stream_zlib_inflate_stream:
 * @stream: a parent stream
 * @cmp_size: the compressed size of the deflated part
 *
 * Opens a new stream to decompress zlib-deflated parts of a stream.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error
 */
EAPI
G3DStream *g3d_stream_zlib_inflate_stream(G3DStream *stream, gsize cmp_size);

#endif /* HAVE_ZLIB */

#ifdef HAVE_LIBGSF

/**
 * g3d_stream_open_gzip_from_stream:
 * @stream: stream to read from
 *
 * Reads data from a gzip-compressed stream.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error.
 */
EAPI
G3DStream *g3d_stream_open_gzip_from_stream(G3DStream *stream);

/**
 * g3d_stream_open_structured_file:
 * @filename: name of container file
 * @subfile: name of (contained) sub-file
 *
 * Open a file within a Structured File as #G3DStream.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error.
 */
EAPI
G3DStream *g3d_stream_open_structured_file(const gchar *filename,
	const gchar *subfile);

/**
 * g3d_stream_open_structured_file_from_stream:
 * @stream: stream of container file
 * @subfile: name of (contained) sub-file
 *
 * Open a file within a Structured File which is opened as a stream. At the
 * moment this only works for streams opened by g3d_stream_open_file() as
 * the file is directly opened again.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error.
 */
EAPI
G3DStream *g3d_stream_open_structured_file_from_stream(G3DStream *stream,
	const gchar *subfile);

/**
 * g3d_stream_open_zip:
 * @filename: name of container file
 * @subfile: name of (contained) sub-file
 *
 * Open a file within a Zip archive.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error.
 */
EAPI
G3DStream *g3d_stream_open_zip(const gchar *filename, const gchar *subfile);

/**
 * g3d_stream_open_zip_from_stream:
 * @stream: stream of container file
 * @subfile: name of (contained) sub-file
 *
 * Open a file within a Zip archive which is opened as a stream. At the
 * moment this only works for streams opened by g3d_stream_open_file() as
 * the file is directly opened again.
 *
 * Returns: a newly allocated #G3DStream or NULL in case of an error.
 */
EAPI
G3DStream *g3d_stream_open_zip_from_stream(G3DStream *stream,
	const gchar *subfile);
#endif

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* _G3D_STREAM_H */

