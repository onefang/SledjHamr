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

#include <g3d/types.h>
#include <g3d/stream.h>

#include <zlib.h>

typedef struct {
	G3DStream *stream;
	gchar *uri;
	z_streamp zstream;
	gsize cmp_size;   /* compressed size */
	gsize cmp_read;   /* compressed bytes read */
	goffset offset;   /* offset (decompressed) */
	guint8 *inbuf;
	guint8 *inbufp;
	guint8 *buffer;
	guint8 *bufp;
	gsize bufpages;
	gsize bufavail;
	gboolean eof;
} G3DStreamZlib;

#define G3D_Z_CHUNK_SIZE (128 * 1024)

static gsize zlib_refill_buffer(G3DStreamZlib *sz)
{
	gint ret;

	/* FIXME: don't read beyond inflate stream */
	sz->zstream->avail_in = g3d_stream_read(sz->stream, sz->inbuf,
		MIN(G3D_Z_CHUNK_SIZE, (sz->cmp_size - sz->cmp_read)));
	sz->cmp_read += sz->zstream->avail_in;
	sz->zstream->next_in = sz->inbuf;
#if DEBUG > 0
	g_debug("ZLIB: %d bytes in input buffer", sz->zstream->avail_in);
#endif

	/* clean up previous refill */
	if(sz->buffer)
		g_free(sz->buffer);
	sz->buffer = NULL;
	sz->bufp = NULL;
	sz->bufpages = 0;

	/* decompress */
	do {
		/* add an output page */
		sz->buffer = g_realloc(sz->buffer, (sz->bufpages + 1) *
			G3D_Z_CHUNK_SIZE * sizeof(guint8));
		sz->bufp = sz->buffer + G3D_Z_CHUNK_SIZE * sz->bufpages;
		sz->bufpages ++;

		sz->zstream->avail_out = G3D_Z_CHUNK_SIZE;
		sz->zstream->next_out = sz->bufp;
		ret = inflate(sz->zstream, Z_NO_FLUSH);
		switch(ret) {
			case Z_STREAM_ERROR:
				g_warning("stream error");
				return 0;
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				g_warning("zlib error: %d", ret);
				return 0;
		}
		sz->bufavail += (G3D_Z_CHUNK_SIZE - sz->zstream->avail_out);
#if DEBUG > 0
		g_debug("ZLIB: %d bytes in output buffer", sz->bufavail);
#endif
	} while(sz->zstream->avail_out == 0);

	sz->bufp = sz->buffer;
	return sz->bufavail;
}

static gsize g3d_stream_zlib_read(gpointer ptr, gsize size, gpointer data)
{
	G3DStreamZlib *sz = data;
	gsize n = size, nc, br = 0;
	guint8 *bufp = ptr;

#if DEBUG > 3
	g_debug("ZLIB: reading %d bytes...", n);
#endif

	while(n > 0) {
		if(sz->bufavail == 0)
			zlib_refill_buffer(sz);

		if(sz->bufavail == 0) {
			sz->eof = TRUE;
			return 0;
		}

		nc = MIN(n, sz->bufavail);
		memcpy(bufp, sz->bufp, nc);
		sz->bufp += nc;
		bufp += nc;
		sz->bufavail -= nc;
		n -= nc;
		br += nc;
		sz->offset += nc;
#if DEBUG > 3
		g_debug("ZLIB: copied %d bytes [%02x]", nc, ((guint8 *)ptr)[0]);
#endif
	}

	return br;
}

static goffset g3d_stream_zlib_tell(gpointer data)
{
	G3DStreamZlib *sz = data;
	return sz->offset;
}

static gboolean g3d_stream_zlib_eof(gpointer data)
{
	G3DStreamZlib *sz = data;
	return sz->eof;
}

static gint g3d_stream_zlib_close(gpointer data)
{
	G3DStreamZlib *sz = data;

	if(sz->inbuf)
		g_free(sz->inbuf);
	if(sz->buffer)
		g_free(sz->buffer);

	inflateEnd(sz->zstream);
	g_free(sz->zstream);
	g_free(sz->uri);
	g_free(sz);
	return 0;
}

/*****************************************************************************/

EAPI
G3DStream *g3d_stream_zlib_inflate_stream(G3DStream *stream, gsize cmp_size)
{
	G3DStreamZlib *sz;
	guint32 flags = 0;
	gint ret;

	sz = g_new0(G3DStreamZlib, 1);
	sz->stream = stream;
	sz->cmp_size = cmp_size;
	sz->zstream = g_new0(z_stream, 1);
	ret = inflateInit(sz->zstream);
	if(ret != Z_OK) {
		g_warning("stream_zlib: failed to init");
		g_free(sz->zstream);
		g_free(sz);
	}

	sz->inbuf = g_new0(guint8, G3D_Z_CHUNK_SIZE);

	flags |= (1 << G3D_STREAM_READABLE);
	sz->uri = g_strdup_printf("%s:gzipped@0x%08x", stream->uri,
		(guint32)g3d_stream_tell(stream));

	return g3d_stream_new_custom(flags, sz->uri,
		g3d_stream_zlib_read, NULL /* readline */,
		NULL /* seek */, g3d_stream_zlib_tell,
		NULL /* size */, g3d_stream_zlib_eof,
		g3d_stream_zlib_close, sz, NULL);
}

