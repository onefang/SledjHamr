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
#include <g3d/config.h>

#include <g3d/types.h>
#include <g3d/stream.h>

#include "imp_3dm_types.h"

static guint32 tdm_object_read_vector_data(G3DStream *stream,
	G3DVector *vertex_data, guint32 vsize, guint32 vcount)
{
	gsize nb = 0;
	gint32 i, j;

	for(i = 0; i < vcount; i ++)
		for(j = 0; j < vsize; j ++) {
			vertex_data[i * vsize + j] = g3d_stream_read_float_le(stream);
			nb += 4;
		}
	return nb;
}

gboolean tdm_object_read_vertex_data_uncompressed(TdmGlobal *global,
	TdmLocal *local)
{
	TdmObjectRecord *obj = local->object;

	local->len -= tdm_object_read_vector_data(global->stream,
		obj->object->vertex_data, 3, obj->object->vertex_count);
	return TRUE;
}

gboolean tdm_object_read_vertex_data_compressed(TdmGlobal *global,
	TdmLocal *local)
{
#if HAVE_ZLIB
	TdmObjectRecord *obj = local->object;
	G3DStream *zstream;
	guint32 csize;

	g3d_stream_read_int32_le(global->stream); /* tcode */
	csize = g3d_stream_read_int32_le(global->stream) - 4; /* size */
	local->len -= 8;

	zstream = g3d_stream_zlib_inflate_stream(global->stream, csize);
	if(zstream) {
		tdm_object_read_vector_data(zstream,
			obj->object->vertex_data, 3, obj->object->vertex_count);
		local->len -= csize;
		g3d_stream_close(zstream);
		/* CRC */
		g3d_stream_read_int32_le(global->stream);
		local->len -= 4;
		return TRUE;
	};
#else
	g_warning("no zlib support, unable to read deflated data");
#endif /* HAVE_ZLIB */
	return FALSE;
}
