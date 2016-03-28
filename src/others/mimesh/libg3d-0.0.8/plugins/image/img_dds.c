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

#include <stdio.h>
#include <string.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/iff.h>

static gboolean decode_dxt1(G3DImage *image, G3DStream *stream);

EAPI
gboolean plugin_load_image_from_stream(G3DContext *context, G3DStream *stream,
	G3DImage *image, gpointer user_data)
{
	guint32 magic, size, flags, depth;
	guint32 pfsize, pfflags, pffourcc, pfbpp;
	guint32 pfrmask, pfgmask, pfbmask, pfamask;
	guint32 caps1, caps2;
	gchar *sfourcc;

	magic = g3d_stream_read_int32_be(stream);
	if(magic != G3D_IFF_MKID('D','D','S',' ')) {
		g_warning("%s is not a DDS file", stream->uri);
		return FALSE;
	}
	image->name = g_strdup(stream->uri);

	size = g3d_stream_read_int32_le(stream);
	flags = g3d_stream_read_int32_le(stream);
	image->height = g3d_stream_read_int32_le(stream);
	image->width = g3d_stream_read_int32_le(stream);
	g3d_stream_read_int32_le(stream); /* pitch or linesize */
	depth = g3d_stream_read_int32_le(stream);
	g3d_stream_read_int32_le(stream); /* num mipmaps */

	g3d_stream_skip(stream, 44);

	pfsize = g3d_stream_read_int32_le(stream);
	pfflags = g3d_stream_read_int32_le(stream);

	pffourcc = g3d_stream_read_int32_be(stream);
	sfourcc = g3d_iff_id_to_text(pffourcc);

	pfbpp = g3d_stream_read_int32_le(stream);
	pfrmask = g3d_stream_read_int32_le(stream);
	pfgmask = g3d_stream_read_int32_le(stream);
	pfbmask = g3d_stream_read_int32_le(stream);
	pfamask = g3d_stream_read_int32_le(stream);
	caps1 = g3d_stream_read_int32_le(stream);
	caps2 = g3d_stream_read_int32_le(stream);

	g3d_stream_skip(stream, 12);

#if DEBUG > 0
	g_debug("DDS: %ux%u %s 0x%08x", image->width, image->height,
		sfourcc, pfflags);
	g_debug("DDS: masks: 0x%04x, 0x%04x, 0x%04x, 0x%04x",
		pfrmask, pfgmask, pfbmask, pfamask);
#endif

	image->pixeldata = g_new0(guint8, image->width * image->height * 4);

	switch(pffourcc) {
		case G3D_IFF_MKID('D','X','T','1'):
			decode_dxt1(image, stream);
			break;
		default:
			g_warning("DDS: unsupported FOURCC: %s", sfourcc);
	}

	g_free(sfourcc);
	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("DDS textures.\n"
		"Author: Markus Dahms");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("dds", ":", 0);
}

/*****************************************************************************/

static void decode_rgb565(guint16 c, G3DFloat *r, G3DFloat *g, G3DFloat *b)
{
	*r = ((c & 0xF800) >> 11) / 32.0;
	*g = ((c & 0x07E0) >> 5)  / 64.0;
	*b = (c & 0x001F)         / 32.0;
}

static gboolean decode_dxt1(G3DImage *image, G3DStream *stream)
{
	gint32 x, y, i, j;
	guint32 index;
	G3DFloat r, g, b, r1, r2, g1, g2, b1, b2;
	guint8 line, v2;

	for(y = 0; y < image->height; y += 4) {
		for(x = 0; x < image->width; x += 4) {
			decode_rgb565(g3d_stream_read_int16_le(stream), &r1, &g1, &b1);
			decode_rgb565(g3d_stream_read_int16_le(stream), &r2, &g2, &b2);
			for(j = 0; j < 4; j ++) {
				line = g3d_stream_read_int8(stream);
				for(i = 0; i < 4; i ++) {
					v2 = line & 0x3;
					line >>= 2;
					r = r1 + ((r2 - r1) / 3.0) * v2;
					g = g1 + ((g2 - g1) / 3.0) * v2;
					b = b1 + ((b2 - b1) / 3.0) * v2;
					index = ((image->height - 4) - y + j) *
						image->width + x + i;
					image->pixeldata[index * 4 + 0] = r * 255.0;
					image->pixeldata[index * 4 + 1] = g * 255.0;
					image->pixeldata[index * 4 + 2] = b * 255.0;
					image->pixeldata[index * 4 + 3] = 0xFF;
				}
			}
		}
	}

	return TRUE;
}
