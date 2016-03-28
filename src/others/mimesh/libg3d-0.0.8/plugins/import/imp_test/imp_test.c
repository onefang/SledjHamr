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
#include <g3d/material.h>
#include <g3d/matrix.h>
#include <g3d/primitive.h>
#include <g3d/object.h>
#include <g3d/texture.h>

static gboolean test_primitive_transfrom(G3DModel *model);
static gboolean test_texture_uv(G3DContext *context, G3DModel *model);

/*****************************************************************************/
/* plugin interface                                                          */
/*****************************************************************************/


EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	guint32 test = 1;

	switch(test) {
		case 0:
			return test_primitive_transfrom(model);
			break;
		case 1:
			return test_texture_uv(context, model);
			break;
		default:
			break;
	}
	return FALSE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("Test plugin.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("test", ":", 0);
}

/****************************************************************************/

static gboolean test_primitive_transfrom(G3DModel *model)
{
	G3DObject *sphere, *cntr;
	G3DMaterial *material;
	G3DTransformation *tf;
	G3DFloat matrix[16];
	/* gint32 i, j; */

	cntr = g_new0(G3DObject, 1);
	cntr->name = g_strdup("container");

	tf = g_new0(G3DTransformation, 1);
	g3d_matrix_identity(tf->matrix);
	g3d_matrix_scale(1.0, 1.0, 2.0, tf->matrix);
	cntr->transformation = tf;
	model->objects = g_slist_append(model->objects, cntr);

	material = g3d_material_new();
	model->materials = g_slist_append(model->materials, material);

	/* 1 */
	sphere = g3d_primitive_sphere(1.0, 36, 36, material);
	cntr->objects = g_slist_append(cntr->objects, sphere);

	/* 2 */
	sphere = g3d_primitive_sphere(1.0, 6, 6, material);
	cntr->objects = g_slist_append(cntr->objects, sphere);

	tf = g_new0(G3DTransformation, 1);
	g3d_matrix_identity(tf->matrix);
	g3d_matrix_translate(2.5, 1.0, 1.0, tf->matrix);
	sphere->transformation = tf;

	/* 3 */
	sphere = g3d_primitive_sphere(1.0, 12, 12, material);
	cntr->objects = g_slist_append(cntr->objects, sphere);

	g3d_matrix_identity(matrix);
	g3d_matrix_translate(5, 2.0, 2.0, matrix);
	g3d_matrix_scale(2.0, 2.0, 1.0, matrix);
	g3d_object_transform(sphere, matrix);

	return TRUE;
}

static gboolean test_texture_uv(G3DContext *context, G3DModel *model)
{
	G3DObject *box;
	G3DMaterial *material;
	G3DFace *face;
	GSList *item;

	material = g3d_material_new();
	material->r = 1.0;
	material->g = 0.8;
	material->b = 0.2;
	material->a = 0.9;
	material->name = g_strdup("default texture");
	material->tex_image = g3d_texture_load_cached(context, model,
		"test-texture.png");

	box = g3d_primitive_box(1.0, 1.0, 1.0, material);
	box->name = g_strdup("test box");

	if(material->tex_image != NULL) {
		material->tex_image->tex_env = G3D_TEXENV_REPLACE;

		for(item = box->faces; item != NULL; item = item->next) {
			face = (G3DFace *)item->data;
			face->tex_image = material->tex_image;
			face->flags |= G3D_FLAG_FAC_TEXMAP;

#define MIN_U 0.0
#define MAX_U 1.0
#define MIN_V -0.5
#define MAX_V 1.5

			face->tex_vertex_data[0 * 2 + 0] = MIN_U;
			face->tex_vertex_data[0 * 2 + 1] = MIN_V;
			face->tex_vertex_data[1 * 2 + 0] = MIN_U;
			face->tex_vertex_data[1 * 2 + 1] = MAX_V;
			face->tex_vertex_data[2 * 2 + 0] = MAX_U;
			face->tex_vertex_data[2 * 2 + 1] = MAX_V;
			face->tex_vertex_data[3 * 2 + 0] = MAX_U;
			face->tex_vertex_data[3 * 2 + 1] = MIN_V;
		}
	}

	model->objects = g_slist_append(model->objects, box);
	model->materials = g_slist_append(model->materials, material);

	return TRUE;
}
