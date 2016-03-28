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

#include <string.h>

#include <g3d/stream.h>

typedef struct {
	gsize size;
	goffset offset;
	guint8 *buffer;
	gboolean free_buffer;
} G3DStreamBuffer;

static gsize g3d_stream_buffer_read(gpointer ptr, gsize size, gpointer data)
{
	G3DStreamBuffer *sbuf = (G3DStreamBuffer *)data;
	guint8 *outbufp = (guint8 *)ptr;
	gsize max_size;

	max_size = MIN(sbuf->size - sbuf->offset, size);
	memcpy(outbufp, sbuf->buffer + sbuf->offset, max_size);
	sbuf->offset += max_size;

	return max_size;
}

static gint g3d_stream_buffer_seek(gpointer data, goffset offset,
	GSeekType whence)
{
	G3DStreamBuffer *sbuf = (G3DStreamBuffer *)data;
	switch(whence) {
		case G_SEEK_SET:
			if((offset >= sbuf->size) || (offset < 0))
				return -1;
			sbuf->offset = offset;
			break;
		case G_SEEK_CUR:
			if(((sbuf->offset + offset) >= sbuf->size) ||
				((sbuf->offset + offset) < 0))
				return -1;
			sbuf->offset += offset;
			break;
		case G_SEEK_END:
			if(((sbuf->size - 1 + offset) >= sbuf->size) ||
				((sbuf->size - 1 + offset) < 0))
				return -1;
			sbuf->offset = sbuf->size - 1 + offset;
			break;
	}
	return 0;
}

static goffset g3d_stream_buffer_tell(gpointer data)
{
	G3DStreamBuffer *sbuf = (G3DStreamBuffer *)data;
	return sbuf->offset;
}

static goffset g3d_stream_buffer_size(gpointer data)
{
	G3DStreamBuffer *sbuf = (G3DStreamBuffer *)data;
	return sbuf->size;
}

static gboolean g3d_stream_buffer_eof(gpointer data)
{
	G3DStreamBuffer *sbuf = (G3DStreamBuffer *)data;
	return (sbuf->offset >= sbuf->size);
}

static gint g3d_stream_buffer_close(gpointer data)
{
	G3DStreamBuffer *sbuf = (G3DStreamBuffer *)data;
	if(sbuf->free_buffer)
		g_free(sbuf->buffer);
	g_free(sbuf);
	return 0;
}

EAPI
G3DStream *g3d_stream_from_buffer(guint8 *buffer, gsize size,
	const gchar *title, gboolean free_buffer)
{
	G3DStream *stream;
	G3DStreamBuffer *sbuf;
	guint32 flags = 0;
	gchar *uri;
	unsigned int usize = ((unsigned int)size);

	sbuf = g_new0(G3DStreamBuffer, 1);
	sbuf->buffer = buffer;
	sbuf->size = size;
	sbuf->free_buffer = free_buffer;
	flags |= (1 << G3D_STREAM_SEEKABLE) | (1 << G3D_STREAM_READABLE);
	uri = g_strdup_printf("buffer://0x%8p:%u/%s", buffer, usize,
		(title ? title : "unnamed"));
	stream = g3d_stream_new_custom(flags, uri,
		g3d_stream_buffer_read,
		NULL /* use generic implementation of *_read_line */,
		g3d_stream_buffer_seek, g3d_stream_buffer_tell,
		g3d_stream_buffer_size, g3d_stream_buffer_eof,
		g3d_stream_buffer_close, sbuf, NULL);
	g_free(uri);
	return stream;
}

