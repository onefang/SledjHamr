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
#include <g3d/iff.h>
#include <g3d/stream.h>
#include <g3d/material.h>

#include "imp_iob.h"

gboolean iob_cb_xLSx(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	G3DMaterial *material;
	G3DFace *face;
	gint32 i, nitems;

	object = (G3DObject *)local->object;
	g_return_val_if_fail(object != NULL, FALSE);

	if((local->id & 0xFF) == '2')
	{
		nitems = g3d_stream_read_int32_be(global->stream);
		local->nb -= 4;
	}
	else
	{
		nitems = g3d_stream_read_int16_be(global->stream);
		local->nb -= 2;
	}

	for(i = 0; i < nitems; i ++)
	{
		/* TODO: find material by parameters, don't create too much
		 * materials */

		/* default material + nth */
		material = g_slist_nth_data(object->materials, i + 1);
		if(material == NULL)
		{
			material = g3d_material_new();
			material->flags |= G3D_FLAG_MAT_TWOSIDE;
			material->name = g_strdup_printf("per face material #%d", i);
			object->materials = g_slist_append(object->materials, material);

			/* assign to face */
			face = g_slist_nth_data(object->faces, i);
			if(face)
				face->material = material;
		}

		switch(local->id)
		{
			case G3D_IFF_MKID('C', 'L', 'S', 'T'):
			case G3D_IFF_MKID('C', 'L', 'S', '2'):
				material->r = (G3DFloat)g3d_stream_read_int8(global->stream) / 255.0;
				material->g = (G3DFloat)g3d_stream_read_int8(global->stream) / 255.0;
				material->b = (G3DFloat)g3d_stream_read_int8(global->stream) / 255.0;
				break;

			case G3D_IFF_MKID('R', 'L', 'S', 'T'):
			case G3D_IFF_MKID('R', 'L', 'S', '2'):
				material->specular[0] =
					(GLfloat)g3d_stream_read_int8(global->stream) / 255.0;
				material->specular[1] =
					(GLfloat)g3d_stream_read_int8(global->stream) / 255.0;
				material->specular[2] =
					(GLfloat)g3d_stream_read_int8(global->stream) / 255.0;
				break;

			case G3D_IFF_MKID('T', 'L', 'S', 'T'):
			case G3D_IFF_MKID('T', 'L', 'S', '2'):
				material->a = 1.0 - (
					(G3DFloat)g3d_stream_read_int8(global->stream) / 255.0 +
					(G3DFloat)g3d_stream_read_int8(global->stream) / 255.0 +
					(G3DFloat)g3d_stream_read_int8(global->stream) / 255.0) / 3.0;
				break;
		}

		local->nb -= 3;
	}

	return TRUE;
}

gboolean iob_cb_COLR(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	G3DMaterial *material;

	object = (G3DObject *)local->object;
	g_return_val_if_fail(object != NULL, FALSE);

	material = g_slist_nth_data(object->materials, 0);
	g_return_val_if_fail(material != NULL, FALSE);

	g3d_stream_read_int8(global->stream);
	material->r = (float)g3d_stream_read_int8(global->stream) / 255.0;
	material->g = (float)g3d_stream_read_int8(global->stream) / 255.0;
	material->b = (float)g3d_stream_read_int8(global->stream) / 255.0;
	local->nb -= 4;

	return TRUE;
}

gboolean iob_cb_DESC(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	G3DMaterial *material;

	if(local->finalize) return TRUE;

	object = g_new0(G3DObject, 1);
	global->model->objects = g_slist_append(global->model->objects, object);

	material = g3d_material_new();
	material->flags |= G3D_FLAG_MAT_TWOSIDE;
	material->name = g_strdup("(default material)");

	object->materials = g_slist_append(object->materials, material);

	local->object = object;

	return TRUE;
}

gboolean iob_cb_EDGx(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	gint32 i, nedges;
	gint32 *edges;

	object = (G3DObject *)local->object;
	g_return_val_if_fail(object != NULL, FALSE);

	if(local->id == G3D_IFF_MKID('E','D','G','E'))
	{
		nedges = g3d_stream_read_int16_be(global->stream);
		local->nb -= 2;
	}
	else
	{
		nedges = g3d_stream_read_int32_be(global->stream);
		local->nb -= 4;
	}

	edges = g_malloc(nedges * 2 * sizeof(gint32));
	for(i = 0; i < nedges; i ++)
	{
		if(local->id == G3D_IFF_MKID('E','D','G','E'))
		{
			edges[i * 2 + 0] = g3d_stream_read_int16_be(global->stream);
			edges[i * 2 + 1] = g3d_stream_read_int16_be(global->stream);
			local->nb -= 4;
		}
		else
		{
			edges[i * 2 + 0] = g3d_stream_read_int32_be(global->stream);
			edges[i * 2 + 1] = g3d_stream_read_int32_be(global->stream);
			local->nb -= 8;
		}
	}

	local->level_object = edges;

	return TRUE;
}

