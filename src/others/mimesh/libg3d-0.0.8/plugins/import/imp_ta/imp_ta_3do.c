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

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/matrix.h>

gboolean ta_3do_read_children(G3DContext *context, G3DStream *stream,
	GSList **list, G3DMaterial *materials);

gboolean ta_3do_load_object(G3DContext *context, G3DStream *stream,
	G3DModel *model, G3DMaterial *materials)
{
	return ta_3do_read_children(context, stream, &(model->objects), materials);
}

gboolean ta_3do_read_children(G3DContext *context, G3DStream *stream,
	GSList **list, G3DMaterial *materials)
{
	G3DObject *object;
	G3DFace *face;
	goffset off_save, off_sav2, off_sibl, off_chld, off_vert, off_prim, off_i;
	guint32 num_prims, colidx;
	gchar buffer[1025];
	gint32 i, j, x, y, z;

	while(!g3d_stream_eof(stream)) {
		/* signature */
		if(g3d_stream_read_int32_le(stream) != 1)
			return FALSE;

		object = g_new0(G3DObject, 1);
		*list = g_slist_append(*list, object);

		/* number of vertices */
		object->vertex_count = g3d_stream_read_int32_le(stream);
		object->vertex_data = g_new0(G3DFloat, 3 * object->vertex_count);

		/* number of primitives */
		num_prims = g3d_stream_read_int32_le(stream);

		/* offset of selection primitive */
		g3d_stream_read_int32_le(stream);

		/* translation from parent */
		x = g3d_stream_read_int32_le(stream);
		y = g3d_stream_read_int32_le(stream);
		z = g3d_stream_read_int32_le(stream);
		object->transformation = g_new0(G3DTransformation, 1);
		g3d_matrix_identity(object->transformation->matrix);
		g3d_matrix_translate(x, y, z, object->transformation->matrix);

		/* offset of object name */
		off_save = g3d_stream_tell(stream) + 4;
		g3d_stream_seek(stream, g3d_stream_read_int32_le(stream), G_SEEK_SET);
		g3d_stream_read_cstr(stream, buffer, 1024);
		buffer[1024] = '\0';
		object->name = g_strdup(buffer);
		g3d_stream_seek(stream, off_save, G_SEEK_SET);
#if DEBUG > 1
		g_debug("TA: object '%s'", object->name);
#endif

		/* always 0 */
		g3d_stream_read_int32_le(stream);

		/* offset of vertex array */
		off_vert = g3d_stream_read_int32_le(stream);
		off_save = g3d_stream_tell(stream);
		g3d_stream_seek(stream, off_vert, G_SEEK_SET);
		for(i = 0; i < object->vertex_count; i ++)
		{
			object->vertex_data[i * 3 + 0] = g3d_stream_read_int32_le(stream);
			object->vertex_data[i * 3 + 1] = g3d_stream_read_int32_le(stream);
			object->vertex_data[i * 3 + 2] = g3d_stream_read_int32_le(stream);
		}
		g3d_stream_seek(stream, off_save, G_SEEK_SET);

		/* offset of primitive array */
		off_prim = g3d_stream_read_int32_le(stream);
		off_save = g3d_stream_tell(stream);
		g3d_stream_seek(stream, off_prim, G_SEEK_SET);
		for(i = 0; i < num_prims; i ++)
		{
			face = g_new0(G3DFace, 1);

			/* color index */
			colidx = g3d_stream_read_int32_le(stream);
#if DEBUG > 2
			g_debug("TA: color index: %d", colidx);
#endif
			if(colidx > 255)
			{
				g_warning("TA: color index > 255 (%d)\n", colidx);
				g_free(face);
				g3d_stream_skip(stream, 28);
				continue;
			}
			face->material = &(materials[colidx]);

			/* number of vertices */
			face->vertex_count = g3d_stream_read_int32_le(stream);
			if(face->vertex_count < 3)
			{
				/* skip this primitive */
				g_free(face);
				g3d_stream_skip(stream, 24);
				continue;
			}
			face->vertex_indices = g_new0(guint32, face->vertex_count);

			/* always 0 */
			g3d_stream_read_int32_le(stream);

			/* offset of vertex index array */
			off_i = g3d_stream_read_int32_le(stream);
			off_sav2 = g3d_stream_tell(stream);
#if DEBUG > 2
			g_debug("TA: vertex index offset: 0x%08x", off_i);
#endif
			g3d_stream_seek(stream, off_i, G_SEEK_SET);
			for(j = 0; j < face->vertex_count; j ++)
				face->vertex_indices[j] = g3d_stream_read_int16_le(stream);
			g3d_stream_seek(stream, off_sav2, G_SEEK_SET);

			/* offset of texture name */
			g3d_stream_read_int32_le(stream);

			/* unknown */
			g3d_stream_skip(stream, 12);

			object->faces = g_slist_prepend(object->faces, face);
		}
		g3d_stream_seek(stream, off_save, G_SEEK_SET);

		/* offset of sibling object */
		off_sibl = g3d_stream_read_int32_le(stream);

		/* offset of child object */
		off_chld = g3d_stream_read_int32_le(stream);

#if DEBUG > 3
		g_debug("TA: child @ 0x%08x, sibling @ 0x%08x", off_chld, off_sibl);
#endif

		if(off_chld != 0)
		{
			off_save = g3d_stream_tell(stream);
			g3d_stream_seek(stream, off_chld, G_SEEK_SET);
			ta_3do_read_children(context, stream, &(object->objects),
				materials);
			g3d_stream_seek(stream, off_save, G_SEEK_SET);
		}

		if(off_sibl == 0)
			return TRUE;

		g3d_stream_seek(stream, off_sibl, G_SEEK_SET);
	}

	return FALSE;
}
