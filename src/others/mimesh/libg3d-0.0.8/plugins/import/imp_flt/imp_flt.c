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

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/model.h>
#include <g3d/matrix.h>

#include "imp_flt_opcodes.h"

FltOpcode *flt_opcode_info(guint32 opcode);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	guint16 opcode, rlen;
	FltOpcode *oi;
	FltGlobalData *gd;
	FltLocalData *ld;
	G3DObject *g3dobj = NULL;
	gpointer level_object = NULL;
	gchar *pad;
	G3DFloat prev_pcnt = 0.0, pcnt;
	G3DMatrix rmatrix[16];

	gd = g_new0(FltGlobalData, 1);
	gd->context = context;
	gd->model = model;
	gd->stream = stream;
	gd->oqueue = g_queue_new();

	while(!g3d_stream_eof(stream)) {
		/* get record information */
		opcode = g3d_stream_read_int16_be(stream);
		rlen = g3d_stream_read_int16_be(stream);

		/* create local data */
		ld = g_new0(FltLocalData, 1);
		ld->opcode = opcode;
		ld->nb = rlen - 4;
		ld->g3dobj = g3dobj;
		ld->level_object = level_object;

		if(opcode == 0) {
			/* end of file or error */
			break;
		}

		oi = flt_opcode_info(opcode);

		pad = g_strnfill(gd->level, ' ');
		g_debug("\\%s[%04d][%c] %8d: %s", pad, opcode,
			oi ? (oi->callback ? 'f' : ' ') : ' ',
			rlen,
			oi ? oi->description : "unknown");
		g_free(pad);

		if(oi && oi->callback)
			oi->callback(gd, ld);

		/* skip remaining bytes */
		if(ld->nb > 0)
			g3d_stream_skip(stream, ld->nb);

		/* free local data */
		g3dobj = ld->g3dobj;
		level_object = ld->level_object;
		g_free(ld);

		/* update caller */
		pcnt = (G3DFloat)g3d_stream_tell(stream) /
			(G3DFloat)g3d_stream_size(stream);
		if((pcnt - prev_pcnt) > 0.002) {
			prev_pcnt = pcnt;
			g3d_context_update_progress_bar(context, pcnt, TRUE);
		}
		g3d_context_update_interface(context);
	}

	g_queue_free(gd->oqueue);
	if(gd->vertex_palette) {
		g_free(gd->vertex_palette->offsets);
		g_free(gd->vertex_palette->flags);
		g_free(gd->vertex_palette->vertex_data);
		g_free(gd->vertex_palette->normal_data);
		g_free(gd->vertex_palette->tex_vertex_data);
		g_free(gd->vertex_palette->vertex_materials);
		g_free(gd->vertex_palette);
	}
	if(gd->texture_palette) {
		g_free(gd->texture_palette->offsets);
		g_free(gd->texture_palette->textures);
		g_free(gd->texture_palette);
	}
	g_free(gd);

	g3d_matrix_identity(rmatrix);
	g3d_matrix_rotate_xyz(G_PI * -90.0 / 180, 0.0, 0.0, rmatrix);
	g3d_model_transform(model, rmatrix);

	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("OpenFlight models.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("flt", ":", 0);
}

/*
 * FLT specific
 */

FltOpcode *flt_opcode_info(guint32 opcode)
{
	guint32 i;

	for(i = 0; flt_opcodes[i].opcode != 0; i ++)
		if(flt_opcodes[i].opcode == opcode)
			return &(flt_opcodes[i]);
	return NULL;
}

guint32 flt_read_typed_index(G3DStream *stream, guint32 type, gint32 *len)
{
	guint32 val = 0;

	switch(type) {
		case 1:
			val = g3d_stream_read_int8(stream);
			*len -= 1;
			break;
		case 2:
			val = g3d_stream_read_int16_be(stream);
			*len -= 2;
			break;
		case 4:
			val = g3d_stream_read_int32_be(stream);
			*len -= 4;
			break;
		default:
			g_warning("FLT: unknown index type %d\n", type);
			break;
	}
	return val;
}

