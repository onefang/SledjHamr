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

#include <g3d/types.h>
#include <g3d/stream.h>

EAPI
gint32 g3d_stream_read_int8(G3DStream *stream)
{
	guint8 c;
	if(g3d_stream_read(stream, &c, 1) != 1)
		return 0;
	return c;
}

EAPI
gint32 g3d_stream_read_int16_be(G3DStream *stream)
{
	return (g3d_stream_read_int8(stream) << 8) | g3d_stream_read_int8(stream);
}

EAPI
gint32 g3d_stream_read_int16_le(G3DStream *stream)
{
	return g3d_stream_read_int8(stream) | (g3d_stream_read_int8(stream) << 8);
}

EAPI
gint32 g3d_stream_read_int32_be(G3DStream *stream)
{
	return (g3d_stream_read_int8(stream) << 24) |
		(g3d_stream_read_int8(stream) << 16) |
		(g3d_stream_read_int8(stream) << 8) | g3d_stream_read_int8(stream);
}

EAPI
gint32 g3d_stream_read_int32_le(G3DStream *stream)
{
	return g3d_stream_read_int8(stream) | (g3d_stream_read_int8(stream) << 8) |
		(g3d_stream_read_int8(stream) << 16) |
		(g3d_stream_read_int8(stream) << 24);
}

static void g3d_stream_read_bytes(G3DStream *stream, guint8 *buf, gsize n)
{
	gint32 i;
	for(i = 0; i < n; i ++)
		buf[i] = g3d_stream_read_int8(stream);
}

static void g3d_stream_read_bytes_swap(G3DStream *stream, guint8 *buf, gsize n)
{
	gint32 i;
	for(i = (n - 1); i >= 0; i --)
		buf[i] = g3d_stream_read_int8(stream);
}

EAPI
G3DFloat g3d_stream_read_float_be(G3DStream *stream)
{
	union {
		G3DFloat f;
		guint8 u[4];
	} u;
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	g3d_stream_read_bytes_swap(stream, u.u, 4);
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	g3d_stream_read_bytes(stream, u.u, 4);
#endif
	return u.f;
}

EAPI
G3DFloat g3d_stream_read_float_le(G3DStream *stream)
{
	union {
		G3DFloat f;
		guint8 u[4];
	} u;
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	g3d_stream_read_bytes(stream, u.u, 4);
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	g3d_stream_read_bytes_swap(stream, u.u, 4);
#endif
	return u.f;
}


EAPI
G3DDouble g3d_stream_read_double_be(G3DStream *stream)
{
	union {
		G3DDouble f;
		guint8 u[8];
	} u;
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	g3d_stream_read_bytes_swap(stream, u.u, 8);
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	g3d_stream_read_bytes(stream, u.u, 8);
#endif
	return u.f;
}

EAPI
G3DDouble g3d_stream_read_double_le(G3DStream *stream)
{
	union {
		G3DDouble f;
		guint8 u[8];
	} u;
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	g3d_stream_read_bytes(stream, u.u, 8);
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	g3d_stream_read_bytes_swap(stream, u.u, 8);
#endif
	return u.f;
}

EAPI
gint32 g3d_stream_read_cstr(G3DStream *stream, gchar *buffer, gint32 max_len)
{
	gint32 n = 0;
	gchar c;

	do {
		c = g3d_stream_read_int8(stream);
		buffer[n] = c;
		n ++;
	} while((c != 0) && (n < max_len));
	buffer[max_len - 1] = '\0';
	return n;
}

