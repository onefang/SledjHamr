/*
    libg3d - 3D object loading library

    Copyright (C) 2006  Oliver Dippel <o.dippel@gmx.de>
	              2008  Markus Dahms <mad@automagically.de>

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
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/material.h>

/*
	Infos for the STL(A)-Format:
	http://www.csit.fsu.edu/~burkardt/data/stla/stla.html
	Infos for the STL(B)-Format:
	http://www.csit.fsu.edu/~burkardt/data/stlb/stlb.html
*/

#define STL_ASCII  0
#define STL_BINARY 1

static gboolean stl_load_binary(G3DContext *context, G3DModel *model,
	G3DStream *stream);
static gboolean stl_load_text(G3DContext *context, G3DModel *model,
	G3DStream *stream);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	gchar line[1024];
	guint32 type;

	/* Check Filetype (ASCII or BINARY) */
	type = STL_BINARY;
	while(!g3d_stream_eof(stream)) {
		if(!g3d_stream_read_line(stream, line, 1023))
			break;
		if(strstr(line, "solid")) {
			setlocale(LC_NUMERIC, "C");
			type = STL_ASCII;
			break;
		}
	}
	/* rewind */
	g3d_stream_seek(stream, 0, G_SEEK_SET);

	if (type == STL_BINARY)
		return stl_load_binary(context, model, stream);
	else
		return stl_load_text(context, model, stream);
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("STLA and STLB stereolithography models.\n"
		"Author: Oliver Dippel");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("stl:stla:stlb", ":", 0);
}

/*****************************************************************************/

static gboolean stl_load_binary(G3DContext *context, G3DModel *model,
	G3DStream *stream)
{
	G3DObject *object;
	G3DMaterial *material;
	G3DFace *face;
	gchar name[81];
	guint32 num_faces, index = 0;
	gint32 n, i, j;

#if DEBUG > 0
	g_debug("STL: format is BINARY");
#endif

	g3d_stream_read(stream, name, 80);
	name[80] = 0;
	num_faces = g3d_stream_read_int32_le(stream);

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("STL-Model");
	model->objects = g_slist_append(model->objects, object);

	material = g3d_material_new();
	material->name = g_strdup("default material");
	object->materials = g_slist_append(object->materials, material);

	object->vertex_count = num_faces * 3;
#if DEBUG > 2
	g_debug("STL: BINARY: vertex_count: %i", object->vertex_count);
#endif
	object->vertex_data = g_new0(G3DFloat, object->vertex_count * 3);
	for(n = 0; n < num_faces; n ++) {
		face = g_new0(G3DFace, 1);
		face->material = material;
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, face->vertex_count);
		face->vertex_indices[0] = index + 0;
		face->vertex_indices[1] = index + 1;
		face->vertex_indices[2] = index + 2;
		object->faces = g_slist_prepend(object->faces, face);
		/* normal */
		for(j = 0; j < 3; j ++)
			g3d_stream_read_float_le(stream);
		/* triangle */
		for(i = 0; i < 3; i ++) {
			for(j = 0; j < 3; j ++)
				object->vertex_data[index * 3 + j] =
					g3d_stream_read_float_le(stream);
			index ++;
		}
		/* 2 Byte Dummy read */
		g3d_stream_read_int16_le(stream);
	}
	return TRUE;
}

static gboolean stl_load_text(G3DContext *context, G3DModel *model,
	G3DStream *stream)
{
	G3DObject *object;
	G3DMaterial *material;
	G3DFace *face;
	gchar line[1024];
	guint32 index = 0;
	G3DFloat x, y, z;

#if DEBUG > 0
	g_debug("STL: format is ASCII");
#endif

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("STL-Model");
	model->objects = g_slist_append(model->objects, object);
	material = g3d_material_new();
	material->name = g_strdup("default material");
	object->materials = g_slist_append(object->materials, material);
	object->vertex_count = 0;
	while(!g3d_stream_eof(stream)) {
		line[0] = 0;
		if(!g3d_stream_read_line(stream, line, 1023))
			break;
		g_strstrip(line);
		if(strncmp(line, "solid", 5) == 0) {
			g_free(object->name);
			object->name = g_strdup(line + 6);
		} else if(strncmp(line, "vertex", 6) == 0) {
			object->vertex_count ++;
			object->vertex_data = g_realloc(object->vertex_data,
				object->vertex_count * 3 * sizeof(G3DFloat));
			if(sscanf(line + 7, G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &x, &y, &z) == 3) {
				object->vertex_data[(object->vertex_count - 1) * 3 + 0] = x;
				object->vertex_data[(object->vertex_count - 1) * 3 + 1] = y;
				object->vertex_data[(object->vertex_count - 1) * 3 + 2] = z;
			} else {
#if DEBUG > 0
				g_debug("imp_stl: parse error in vertex line: %s", line);
#endif
			}
		} else if(strncmp(line, "facet", 5) == 0) {
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 3;
			face->vertex_indices = g_new0(guint32, face->vertex_count);
			face->vertex_indices[0] = index + 0;
			face->vertex_indices[1] = index + 1;
			face->vertex_indices[2] = index + 2;
			object->faces = g_slist_prepend(object->faces, face);
			index += 3;
		}
	}
	return TRUE;
}

