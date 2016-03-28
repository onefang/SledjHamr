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

	plugin by Martin Gerhardy <martin.gerhardy@gmail.com>
*/

#include <stdio.h>
#include <string.h>

#include <g3d/types.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/stream.h>
#include <g3d/iff.h>

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	G3DObject *object;
	gchar magic[16], texture[32];
	guint32 type, filesize;
	guint32 num_bones, num_meshs, num_frames;
	guint32 ofs_bones, ofs_meshs, ofs_frames;
	guint32 ofs_texcoords, ofs_verts, ofs_indices;
	goffset off_start;
	G3DImage *image = NULL;
	G3DMaterial *material, *mat;
	G3DFace *face;
	GSList *mitem;
	G3DFloat *normals;
	int i;

	off_start = g3d_stream_tell(stream);

	g3d_stream_read(stream, magic, 16);
	if(strcmp("DARKPLACESMODEL", magic)) {
		g_warning("DPM: Unknown magic id: '%s'\n", magic);
		return FALSE;
	}

	object = g_new0(G3DObject, 1);

	/* read the header information */
	type = g3d_stream_read_int32_be(stream);
	filesize = g3d_stream_read_int32_be(stream);
	/* skip some floats */
	for (i = 0; i < 8; i++)
		g3d_stream_read_float_le(stream);
	num_bones = g3d_stream_read_int32_be(stream);
	num_meshs = g3d_stream_read_int32_be(stream);
	num_frames = g3d_stream_read_int32_be(stream);
	ofs_bones = g3d_stream_read_int32_be(stream);
	ofs_meshs = g3d_stream_read_int32_be(stream);
	ofs_frames = g3d_stream_read_int32_be(stream);

	/* default material */
	material = g3d_material_new();
	material->name = g_strdup("default material");
	object->materials = g_slist_append(object->materials, material);

	num_meshs = 1; /* only load the first mesh */
	for (i = 0; i < num_meshs; i++) {
		guint32 nvertex, ntris;
		int j;

		g3d_stream_seek(stream, off_start + ofs_meshs, G_SEEK_SET);
		g3d_stream_read(stream, texture, 32);

		/* read texture image */
		if(strlen(texture) > 0) {
			image = g3d_texture_load_cached(context, model, texture);
			if(image == NULL) {
				/* try jpeg */
				char *strp = strrchr(texture, '.');
				if(strp) {
					strcpy(strp, ".jpg");
					image = g3d_texture_load_cached(context, model, texture);
				}
			}
		}

		if(image == NULL) {
			mitem = model->materials;
			while(mitem) {
				mat = (G3DMaterial *)mitem->data;
				if(strcmp(mat->name, object->name) == 0) {
					image = mat->tex_image;
					break;
				}
				mitem = mitem->next;
			}
		}

		nvertex = g3d_stream_read_int32_be(stream);
		ntris = g3d_stream_read_int32_be(stream);
		ofs_verts = g3d_stream_read_int32_be(stream);
		ofs_texcoords = g3d_stream_read_int32_be(stream);
		ofs_indices = g3d_stream_read_int32_be(stream);

		/* read vertex data */
		g3d_stream_seek(stream, off_start + ofs_verts, G_SEEK_SET);
		object->vertex_count = nvertex;
		object->vertex_data = g_new0(G3DFloat, nvertex * 3);
		normals = g_new0(G3DFloat, nvertex * 3);
		for(j = 0; j < nvertex; j++) {

		}

		/* read texture coords */
		g3d_stream_seek(stream, off_start + ofs_texcoords, G_SEEK_SET);
		object->tex_vertex_data = g_new0(G3DFloat, nvertex * 2);
		for(j = 0; j < nvertex; j++) {
			object->tex_vertex_data[j * 2 + 0] =
				g3d_stream_read_float_be(stream);
			object->tex_vertex_data[j * 2 + 1] =
				g3d_stream_read_float_be(stream);
		}

		/* read triangles */
		/* TODO */
	}

	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("Darkplaces engine models.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("dpm", ":", 0);
}

