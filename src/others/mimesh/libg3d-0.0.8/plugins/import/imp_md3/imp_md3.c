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

#include <string.h>
#include <math.h>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

#include <g3d/types.h>
#include <g3d/object.h>
#include <g3d/stream.h>
#include <g3d/iff.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/vector.h>

#define MD3_TYPE_MD3 0x01
#define MD3_TYPE_MDC 0x02

gboolean md3_load_skin(G3DContext *context, G3DModel *model,
	const gchar *filename);
gboolean md3_read_tag(G3DStream *stream, G3DContext *context, G3DModel *model);
gboolean md3_read_mesh(G3DStream *stream, G3DContext *context, G3DModel *model);


EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	guint32 magic, version, nboneframes, ntags, nmeshes, nskins;
	guint32 off_bfs, off_tags, off_meshes, filesize, i, flags;

	magic = g3d_stream_read_int32_be(stream);
	if((magic != G3D_IFF_MKID('I', 'D', 'P', '3')) &&
		(magic != G3D_IFF_MKID('I', 'D', 'P', 'C'))) {
		g_warning("MD3: %s is not a valid md3 file", stream->uri);
		return FALSE;
	}

	version = g3d_stream_read_int32_le(stream);
	g3d_stream_skip(stream, 64);

	flags = g3d_stream_read_int32_le(stream);
	nboneframes = g3d_stream_read_int32_le(stream);
	ntags = g3d_stream_read_int32_le(stream);
	nmeshes = g3d_stream_read_int32_le(stream);
	nskins = g3d_stream_read_int32_le(stream);
	off_bfs = g3d_stream_read_int32_le(stream);
	off_tags = g3d_stream_read_int32_le(stream);
	off_meshes = g3d_stream_read_int32_le(stream);
	filesize = g3d_stream_read_int32_le(stream);

	/* try to load skin */
	md3_load_skin(context, model, stream->uri);

	g_debug("MD3: version: %u, file size: %u bytes", version, filesize);
	g_debug("MD3: tags @ 0x%08x, meshes @ 0x%08x", off_tags, off_meshes);

	g3d_stream_seek(stream, off_tags, G_SEEK_SET);
	if(magic == G3D_IFF_MKID('I', 'D', 'P', '3'))
	for(i = 0; i < nboneframes * ntags; i ++)
		md3_read_tag(stream, context, model);

	/* read meshes */
	g3d_stream_seek(stream, off_meshes, G_SEEK_SET);
	for(i = 0; i < nmeshes; i ++)
		md3_read_mesh(stream, context, model);

	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("Quake 3 models.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("md3:mdc", ":", 0);
}

/*
 * MD3 specific
 */

gboolean md3_load_skin(G3DContext *context, G3DModel *model,
	const gchar *filename)
{
	gchar *basename, *skinname, **parts;
	gchar line[256];
	G3DStream *stream;
	G3DMaterial *material;

	basename = g_path_get_basename(filename);
	skinname = g_strdup_printf("%.*s_default.skin",
		((int) strlen(basename)) - 4, basename);

	g_debug("MD3: trying to open skin file %s", skinname);

	stream = g3d_stream_open_file(skinname, "r");

	g_free(basename);
	g_free(skinname);

	/* no skin */
	if(stream == NULL)
		return FALSE;

	while(g3d_stream_read_line(stream, line, 255) != NULL) {
		parts = g_strsplit(line, ",", 2);
		if(parts[0] && parts[1]) {
			g_strchomp(parts[1]);
			if(strlen(parts[1]) > 0)
			{
				g_debug("MD3: skin texture for %s: %s",
					parts[0], parts[1]);

				material = g3d_material_new();
				material->name = g_strdup(parts[0]);
				material->tex_image = g3d_texture_load_cached(context, model,
					parts[1]);

				model->materials = g_slist_append(model->materials,
					material);
			}
		}
		g_strfreev(parts);
	}

	g3d_stream_close(stream);

	return TRUE;
}

