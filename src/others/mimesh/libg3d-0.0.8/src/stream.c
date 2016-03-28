/* $Id$ */

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

#include <g3d/stream.h>

EAPI
gboolean g3d_stream_is_seekable(G3DStream *stream)
{
	return (stream->flags & (1 << G3D_STREAM_SEEKABLE));
}

EAPI
gchar *g3d_stream_get_uri(G3DStream *stream)
{
	return stream->uri;
}

EAPI
gsize g3d_stream_read(G3DStream *stream, gpointer ptr, gsize size)
{
	if(stream->read)
		return stream->read(ptr, size, stream->data);
	return -1;
}

static gchar *generic_readline(G3DStream *stream, gchar *buf, gsize size)
{
	gint32 i;

	for(i = 0; i < (size - 1); i ++) {
		if(g3d_stream_eof(stream))
			return NULL;
		if(g3d_stream_read(stream, buf + i, 1) <= 0)
			return NULL;
		if(buf[i] == '\n') {
			buf[i + 1] = '\0';
			return buf;
		}
	}
	buf[size - 1] = '\0';
	return buf;
}

EAPI
gchar *g3d_stream_read_line(G3DStream *stream, gchar *buf, gsize size)
{
	stream->linecount ++;

	if(stream->readline)
		return stream->readline(buf, size, stream->data);
	else
		return generic_readline(stream, buf, size);
}

EAPI
gint g3d_stream_skip(G3DStream *stream, goffset offset)
{
/*	g_return_val_if_fail(offset >= 0, -1); */
	if(stream->seek)
		return stream->seek(stream->data, offset, G_SEEK_CUR);
	else {
		goffset todo = offset;
		gsize s;
		guint8 *buffer = g_new0(guint8, 1024);
		while(todo > 0) {
			s = MIN(todo, 1024);
			if(stream->read(buffer, s, stream->data) < s) {
				g_free(buffer);
				return -1;
			}
			offset -= s;
		}
		g_free(buffer);
		return 0;
	}
}

EAPI
gint g3d_stream_seek(G3DStream *stream, goffset offset, GSeekType whence)
{
	if(stream->seek)
		return stream->seek(stream->data, offset, whence);
	return -1;
}

EAPI
goffset g3d_stream_tell(G3DStream *stream)
{
	if(stream->tell)
		return stream->tell(stream->data);
	return 0;
}

EAPI
guint32 g3d_stream_line(G3DStream *stream)
{
	return stream->linecount;
}

EAPI
goffset g3d_stream_size(G3DStream *stream)
{
	if(stream->size)
		return stream->size(stream->data);
	return -1;
}

EAPI
gboolean g3d_stream_eof(G3DStream *stream)
{
	if(stream->eof)
		return stream->eof(stream->data);
	return FALSE;
}

EAPI
gint g3d_stream_close(G3DStream *stream)
{
	gint retval = 0;
	if(stream->close)
		retval = stream->close(stream->data);
	g_free(stream->uri);
	g_free(stream);
	return retval;
}

EAPI
G3DStream *g3d_stream_new_custom(guint32 flags, const gchar *uri,
	G3DStreamReadFunc readfunc, G3DStreamReadLineFunc readlinefunc,
	G3DStreamSeekFunc seekfunc, G3DStreamTellFunc tellfunc,
	G3DStreamSizeFunc sizefunc,
	G3DStreamEofFunc eoffunc, G3DStreamCloseFunc closefunc, gpointer data,
	G3DStream *zip_container)
{
	G3DStream *stream;

	stream = g_new0(G3DStream, 1);
	stream->flags = flags;
	stream->uri = g_strdup(uri);
	stream->read = readfunc;
	stream->readline = readlinefunc;
	stream->seek = seekfunc;
	stream->tell = tellfunc;
	stream->size = sizefunc;
	stream->eof = eoffunc;
	stream->close = closefunc;
	stream->data = data;
	stream->zip_container = zip_container;

	return stream;
}

