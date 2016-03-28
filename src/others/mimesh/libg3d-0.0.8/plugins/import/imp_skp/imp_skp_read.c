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
#include "imp_skp.h"
#include "imp_skp_read.h"

guint32 skp_read_xint16(G3DStream *stream)
{
	guint32 val;

	val = g3d_stream_read_int16_le(stream);
	if(val & 0x8000L) {
		val &= 0x7FFF;
		val |= (g3d_stream_read_int16_le(stream) << 16);
	}
	return val;
}

gchar *skp_read_char(G3DStream *stream)
{
	guint32 magic, n;
	gchar *text;

	magic = g3d_stream_read_int32_be(stream);
	if(magic != 0xffff0000) {
		g_warning("SKP: wrong text magic: 0x%08x", magic);
		return NULL;
	}
	n = g3d_stream_read_int16_le(stream);

	text = g_new0(gchar, n + 1);
	g3d_stream_read(stream, text, n);

	return text;
}

gchar *skp_read_wchar(G3DStream *stream)
{
	gint32 i;
	guint32 magic, n;
	gunichar2 *u16text;
	gchar *text;
	GError *error = NULL;

	magic = g3d_stream_read_int32_be(stream);
	if((magic & 0xFFFFFF00) != 0xfffeff00) {
#if DEBUG > 1
		g_debug("SKP: wrong UTF-16 magic: 0x%08x", magic);
#endif
		g3d_stream_seek(stream, -4, G_SEEK_CUR);
		return NULL;
	}
	n = magic & 0x000000FF;

	u16text = g_new0(gunichar2, n + 1);
	for(i = 0; i < n; i ++) {
		u16text[i] = g3d_stream_read_int16_le(stream);
	}

	text = g_utf16_to_utf8(u16text, n, NULL, NULL, &error);
	if(error != NULL) {
		g_warning("UTF-16 to UTF-8 conversion failed: %s",
			error->message);
		g_error_free(error);
	}
	g_free(u16text);

	return text;
}

gboolean skp_read_dbl3(G3DStream *stream,
	gdouble *d1, gdouble *d2, gdouble *d3)
{
	*d1 = g3d_stream_read_double_le(stream);
	*d2 = g3d_stream_read_double_le(stream);
	*d3 = g3d_stream_read_double_le(stream);
	return TRUE;
}

gboolean skp_read_10b(G3DStream *stream)
{
	guint32 x1, x2;
	guint8 u1, u2;

	x1 = g3d_stream_read_int32_be(stream);
	u1 = g3d_stream_read_int8(stream);

	if(((x1 & 0x00FFFFFF) != 0x0001) || (u1 != 0x01)) {
		g_warning("skp_read_10b: %#08x, %#02x", x1, u1);
	}
	u2 = g3d_stream_read_int8(stream);
	x2 = g3d_stream_read_int32_le(stream);
	g_debug("\tread 10b: %08x %02x %02x %08x", x1, u1, u2, x2);
	return TRUE;
}

