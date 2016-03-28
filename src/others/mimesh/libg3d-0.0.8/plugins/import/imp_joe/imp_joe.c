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
#include <locale.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/model.h>
#include <g3d/object.h>
#include <g3d/iff.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/matrix.h>

G3DObject *joe_load_object(G3DContext *context, const gchar *filename,
	G3DModel *model);
GHashTable *joe_load_car(const gchar *filename);
void joe_destroy_car(GHashTable *hashtable);
gboolean joe_parse_vertex(const gchar *text, G3DFloat *x, G3DFloat *y, G3DFloat *z);
void joe_object_flip_x(G3DObject *object);

/*****************************************************************************/
/* plugin interface                                                          */
/*****************************************************************************/

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer plugin_data)
{
	G3DObject *object;
	GHashTable *cardata;
	gchar *value;
	G3DFloat x, y, z;
	G3DMatrix matrix[16];
	gboolean rval = FALSE;

	if(g_strcasecmp(stream->uri + strlen(stream->uri) - 3, "car") == 0)
	{
		/* .car file */
		setlocale(LC_NUMERIC, "C");

		cardata = joe_load_car(stream->uri);

		joe_load_object(context, "body.joe", model);
		joe_load_object(context, "interior.joe", model);
		joe_load_object(context, "glass.joe", model);

		/* wheels */
		object = joe_load_object(context, "wheel_front.joe", model);
		joe_object_flip_x(object);
		value = g_hash_table_lookup(cardata, "wheel-FL.position");
		if(value != NULL)
		{
			joe_parse_vertex(value, &x, &y, &z);
			g3d_matrix_identity(matrix);
			g3d_matrix_translate(y, x, z, matrix);
			g3d_object_transform(object, matrix);
		}

		object = joe_load_object(context, "wheel_front.joe", model);
		value = g_hash_table_lookup(cardata, "wheel-FR.position");
		if(value != NULL)
		{
			joe_parse_vertex(value, &x, &y, &z);
			g3d_matrix_identity(matrix);
			g3d_matrix_translate(y, x, z, matrix);
			g3d_object_transform(object, matrix);
		}

		object = joe_load_object(context, "wheel_rear.joe", model);
		joe_object_flip_x(object);
		value = g_hash_table_lookup(cardata, "wheel-RL.position");
		if(value != NULL)
		{
			joe_parse_vertex(value, &x, &y, &z);
			g3d_matrix_identity(matrix);
			g3d_matrix_translate(y, x, z, matrix);
			g3d_object_transform(object, matrix);
		}

		object = joe_load_object(context, "wheel_rear.joe", model);
		value = g_hash_table_lookup(cardata, "wheel-RR.position");
		if(value != NULL)
		{
			joe_parse_vertex(value, &x, &y, &z);
			g3d_matrix_identity(matrix);
			g3d_matrix_translate(y, x, z, matrix);
			g3d_object_transform(object, matrix);
		}

		joe_destroy_car(cardata);
		rval = TRUE;
	}
	else
	{
		/* .joe file */
		rval = (joe_load_object(context, stream->uri, model) != NULL);
	}

	g3d_matrix_identity(matrix);
	g3d_matrix_rotate_xyz(G_PI * -90.0 / 180, 0.0, 0.0, matrix);
	g3d_model_transform(model, matrix);

	return rval;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("VDrift models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("joe:car", ":", 0);
}

/*****************************************************************************/

G3DObject *joe_load_object(G3DContext *context, const gchar *filename,
	G3DModel *model)
{
	G3DStream *stream;
	gchar *basename, *texname;
	G3DObject *object;
	G3DMaterial *material;
	G3DFace *face;
	G3DImage *image;
	GSList *item;
	guint32 magic, version;
	guint32 num_faces, num_frames, num_verts, num_texcoords, num_normals;
	gint32 frame, i, j, index;
	guint16 *tex_indices, *normal_indices;
	G3DFloat *normals = NULL, *texcoords = NULL;

	stream = g3d_stream_open_file(filename, "rb");
	if(stream == NULL) {
		g_critical("JOE: failed to read '%s'", filename);
		return NULL;
	}

	magic = g3d_stream_read_int32_be(stream);
	if(magic != G3D_IFF_MKID('I','D','P','2')) {
		g_critical("JOE: wrong magic in '%s'", filename);
		g3d_stream_close(stream);
		return NULL;
	}

	/* base file name for object name & texture loading */
	basename = g_path_get_basename(filename);

	/* version 3 */

	version = g3d_stream_read_int32_le(stream);
	num_faces = g3d_stream_read_int32_le(stream);
	num_frames = g3d_stream_read_int32_le(stream);

	printf("JOE: faces: %d, frames: %d\n", num_faces, num_frames);

	/* create object */
	object = g_new0(G3DObject, 1);
	object->name = g_strdup(basename);
	model->objects = g_slist_append(model->objects, object);

	/* load texture image */
	memcpy(basename + strlen(basename) - 3, "png", 3);
	texname = g_strdup_printf("textures/%s", basename);
	image = g3d_texture_load_cached(context, model, texname);
	if(image == NULL)
		g_warning("JOE: failed to load texture '%s'\n", texname);
	else
		image->tex_id = g_str_hash(texname);
	g_free(texname);

	/* create default material */
	material = g3d_material_new();
	material->name = g_strdup("default material");
	material->tex_image = image;
	object->materials = g_slist_append(object->materials, material);

	/* frames */
	for(frame = 0; frame < 1; frame ++) {
		/* create temporary storage */
		tex_indices = g_new0(guint16, num_faces * 3 * 3);
		normal_indices = g_new0(guint16, num_faces * 3 * 2);

		/* faces blob */
		for(i = 0; i < num_faces; i ++) {
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 3;
			face->vertex_indices = g_new0(guint32, 3);

			for(j = 0; j < 3; j ++)
				face->vertex_indices[j] = g3d_stream_read_int16_le(stream);

			/* normalIndex */
			for(j = 0; j < 3; j ++)
				normal_indices[i * 3 + j] = g3d_stream_read_int16_le(stream);

			/* textureIndex */
			/* JOE_MAX_TEXTURES times, 1x for version 3 */
			for(j = 0; j < 3; j ++)
				tex_indices[i * 3 + j] = g3d_stream_read_int16_le(stream);

			object->faces = g_slist_append(object->faces, face);
		}

		/* num_verts */
		num_verts = g3d_stream_read_int32_le(stream);

		/* num_texcoords */
		num_texcoords = g3d_stream_read_int32_le(stream);
		if(num_texcoords != 0)
			texcoords = g_new0(G3DFloat, num_texcoords * 2);

		/* num_normals */
		num_normals = g3d_stream_read_int32_le(stream);
		if(num_normals != 0)
			normals = g_new0(G3DFloat, num_normals * 3);

		g_debug("JOE: verts: %d, texcoords: %d, normals: %d\n",
			num_verts, num_texcoords, num_normals);

		/* verts blob */
		object->vertex_count = num_verts;
		object->vertex_data = g_new0(G3DFloat, num_verts * 3);
		for(i = 0; i < num_verts; i ++) {
			object->vertex_data[i * 3 + 0] = g3d_stream_read_float_le(stream);
			object->vertex_data[i * 3 + 1] = g3d_stream_read_float_le(stream);
			object->vertex_data[i * 3 + 2] = g3d_stream_read_float_le(stream);
		}

		/* normals blob */
		for(i = 0; i < num_normals; i ++) {
			normals[i * 3 + 0] = - g3d_stream_read_float_le(stream);
			normals[i * 3 + 1] = - g3d_stream_read_float_le(stream);
			normals[i * 3 + 2] = - g3d_stream_read_float_le(stream);
		}

		/* texcoords blob */
		for(i = 0; i < num_texcoords; i ++) {
			texcoords[i * 2 + 0] = g3d_stream_read_float_le(stream);
			texcoords[i * 2 + 1] = g3d_stream_read_float_le(stream);
		}

		/* fix faces */
		for(item = object->faces, i = 0; item != NULL; item = item->next, i ++)
		{
			face = (G3DFace *)item->data;

			face->flags |= G3D_FLAG_FAC_NORMALS;
			if(image != NULL) face->flags |= G3D_FLAG_FAC_TEXMAP;

			face->normals = g_new0(G3DFloat, 3 * 3);
			face->tex_image = image;
			face->tex_vertex_count = 3;
			face->tex_vertex_data = g_new0(G3DFloat, 3 * 2);
			for(j = 0; j < 3; j ++)
			{
				index = normal_indices[i * 3 + j];
				face->normals[j * 3 + 0] = normals[index * 3 + 0];
				face->normals[j * 3 + 1] = normals[index * 3 + 1];
				face->normals[j * 3 + 2] = normals[index * 3 + 2];

				index = tex_indices[i * 3 + j];
				face->tex_vertex_data[j * 2 + 0] = texcoords[index * 2 + 0];
				face->tex_vertex_data[j * 2 + 1] = texcoords[index * 2 + 1];
			}
		}

		/* clear temporary storage */
		g_free(normal_indices);
		g_free(tex_indices);

		if(normals != NULL)
		{
			g_free(normals);
			normals = NULL;
		}

		if(texcoords != NULL)
		{
			g_free(texcoords);
			texcoords = NULL;
		}
	}

	/* clean up */
	g_free(basename);
	g3d_stream_close(stream);

	return object;
}

GHashTable *joe_load_car(const gchar *filename)
{
	G3DStream *stream;
	GHashTable *ht;
	gchar buffer[2048 + 1], section[256], varname[256], value[256];
	gchar *ep;

	stream = g3d_stream_open_file(filename, "r");
	if(stream == NULL) {
		g_critical("JOE: failed to read '%s'", filename);
		return NULL;
	}

	ht = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	memset(section, 0, 256);

	while(!g3d_stream_eof(stream)) {
		g3d_stream_read_line(stream, buffer, 2048);
		if((buffer[0] == '\0') || (buffer[0] == '\n'))
			continue;

		if(buffer[0] == '[') {
			/* section title */
			if(sscanf(buffer, "[ %s ]", section) != 1) {
				g_warning("JOE: CAR: failed to read section title '%s'\n",
					buffer);
			}
		} else {
			/* property */
			ep = strchr(buffer, '=');
			if(ep != NULL)
			{
				memset(varname, 0, 256);
				strncpy(varname, buffer, (ep - buffer));
				g_strstrip(varname);

				strcpy(value, ep + 1);
				g_strstrip(value);
#if DEBUG > 0
				g_debug("JOE: %s.%s = %s", section, varname, value);
#endif
				g_hash_table_insert(ht,
					g_strdup_printf("%s.%s", section, varname),
					g_strdup(value));
			}
		}
	}

	return ht;
}

void joe_destroy_car(GHashTable *hashtable)
{
	g_hash_table_destroy(hashtable);
}

gboolean joe_parse_vertex(const gchar *text, G3DFloat *x, G3DFloat *y, G3DFloat *z)
{
	return (sscanf(text, G3D_SCANF_FLOAT ", " G3D_SCANF_FLOAT ", " G3D_SCANF_FLOAT, x, y, z) == 3);
}

void joe_object_flip_x(G3DObject *object)
{
	guint32 i;

	for(i = 0; i < object->vertex_count; i ++)
		object->vertex_data[i * 3 + 0] *= -1;
}