gboolean iob_cb_FACx(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	gint32 *edges, e[3], v1, v2, v3;
	gint32 i, nfaces;

	object = (G3DObject *)local->object;
	g_return_val_if_fail(object != NULL, FALSE);

	/* edges are read in EDGE/EDG2 chunk */
	edges = (gint32 *)local->level_object;
	g_return_val_if_fail(edges != NULL, FALSE);

	if(local->id == G3D_IFF_MKID('F','A','C','E'))
	{
		nfaces = g3d_stream_read_int16_be(global->stream);
		local->nb -= 2;
	}
	else
	{
		nfaces = g3d_stream_read_int32_be(global->stream);
		local->nb -= 4;
	}

	for(i = 0; i < nfaces; i ++)
	{
		G3DFace *face = g_new0(G3DFace, 1);
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);

		if(local->id == G3D_IFF_MKID('F','A','C','E'))
		{
			e[0] = g3d_stream_read_int16_be(global->stream);
			e[1] = g3d_stream_read_int16_be(global->stream);
			e[2] = g3d_stream_read_int16_be(global->stream);
			local->nb -= 6;
		}
		else
		{
			e[0] = g3d_stream_read_int32_be(global->stream);
			e[1] = g3d_stream_read_int32_be(global->stream);
			e[2] = g3d_stream_read_int32_be(global->stream);
			local->nb -= 12;
		}

		face->vertex_indices[0] = v1 = edges[e[0] * 2 + 0];
		face->vertex_indices[1] = v2 = edges[e[0] * 2 + 1];
		if((v1 != edges[e[1] * 2 + 0]) && (v2 != edges[e[1] * 2 + 0]))
			v3 = edges[e[1] * 2 + 0];
		else if((v1 != edges[e[1] * 2 + 1]) && (v2 != edges[e[1] * 2 + 1]))
			v3 = edges[e[1] * 2 + 1];
		else if((v1 != edges[e[2] * 2 + 0]) && (v2 != edges[e[2] * 2 + 0]))
			v3 = edges[e[2] * 2 + 0];
		else
			v3 = edges[e[2] * 2 + 1];

		face->vertex_indices[2] = v3;

		face->material = g_slist_nth_data(object->materials, 0);
		object->faces = g_slist_append(object->faces, face);
	}

	/* free edges now */
	g_free(edges);
	local->level_object = NULL;

	return TRUE;
}

gboolean iob_cb_NAME(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	gchar buffer[512];

	object = (G3DObject *)local->object;
	g_return_val_if_fail(object != NULL, FALSE);

	g3d_stream_read(global->stream, buffer, local->nb);

	object->name = g_convert(buffer, local->nb,
		"UTF-8", "ISO-8859-1",
		NULL, NULL, NULL);

	local->nb = 0;

	return TRUE;
}

gboolean iob_cb_PNTx(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	gint32 i;

	object = (G3DObject *)local->object;
	g_return_val_if_fail(object != NULL, FALSE);

	if(local->id == G3D_IFF_MKID('P','N','T','S'))
	{
		object->vertex_count = g3d_stream_read_int16_be(global->stream);
		local->nb -= 2;
	}
	else
	{
		object->vertex_count = g3d_stream_read_int32_be(global->stream);
		local->nb -= 4;
	}

	object->vertex_data = g_new0(G3DFloat, object->vertex_count * 3);

	for(i = 0; i < object->vertex_count; i ++)
	{
		object->vertex_data[i * 3 + 0] = iob_read_fract(global->stream);
		object->vertex_data[i * 3 + 1] = iob_read_fract(global->stream);
		object->vertex_data[i * 3 + 2] = iob_read_fract(global->stream);
		local->nb -= 12;
	}

	return TRUE;
}

gboolean iob_cb_REFL(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	G3DMaterial *material;
	gint32 i;

	object = (G3DObject *)local->object;
	g_return_val_if_fail(object != NULL, FALSE);

	material = g_slist_nth_data(object->materials, 0);
	g_return_val_if_fail(material != NULL, FALSE);

	g3d_stream_read_int8(global->stream);
	for(i = 0; i < 3; i ++)
		material->specular[i] =
			(GLfloat)g3d_stream_read_int8(global->stream) / 255.0;
	local->nb -= 4;

	return TRUE;
}

gboolean iob_cb_TRAN(G3DIffGlobal *global, G3DIffLocal *local)
{
	G3DObject *object;
	G3DMaterial *material;

	object = (G3DObject *)local->object;
	g_return_val_if_fail(object != NULL, FALSE);

	material = g_slist_nth_data(object->materials, 0);
	g_return_val_if_fail(material != NULL, FALSE);

	g3d_stream_read_int8(global->stream);
	material->a = 1.0 - (
		(G3DFloat)g3d_stream_read_int8(global->stream) / 255.0 +
		(G3DFloat)g3d_stream_read_int8(global->stream) / 255.0 +
		(G3DFloat)g3d_stream_read_int8(global->stream) / 255.0) / 3.0;
	local->nb -= 4;

	return TRUE;
}

