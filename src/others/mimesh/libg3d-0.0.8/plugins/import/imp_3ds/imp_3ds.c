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
#include <stdarg.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/matrix.h>

#include "imp_3ds.h"
#include "imp_3ds_chunks.h"

/*****************************************************************************/
/* plugin interface                                                          */
/*****************************************************************************/

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer plugin_data)
{
	G3DMatrix rmatrix[16];
	gint32 nbytes, magic;
	gboolean retval;
	x3ds_global_data global;
	x3ds_parent_data *parent;

	magic = g3d_stream_read_int16_le(stream);
	if((magic != 0x4D4D) && (magic != 0xC23D))
	{
		g_warning("file %s is not a 3ds file", stream->uri);
		return FALSE;
	}
	nbytes = g3d_stream_read_int32_le(stream);
	nbytes -= 6;
#if DEBUG > 0
	g_debug("\\[%4.4X] 3DS file: main length: %d", magic, nbytes);
#endif

	global.context = context;
	global.model = model;
	global.stream = stream;
	global.scale = 1.0;
	global.max_tex_id = 0;

	parent = g_new0(x3ds_parent_data, 1);
	parent->id = magic;
	parent->nb = nbytes;

	retval = x3ds_read_ctnr(&global, parent);

	g3d_matrix_identity(rmatrix);
	g3d_matrix_rotate_xyz(G_PI * -90.0 / 180, 0.0, 0.0, rmatrix);
	g3d_model_transform(model, rmatrix);

	g_free(parent);

	return retval;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("AutoCAD 3D Studio models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("3ds:prj", ":", 0);
}

/*****************************************************************************/

gboolean x3ds_read_ctnr(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 chunk_id, chunk_len, i;
	x3ds_parent_data *subparent;
	gpointer level_object;
	gchar *padding = "                                   ";

	level_object = NULL;

	while(parent->nb > 0) {
		chunk_id  = g3d_stream_read_int16_le(global->stream);
		chunk_len = g3d_stream_read_int32_le(global->stream);
		parent->nb -= 6;
		chunk_len -= 6;

		i = 0;
		while((x3ds_chunks[i].id != 0) && (x3ds_chunks[i].id != chunk_id))
			i ++;

		if(x3ds_chunks[i].id == chunk_id) {
			g_debug("\\%s(%d)[0x%04X][%c%c] %s (%d bytes)",
				padding + (strlen(padding) - parent->level), parent->level,
				chunk_id,
				x3ds_chunks[i].container ? 'c' : ' ',
				x3ds_chunks[i].callback ? 'f' : ' ',
				x3ds_chunks[i].desc, chunk_len);
			if (chunk_id==0) {
				g_warning("error: bad chunk id");
				return FALSE;
			}

			subparent = g_new0(x3ds_parent_data, 1);
			subparent->id = parent->id;
			subparent->object = parent->object;
			subparent->level = parent->level + 1;
			subparent->level_object = level_object;
			subparent->nb = chunk_len;

			if(x3ds_chunks[i].callback) {
				/* callback may change "nb" and "object" of
				 * "subparent" structure for following container run */

				x3ds_chunks[i].callback(global, subparent);
			}

			subparent->id = chunk_id;

			if(x3ds_chunks[i].container) {
				if(x3ds_read_ctnr(global, subparent) == FALSE) {
					/* abort on error */
					return FALSE;
				}
			}

			if(subparent->nb)
				g3d_stream_skip(global->stream, subparent->nb);

			level_object = subparent->level_object;

			g_free(subparent);
		} else {
			g_warning("[3DS] unknown chunk type 0x%04X", chunk_id);
			g3d_stream_skip(global->stream, chunk_len);
		}
		parent->nb -= chunk_len;

		/* update progress bar */
		x3ds_update_progress(global, parent->level);
	}

	return TRUE;
}

void x3ds_update_progress(x3ds_global_data *global, guint32 level)
{
	goffset fpos;

	/* update progress bar */
	if(level < 4) {
		fpos = g3d_stream_tell(global->stream);
		g3d_context_update_progress_bar(global->context,
			((G3DFloat)fpos / (G3DFloat)g3d_stream_size(global->stream)), TRUE);
	}
}

gint32 x3ds_read_cstr(G3DStream *stream, gchar *string)
{
	gint32 n = 0;
	char c;
	do {
		c = g3d_stream_read_int8(stream);
		string[n] = c;
		n++;
	} while(c != 0);
	return n;
}

G3DObject *x3ds_newobject(G3DModel *model, const gchar *name)
{
	G3DObject *object = g_malloc0(sizeof(G3DObject));
	G3DMaterial *material = g3d_material_new();

	object->name = g_strdup(name);
	object->faces = NULL;
	model->objects = g_slist_append(model->objects, object);
	object->materials = g_slist_append(object->materials, material);

	return object;
}