gboolean md3_read_tag(G3DStream *stream, G3DContext *context, G3DModel *model)
{
	gchar name[65];

	g3d_stream_read(stream, name, 64);
	name[64] = '\0';

	g_debug("MD3: tag: %s", name);

	/* position */
	g3d_stream_read_float_le(stream);
	g3d_stream_read_float_le(stream);
	g3d_stream_read_float_le(stream);

	/* rotation */
	g3d_stream_read_float_le(stream);
	g3d_stream_read_float_le(stream);
	g3d_stream_read_float_le(stream);

	g3d_stream_read_float_le(stream);
	g3d_stream_read_float_le(stream);
	g3d_stream_read_float_le(stream);

	g3d_stream_read_float_le(stream);
	g3d_stream_read_float_le(stream);
	g3d_stream_read_float_le(stream);

	return TRUE;
}

gboolean md3_read_mesh(G3DStream *stream, G3DContext *context, G3DModel *model)
{
	G3DObject *object;
	G3DImage *image = NULL;
	G3DMaterial *material, *mat;
	G3DFace *face;
	GSList *mitem;
	guint32 magic, i, j;
	guint8 type = 0, r, s;
	G3DFloat rho, sigma, *normals;
	gchar name[64], *strp;
	guint32 nmeshframe, nskin, nvertex, ntris, mlength, flags;
	goffset off_tris, off_texvec, off_vertex, off_start, off_skins;

	off_start = g3d_stream_tell(stream);

	magic = g3d_stream_read_int32_be(stream);

	if(magic == G3D_IFF_MKID('I', 'D', 'P', '3'))
		type = MD3_TYPE_MD3;
	else /* if(magic == 0x07000000)*/
		type = MD3_TYPE_MDC;
#if 0
	else
	{
		g_warning("MD3: mesh magic unknown (%02x%02x%02x%02x)\n",
			(magic >> 24) & 0xFF,
			(magic >> 16) & 0xFF,
			(magic >> 8) & 0xFF,
			magic & 0xFF);
		return FALSE;
	}
#endif

	object = g_new0(G3DObject, 1);

	/* read name */
	g3d_stream_read(stream, name, 64);
	object->name = g_strndup(name, 64);

	flags = g3d_stream_read_int32_le(stream);

	if(type == MD3_TYPE_MD3) {
		nmeshframe = g3d_stream_read_int32_le(stream);
		nskin = g3d_stream_read_int32_le(stream);
	} else if(type == MD3_TYPE_MDC) {
		g3d_stream_read_int32_le(stream); /* ncompframes */
		g3d_stream_read_int32_le(stream); /* nbaseframes */
		g3d_stream_read_int32_le(stream); /* nshaders */
	}

	nvertex = g3d_stream_read_int32_le(stream);
	ntris = g3d_stream_read_int32_le(stream);

	off_tris = g3d_stream_read_int32_le(stream);
	off_skins = g3d_stream_read_int32_le(stream);

	off_texvec = g3d_stream_read_int32_le(stream);
	off_vertex = g3d_stream_read_int32_le(stream);

	if(type == MD3_TYPE_MDC) {
		g3d_stream_read_int32_le(stream); /* off_compvert */
		g3d_stream_read_int32_le(stream); /* off_fbasef */
		g3d_stream_read_int32_le(stream); /* off_fcompf */
	}

	mlength = g3d_stream_read_int32_le(stream);

	if((nvertex == 0) || (ntris == 0)) {
		g_warning("MD3: %u vertices, %u triangles", nvertex, ntris);
		g3d_stream_seek(stream, off_start + mlength, G_SEEK_SET);
		return FALSE;
	}

	/* default material */
	material = g3d_material_new();
	material->name = g_strdup("default material");
	object->materials = g_slist_append(object->materials, material);

	/* skins */
	g3d_stream_seek(stream, off_start + off_skins, G_SEEK_SET);
	g3d_stream_read(stream, name, 64);
	g_debug("MD3: skin name: %s", name);

	/* read texture image */
	if(strlen(name) > 0) {
		image = g3d_texture_load_cached(context, model, name);
		if(image == NULL) {
			/* try jpeg */
			strp = strrchr(name, '.');
			if(strp) {
				strcpy(strp, ".jpg");
				image = g3d_texture_load_cached(context, model, name);
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

	/* read vertex data */
	g3d_stream_seek(stream, off_start + off_vertex, G_SEEK_SET);
	object->vertex_count = nvertex;
	object->vertex_data = g_new0(G3DFloat, nvertex * 3);
	normals = g_new0(G3DFloat, nvertex * 3);
	for(i = 0; i < nvertex; i ++) {
		gint16 d;

		d = g3d_stream_read_int16_le(stream);
		object->vertex_data[i * 3 + 0] = d;
		d = g3d_stream_read_int16_le(stream);
		object->vertex_data[i * 3 + 1] = d;
		d = g3d_stream_read_int16_le(stream);
		object->vertex_data[i * 3 + 2] = d;

		/* compressed normal */
		/* FIXME: the normals don't look right... */
		r = g3d_stream_read_int8(stream); /* rho */
		s = g3d_stream_read_int8(stream); /* sigma */
		rho = r * 2 * M_PI / 256.0;
		sigma = s * 2 * M_PI / 256.0;

		normals[i * 3 + 0] = - cos(sigma) * sin(rho);
		normals[i * 3 + 1] = - cos(sigma) * sin(rho);
		normals[i * 3 + 2] = - cos(rho);

		g3d_vector_unify(
			&(normals[i * 3 + 0]),
			&(normals[i * 3 + 1]),
			&(normals[i * 3 + 2]));
	}

	/* read texture vertex data */
	g3d_stream_seek(stream, off_start + off_texvec, G_SEEK_SET);
	object->tex_vertex_data = g_new0(G3DFloat, nvertex * 2);
	for(i = 0; i < nvertex; i ++) {
		object->tex_vertex_data[i * 2 + 0] = g3d_stream_read_float_le(stream);
		object->tex_vertex_data[i * 2 + 1] = g3d_stream_read_float_le(stream);
	}

	/* read triangles */
	g3d_stream_seek(stream, off_start + off_tris, G_SEEK_SET);
	for(i = 0; i < ntris; i ++)
	{
		face = g_new0(G3DFace, 1);
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		face->material = material;

		face->flags |= G3D_FLAG_FAC_NORMALS;
		face->normals = g_new0(G3DFloat, 3 * 3);

		face->tex_image = image;
		if(face->tex_image)
		{
			face->tex_vertex_data = g_new0(G3DFloat, 3 * 2);
			face->flags |= G3D_FLAG_FAC_TEXMAP;
		}

		for(j = 0; j < 3; j ++)
		{
			face->vertex_indices[j] = g3d_stream_read_int32_le(stream);

			/* copy normals */
			face->normals[j * 3 + 0] =
				normals[face->vertex_indices[j] * 3 + 0];
			face->normals[j * 3 + 1] =
				normals[face->vertex_indices[j] * 3 + 1];
			face->normals[j * 3 + 2] =
				normals[face->vertex_indices[j] * 3 + 2];

			/* texture stuff */
			if(face->tex_image)
			{
				face->tex_vertex_data[j * 2 + 0] =
					object->tex_vertex_data[face->vertex_indices[j] * 2 + 0];
				face->tex_vertex_data[j * 2 + 1] =
					object->tex_vertex_data[face->vertex_indices[j] * 2 + 1];
			}
		}

		object->faces = g_slist_append(object->faces, face);
	}

	/* free unused data */
	if(object->tex_vertex_data)
	{
		g_free(object->tex_vertex_data);
		object->tex_vertex_data = NULL;
	}
	if(normals)
		g_free(normals);

	model->objects = g_slist_append(model->objects, object);

	g3d_stream_seek(stream, off_start + mlength, G_SEEK_SET);

	return TRUE;
}

