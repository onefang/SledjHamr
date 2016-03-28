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
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/stream.h>
#include <g3d/iff.h>

#include "imp_md2_normals.h"

#define MD2_SKINNAMELEN 64

static const char *textureExtensions[] = {
	".pcx", ".bmp", ".jpg", ".tga", ".png", NULL };

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	guint32 idid, idver, skinwidth, skinheight, framesize;
	guint32 numskins, numverts, numtexs, numfaces, numglcmds, numframes;
	guint32 offskins, offtexs, offfaces, offframes, offglcmds, offend;
	G3DFloat *texco = NULL, *normals;
	gchar **skinnames = NULL;
	gint i, j;
	G3DObject *object;
	G3DMaterial *material;
	G3DImage *image = NULL;

	idid = g3d_stream_read_int32_be(stream);
	if(idid != G3D_IFF_MKID('I','D','P','2')) {
		g_critical("file '%s' is not a .md2 file", stream->uri);
		return FALSE;
	}

	idver = g3d_stream_read_int32_le(stream);
	if(idver != 8) {
		g_warning("file '%s' has wrong version (%d)", stream->uri, idver);
#define CLOSE_ON_WRONG_VERSION
#ifdef CLOSE_ON_WRONG_VERSION
		return FALSE;
#endif
	}

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("Q2Object");
	material = g3d_material_new();
	object->materials = g_slist_append(object->materials, material);
	model->objects = g_slist_append(model->objects, object);

	skinwidth  = g3d_stream_read_int32_le(stream);
	skinheight = g3d_stream_read_int32_le(stream);
	framesize  = g3d_stream_read_int32_le(stream);
	numskins   = g3d_stream_read_int32_le(stream);
	numverts   = g3d_stream_read_int32_le(stream);
	numtexs    = g3d_stream_read_int32_le(stream);
	numfaces   = g3d_stream_read_int32_le(stream);
	numglcmds  = g3d_stream_read_int32_le(stream);
	numframes  = g3d_stream_read_int32_le(stream);

	object->vertex_count = numverts;
	object->vertex_data = g_new0(G3DFloat, numverts * 3);
	normals = g_new0(G3DFloat, numverts * 3);

	offskins   = g3d_stream_read_int32_le(stream);
	offtexs    = g3d_stream_read_int32_le(stream);
	offfaces   = g3d_stream_read_int32_le(stream);
	offframes  = g3d_stream_read_int32_le(stream);
	offglcmds  = g3d_stream_read_int32_le(stream);
	offend     = g3d_stream_read_int32_le(stream);

	if(numskins > 0) {
		skinnames = g_new0(gchar *, numskins);
		for(i = 0; i < numskins; i ++) {
			skinnames[i] = g_new0(gchar, MD2_SKINNAMELEN);
			g3d_stream_read(stream, skinnames[i], MD2_SKINNAMELEN);

			/* some md2 models have a dot as first character to tell the engine
			 * load the texture from the dir where the model is located */
			if(skinnames[i][0] == '.')
				memmove(skinnames[i], skinnames[i] + 1, MD2_SKINNAMELEN - 1);
#if DEBUG > 0
			g_debug("skin #%d: %s", i + 1, skinnames[i]);
#endif
		}

		/* not every skin has a texture assigned, the engines will search
		 * a list of supported images to get the texture */
		for(j = 0; j < numskins; j++) {
			gchar skinname[MD2_SKINNAMELEN];
			gchar *basename;

			/* real filename */
			if(g_file_test(skinnames[j], G_FILE_TEST_EXISTS))
				image = g3d_texture_load_cached(context, model, skinnames[j]);
			if(image)
				break;
			basename = g_path_get_basename(skinnames[j]);
			if(g_file_test(basename, G_FILE_TEST_EXISTS))
				image = g3d_texture_load_cached(context, model, skinnames[j]);
			g_free(basename);
			if(image)
				break;

			/* without extension */
			for(i = 0; textureExtensions[i] != NULL; i ++) {
				g_snprintf(skinname, sizeof(skinname), "%s%s", skinnames[j],
					textureExtensions[i]);
				if(g_file_test(skinname, G_FILE_TEST_EXISTS))
					image = g3d_texture_load_cached(context, model, skinname);
				if(image)
					break;
				basename = g_path_get_basename(skinname);
				if(g_file_test(basename, G_FILE_TEST_EXISTS))
					image = g3d_texture_load_cached(context, model, skinname);
				g_free(basename);
				if(image)
					break;
			}

			/* replace extension */
			for(i = 0; textureExtensions[i] != NULL; i ++) {
				g_snprintf(skinname, sizeof(skinname), "%.*s%s",
					((int) strlen(skinnames[j])) - 4, skinnames[j],
					textureExtensions[i]);
				if(g_file_test(skinname, G_FILE_TEST_EXISTS))
					image = g3d_texture_load_cached(context, model, skinname);
				if(image)
					break;
				basename = g_path_get_basename(skinname);
				if(g_file_test(basename, G_FILE_TEST_EXISTS))
					image = g3d_texture_load_cached(context, model, skinname);
				g_free(basename);
				if(image)
					break;
			}
			if(image)
				break;
		}

		/* fallback skin name */
		if(image == NULL)
			image = g3d_texture_load_cached(context, model, "tris0.bmp");
		if(image)
			image->tex_env = G3D_TEXENV_REPLACE;
	}

	g3d_stream_seek(stream, offframes, G_SEEK_SET);
	/* vertices per frame */
