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
#include <stdlib.h>
#include <string.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/iff.h>
#include <g3d/vector.h>

static G3DObject *glb_load_object(G3DContext *context, G3DStream *stream,
	G3DModel *model);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	return (glb_load_object(context, stream, model) != NULL);
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("UltimateStunts models.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("glb", ":", 0);
}

/*****************************************************************************/
/* GLB specific                                                              */

static G3DFloat glb_get_float(G3DStream *stream)
{
	return 0.001 * (
		(float)((guint32)g3d_stream_read_int32_be(stream)) - (float)(0x7FFFFFFF));
}

static G3DObject *glb_load_object(G3DContext *context, G3DStream *stream,
	G3DModel *model)
{
	G3DObject *pobject, *object;
	G3DMaterial *material;
	G3DFace *face;
	guint32 magic, otype, index;
	gint32 i, j, msize, namelen, datalen, nvertices, nindices;
	gchar *name;
	G3DFloat *normals = NULL, *texcoords = NULL;

	magic = g3d_stream_read_int32_be(stream);
	if(magic != G3D_IFF_MKID('\0', 'G', 'L', 'B')) {
		g_warning("%s is not a correct GLB file (wrong magic)\n",
			stream->uri);
		return NULL;
	}

	pobject = g_new0(G3DObject, 1);
	pobject->name = g_strdup(stream->uri);
	model->objects = g_slist_append(model->objects, pobject);

	while(!g3d_stream_eof(stream)) {
		otype = g3d_stream_read_int32_be(stream);
		namelen = g3d_stream_read_int32_be(stream);

		if(namelen == 0)
			break;

		name = g_new0(gchar, namelen + 1);
		g3d_stream_read(stream, name, namelen);
#if DEBUG > 0
		printf("GLB: object named '%s'\n", name);
#endif

		object = g_new0(G3DObject, 1);
		object->name = g_strdup(name);
		g_free(name);
		pobject->objects = g_slist_append(pobject->objects, object);

		/* hide collision planes by default */
		if(strncmp(object->name, "Collision plane", 15) == 0)
			object->hide = TRUE;

		datalen = g3d_stream_read_int32_be(stream);

		if(otype != 0) {
			/* skip */
			g3d_stream_skip(stream, datalen);
			continue;
		}

		/* object type 0 */
		msize = g3d_stream_read_int32_be(stream);
		nvertices = g3d_stream_read_int32_be(stream);
		nindices = g3d_stream_read_int32_be(stream);

#if DEBUG > 0
		printf("GLB: material size: %d bytes, %d vertices, %d indices\n",
			msize, nvertices, nindices);
#endif
		if(msize > 0) {
			/* material */
			material = g3d_material_new();
			material->name = g_strdup("default material");
			object->materials = g_slist_append(object->materials, material);

			material->r = (G3DFloat)g3d_stream_read_int8(stream) / 255.0;
			material->g = (G3DFloat)g3d_stream_read_int8(stream) / 255.0;
			material->b = (G3DFloat)g3d_stream_read_int8(stream) / 255.0;
			material->a = (G3DFloat)g3d_stream_read_int8(stream) / 255.0;

			if(material->a == 0.0)
				material->a = 1.0;

			/* replacement color */
			g3d_stream_read_int8(stream);
			g3d_stream_read_int8(stream);
			g3d_stream_read_int8(stream);
			g3d_stream_read_int8(stream); /* unused */

			g3d_stream_read_int8(stream); /* LODs */
			g3d_stream_read_int8(stream); /* reflectance */
			/* emissivity */
			material->shininess = (G3DFloat)g3d_stream_read_int8(stream) / 255.0;
			g3d_stream_read_int8(stream); /* static friction */
			g3d_stream_read_int8(stream); /* dynamic friction */
			g3d_stream_read_int8(stream); /* unused */
			g3d_stream_read_int8(stream); /* unused */
			g3d_stream_read_int8(stream); /* unused */

			/* texture name */
			namelen = msize - 16;
			if(namelen > 0) {
				name = g_new0(gchar, namelen + 1);
				g3d_stream_read(stream, name, namelen);
#if DEBUG > 1
				printf("GLB: texture name: %s\n", name);
#endif

				/* texture name is something like "0", the real name is in
				 * "../$carname.conf"; try to load default texture */
				if(name[0] == '0') {
					if(g_file_test("textures.jpg", G_FILE_TEST_EXISTS)) {
						material->tex_image = g3d_texture_load_cached(
							context, model, "textures.jpg");
						if(material->tex_image != NULL)
							material->tex_image->tex_id = 1;
					}
				}

				g_free(name);
			}
		}

		/* vertices */
		if(nvertices > 0) {
			object->vertex_count = nvertices;
			object->vertex_data = g_new0(G3DFloat, nvertices * 3);
			normals = g_new0(G3DFloat, nvertices * 3);
			texcoords = g_new0(G3DFloat, nvertices * 2);

			for(i = 0; i < nvertices; i ++) {
				object->vertex_data[i * 3 + 0] = glb_get_float(stream);
				object->vertex_data[i * 3 + 1] = glb_get_float(stream);
				object->vertex_data[i * 3 + 2] = glb_get_float(stream);

#if DEBUG > 3
				printf("D: %f, %f, %f\n",
					object->vertex_data[i * 3 + 0],
					object->vertex_data[i * 3 + 1],
					object->vertex_data[i * 3 + 2]);
#endif

				/* normal */
				normals[i * 3 + 0] = glb_get_float(stream);
				normals[i * 3 + 1] = glb_get_float(stream);
				normals[i * 3 + 2] = glb_get_float(stream);
				g3d_vector_unify(
					normals + i * 3 + 0,
					normals + i * 3 + 1,
					normals + i * 3 + 2);

				/* texture coordinates */
				texcoords[i * 2 + 0] = glb_get_float(stream) / 64;
				texcoords[i * 2 + 1] = 1.0 - glb_get_float(stream) / 64;
			}
		}

		if(nindices > 0) {
			for(i = 0; i < nindices; i += 3) {
				face = g_new0(G3DFace, 1);
				face->vertex_count = 3;
				face->vertex_indices = g_new0(guint32, 3);
				face->normals = g_new0(G3DFloat, 3 * 3);
				face->flags |= G3D_FLAG_FAC_NORMALS;
				for(j = 0; j < 3; j ++) {
					face->vertex_indices[j] = g3d_stream_read_int32_be(stream);

					/* set normals */
					index = face->vertex_indices[j];
					face->normals[j * 3 + 0] = normals[index * 3 + 0];
					face->normals[j * 3 + 1] = normals[index * 3 + 1];
					face->normals[j * 3 + 2] = normals[index * 3 + 2];
				}
				face->material = g_slist_nth_data(object->materials, 0);

				if(face->material->tex_image != NULL) {
					face->tex_vertex_count = 3;
					face->tex_vertex_data = g_new0(G3DFloat, 3 * 2);
					face->tex_image = face->material->tex_image;
					for(j = 0; j < 3; j ++) {
						index = face->vertex_indices[j];

						face->tex_vertex_data[j * 2 + 0] =
							texcoords[index * 2 + 0];
						face->tex_vertex_data[j * 2 + 1] =
							texcoords[index * 2 + 1];
						face->flags |= G3D_FLAG_FAC_TEXMAP;
					}
				}

				object->faces = g_slist_append(object->faces, face);
			}
		}

		if(normals != NULL)
			g_free(normals);
		if(texcoords != NULL)
			g_free(texcoords);
	}

	return pobject;
}
