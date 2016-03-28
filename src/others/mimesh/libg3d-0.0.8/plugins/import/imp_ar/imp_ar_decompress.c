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
#include <glib.h>
#include <g3d/stream.h>

#include "imp_ar.h"

#define AR_FLAG_COPIED    0x80
#define AR_FLAG_COMPRESS  0x40

guint8 *ar_decompress_chunk(guint8 *src, guint16 srcsize, guint16 *dstsize)
{
	guint8 *dst = NULL, bit = 16;
	gint32 i, j = 0, k, pos, size;
	guint16 cmd;

#if DEBUG > 2
	printf("D: decompressing %d bytes chunk\n", srcsize);
#endif

	if(src[0] == AR_FLAG_COPIED)
	{
		*dstsize = srcsize - 1;
		dst = g_new0(guint8, *dstsize);
		memcpy(dst, src + 1, *dstsize);
		return dst;
	}

	*dstsize = 0;
	cmd = (src[1] << 8) + src[2];
	for(i = 3; i < srcsize;)
	{
		if(bit == 0)
		{
			/* get new command */
			cmd = (src[i] << 8) + src[i + 1];
			i += 2;
			bit = 16;
		}

		if(cmd & 0x8000)
		{
			pos = (src[i] << 4) + (src[i + 1] >> 4);
			i ++;
			if(pos != 0)
			{
				/* copy known chunk */
				size = (src[i] & 0xF) + 3;
				*dstsize += size;
				dst = g_realloc(dst, *dstsize);
				i ++;
				for(k = 0; k < size; k ++)
					dst[j + k] = dst[j - pos + k];
				j += size;
			}
			else
			{
				/* RLE style... */
				size = (src[i] << 8) + src[i + 1] + 16;
				*dstsize += size;
				dst = g_realloc(dst, *dstsize);
				i += 2;
				for(k = 0; k < size; k ++)
					dst[j + k] = src[i];
				i ++;
				j += size;
			}
		}
		else
		{
			/* plain copy */
			*dstsize += 1;
			dst = g_realloc(dst, *dstsize);
			dst[j] = src[i];
			i ++;
			j ++;
		}

		cmd <<= 1;
		bit --;
	}

	return dst;
}

gboolean ar_decompress_to_file(G3DStream *stream, ArDirEntry *dirent)
{
	FILE *o;
	gchar cmd;
	guint32 size;
	guint16 srcsize, dstsize;
	guint8 *src, *dst;

	o = fopen(dirent->name, "wb");
	if(o == NULL) {
		g_warning("failed to write to '%s'", dirent->name);
		return FALSE;
	}

	/* seek to file start */
	g3d_stream_seek(stream, dirent->offset, G_SEEK_SET);

	/* skip tags */
	do {
		cmd = g3d_stream_read_int8(stream);
		if(cmd != 'D') {
			size = g3d_stream_read_int32_le(stream);
			g3d_stream_skip(stream, size);
		}
	} while(cmd != 'D');

#if DEBUG > 2
	printf("D: starting decompression part\n");
#endif

	/* decompress stuff */
	while(1) {
		srcsize = g3d_stream_read_int16_le(stream);
		if(srcsize == 0)
			break;

		src = g_new0(guint8, srcsize);
		g3d_stream_read(stream, src, srcsize);
		dst = ar_decompress_chunk(src, srcsize, &dstsize);

		if(dstsize > 0) {
			fwrite(dst, 1, dstsize, o);
			g_free(dst);
		}

		g_free(src);
	}

	fclose(o);

	return TRUE;
}