#if DEBUG > 0
	g_debug("numframes: %d", numframes);
#endif
	for(i = 0; i < numframes; i ++) {
		G3DFloat s0,s1,s2, t0,t1,t2;
		gchar fname[16];
		guint32 j;

		s0 = g3d_stream_read_float_le(stream); /* scale */
		s1 = g3d_stream_read_float_le(stream);
		s2 = g3d_stream_read_float_le(stream);
		t0 = g3d_stream_read_float_le(stream); /* translate */
		t1 = g3d_stream_read_float_le(stream);
		t2 = g3d_stream_read_float_le(stream);
		g3d_stream_read(stream, fname, 16); /* frame name*/

		for(j = 0; j < numverts; j ++) {
			G3DFloat x,y,z;
			guint32 v,n;

			v = g3d_stream_read_int8(stream);
			x = (G3DFloat)v * s0 + t0;
			v = g3d_stream_read_int8(stream);
			y = (G3DFloat)v * s1 + t1;
			v = g3d_stream_read_int8(stream);
			z = (G3DFloat)v * s2 + t2;
			n = g3d_stream_read_int8(stream);
			if(i == 0) {
				object->vertex_data[j * 3 + 0] = x;
				object->vertex_data[j * 3 + 1] = y;
				object->vertex_data[j * 3 + 2] = z;

				normals[j * 3 + 0] = md2_normals[n * 3 + 0];
				normals[j * 3 + 1] = md2_normals[n * 3 + 1];
				normals[j * 3 + 2] = md2_normals[n * 3 + 2];
			}
		}
	}

	g3d_stream_seek(stream, offtexs, G_SEEK_SET);
	/* texture coordinates */
	if(numtexs > 0) {
		texco = g_new0(G3DFloat, numtexs * 2);
		for(i = 0; i < numtexs; i ++) {
			texco[i * 2 + 0] = g3d_stream_read_int16_le(stream) /
				(G3DFloat)skinwidth;
			texco[i * 2 + 1] = g3d_stream_read_int16_le(stream) /
				(G3DFloat)skinheight;
		}
	}

	/* faces */
	for(i = 0; i < numfaces; i ++) {
		G3DFace *face;
		guint32 i;
		guint16 index;

		face = g_new0(G3DFace, 1);
		object->faces = g_slist_append(object->faces, face);
		face->material = material;
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		face->tex_vertex_data = g_new0(G3DFloat, 3 * 2);
		face->normals = g_new0(G3DFloat, 3 * 3);
		face->flags |= G3D_FLAG_FAC_NORMALS;

		if(image)
		{
			face->flags |= G3D_FLAG_FAC_TEXMAP;
			face->tex_image = image;
		}

		for(i = 0; i < 3; i ++)
		{
			face->vertex_indices[i] = g3d_stream_read_int16_le(stream);
			face->normals[i * 3 + 0] =
				- normals[face->vertex_indices[i] * 3 + 0];
			face->normals[i * 3 + 1] =
				- normals[face->vertex_indices[i] * 3 + 1];
			face->normals[i * 3 + 2] =
				- normals[face->vertex_indices[i] * 3 + 2];
		}

		for(i = 0; i < 3; i ++)
		{
			index = g3d_stream_read_int16_le(stream);
			face->tex_vertex_data[i * 2 + 0] = texco[index * 2 + 0];
			face->tex_vertex_data[i * 2 + 1] = texco[index * 2 + 1];
		}
	}


	/* free skin names */
	if(skinnames)
	{
		for(i = 0; i < numskins; i ++)
			g_free(skinnames[i]);
		g_free(skinnames);
	}

	if(texco)
		g_free(texco);
	if(normals)
		g_free(normals);

	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("ID Software's Quake II models.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("md2", ":", 0);
}

