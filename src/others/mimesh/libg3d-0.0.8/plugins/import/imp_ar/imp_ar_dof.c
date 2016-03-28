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
#include <g3d/stream.h>
#include <g3d/texture.h>
#include <g3d/material.h>
#include <g3d/iff.h>

gchar *ar_dof_read_string(G3DStream *stream, gint32 *dlen)
{
	gint32 len;
	gchar *text;

	len = g3d_stream_read_int16_le(stream);
	*dlen -= 2;

	text = g_new0(gchar, len + 1);
	g3d_stream_read(stream, text, len);
	*dlen -= len;

	return text;
}

G3DMaterial *ar_dof_load_mat(G3DContext *context, G3DModel *model,
	G3DStream *stream)
{
	G3DMaterial *material;
	gint32 id, len = 0, dlen, i, ntex, trans = 0, blend = 0;
	gchar *tmp;

	id = g3d_stream_read_int32_be(stream);
	if(id != G3D_IFF_MKID('M','A','T','0'))
		return NULL;

	material = g3d_material_new();

	dlen = g3d_stream_read_int32_le(stream);
	do {
		id = g3d_stream_read_int32_be(stream);
		if(id != G3D_IFF_MKID('M','E','N','D'))
			len = g3d_stream_read_int32_le(stream);

		switch(id) {
			case G3D_IFF_MKID('M','E','N','D'):
				break;

			case G3D_IFF_MKID('M','H','D','R'):
				material->name = ar_dof_read_string(stream, &dlen);
				tmp = ar_dof_read_string(stream, &dlen);
				g_free(tmp);
				break;

			case G3D_IFF_MKID('M','C','O','L'):
				/* ambient */
				material->r = g3d_stream_read_float_le(stream);
				material->g = g3d_stream_read_float_le(stream);
				material->b = g3d_stream_read_float_le(stream);
				material->a = g3d_stream_read_float_le(stream);
				dlen -= 16;
				/* diffuse */
				g3d_stream_skip(stream, 16);
				dlen -= 16;
				/* specular */
				material->specular[0] = g3d_stream_read_float_le(stream);
				material->specular[1] = g3d_stream_read_float_le(stream);
				material->specular[2] = g3d_stream_read_float_le(stream);
				material->specular[3] = g3d_stream_read_float_le(stream);
				dlen -= 16;
				/* emission */
				g3d_stream_skip(stream, 16);
				dlen -= 16;
				/* shininess */
				material->shininess = g3d_stream_read_float_le(stream);
				dlen -= 4;
				break;

			case G3D_IFF_MKID('M','T','E','X'):
				ntex = g3d_stream_read_int32_le(stream);
				dlen -= 4;
				for(i = 0; i < ntex; i ++) {
					tmp = ar_dof_read_string(stream, &dlen);
					if(i == 0) {
						material->tex_image =
							g3d_texture_load_cached(context, model, tmp);
						if(material->tex_image)
							material->tex_image->tex_id = g_str_hash(tmp);
					}
					g_free(tmp);
				}
				break;

			case G3D_IFF_MKID('M','T','R','A'):
				/* transparency */
				trans = g3d_stream_read_int32_le(stream);
				/* blend mode */
				blend = g3d_stream_read_int32_le(stream);

				printf("D: MTRA: %s: trans: 0x%04x, blend: 0x%04x\n",
					(material->name ? material->name : "unnamed"),
					trans, blend);

				dlen -= 8;
				break;

			case G3D_IFF_MKID('M','C','F','L'):
				/* creation flags */
				g3d_stream_read_int32_le(stream);
				dlen -= 4;
				break;

			case G3D_IFF_MKID('M','U','V','W'):
				/* u offset */
				g3d_stream_read_int32_le(stream);
				/* v offset */
				g3d_stream_read_int32_le(stream);
				dlen -= 8;

				/* u tiling */
				g3d_stream_read_int32_le(stream);
				/* v tiling */
				g3d_stream_read_int32_le(stream);
				dlen -= 8;

				/* angle */
				g3d_stream_read_float_le(stream);
				/* blur */
				g3d_stream_read_float_le(stream);
				/* blur offset */
				g3d_stream_read_int32_le(stream);
				dlen -= 12;
				break;

			default:
				g3d_stream_skip(stream, len);
				dlen -= len;
				break;
		}
	}
	while((dlen > 0) && (id != G3D_IFF_MKID('M','E','N','D')));

	if(material->tex_image != NULL) {
		if(blend == 1)
			material->tex_image->tex_env = G3D_TEXENV_BLEND;
		else
			material->tex_image->tex_env = G3D_TEXENV_DECAL;
	}

	return material;
}

G3DObject *ar_dof_load_obj(G3DContext *context, G3DModel *model,
	G3DStream *stream)
{
	G3DObject *object, *pobj;
	G3DFace *face;
	G3DMaterial *material;
	GSList *item;
	gint32 id, len = 0, dlen, nverts, ntver, nnorm, nind, i, j, index;
	G3DFloat *tex_vertices = NULL, *normals = NULL;

	id = g3d_stream_read_int32_be(stream);
	dlen = g3d_stream_read_int32_le(stream);

	if(id != G3D_IFF_MKID('G','O','B','1')) {
		g3d_stream_skip(stream, dlen);
		return NULL;
	}

	object = g_new0(G3DObject, 1);
	object->name = g_strdup_printf("object @ 0x%08x",
		(guint32)g3d_stream_tell(stream));

	/* parent object for material references */
	pobj = (G3DObject *)g_slist_nth_data(model->objects, 0);

	/* default material */
	material = (G3DMaterial *)g_slist_nth_data(model->materials, 0);

	do {
		id = g3d_stream_read_int32_be(stream);
		if(id != G3D_IFF_MKID('G','E','N','D'))
			len = g3d_stream_read_int32_le(stream);

		switch(id) {
			case G3D_IFF_MKID('G','E','N','D'):
				/* end of object */
				break;

			case G3D_IFF_MKID('G','H','D','R'):
				/* object header */
				/* flags */
				i = g3d_stream_read_int32_le(stream);
				printf("D: GHDR: flags = 0x%04X\n", i);
				/* paint flags */
				i = g3d_stream_read_int32_le(stream);
				printf("D: GHDR: paint flags = 0x%04X\n", i);

				/* material ref */
				i = g3d_stream_read_int32_le(stream);
				material = g_slist_nth_data(pobj->materials, i);
				if(material == NULL)
					material = (G3DMaterial *)g_slist_nth_data(
						model->materials, 0);

				dlen -= 12;
				break;

			case G3D_IFF_MKID('V','E','R','T'):
				/* vertices */
				nverts = g3d_stream_read_int32_le(stream);

#if DEBUG > 2
				printf("D: %d vertices\n", nverts);
#endif

				dlen -= 4;
				if(nverts > 0) {
					object->vertex_count = nverts;
					object->vertex_data = g_new0(G3DFloat, nverts * 3);
					for(i = 0; i < nverts; i ++) {
						for(j = 0; j < 3; j ++)
							object->vertex_data[i * 3 + j] =
								g3d_stream_read_float_le(stream);
						dlen -= 12;
					}
				}
				break;

			case G3D_IFF_MKID('N', 'O','R','M'):
				/* normals */
				nnorm = g3d_stream_read_int32_le(stream);
				normals = g_new0(G3DFloat, nnorm * 3);
				dlen -= 4;
				for(i = 0; i < nnorm; i ++) {
					for(j = 0; j < 3; j ++)
						normals[i * 3 + j] = g3d_stream_read_float_le(stream);
					dlen -= 12;
				}
				break;

			case G3D_IFF_MKID('T', 'V','E','R'):
				/* texture vertices */
				ntver = g3d_stream_read_int32_le(stream);
				tex_vertices = g_new0(G3DFloat, ntver * 2);
				dlen -= 4;

#if DEBUG > 2
				printf("D: %d texture vertices @ 0x%08x\n", ntver,
					(guint32)g3d_stream_tell(stream) - 12);
#endif

				for(i = 0; (i < ntver) && (len > 0); i ++) {
					tex_vertices[i * 2 + 0] = g3d_stream_read_float_le(stream);
					tex_vertices[i * 2 + 1] =
						1.0 - g3d_stream_read_float_le(stream);
					dlen -= 8;
				}
				break;

			case G3D_IFF_MKID('B','R','S','T'):
				/* bursts */
				i = g3d_stream_read_int32_le(stream);
				dlen -= 4;
				g3d_stream_skip(stream, i * 4); /* burstStart */
				g3d_stream_skip(stream, i * 4); /* burstCount */
				g3d_stream_skip(stream, i * 4); /* burstMtlID */
				g3d_stream_skip(stream, i * 4); /* burstVperP */
				dlen -= (4 * 4 * i);
				break;

			case G3D_IFF_MKID('V','C','O','L'):
				/* vertex colors */
				i = g3d_stream_read_int32_le(stream);
				dlen -= 4;
				g3d_stream_skip(stream, i * 4 * 3);
				dlen -= (i * 4 * 3);
				break;

			case G3D_IFF_MKID('I','N','D','I'):
				/* indices */
				nind = g3d_stream_read_int32_le(stream);
				dlen -= 4;
				len -= 4;

#if DEBUG > 2
				printf("D: %d indices in %d bytes\n", nind, len);
#endif

				for(i = 0; i < nind; i += 3) {
					face = g_new0(G3DFace, 1);
					face->material = material;
					face->vertex_count = 3;
					face->vertex_indices = g_new0(guint32, 3);

					for(j = 0; j < 3; j ++)
						face->vertex_indices[j] =
							g3d_stream_read_int16_le(stream);
					dlen -= 6;
					len -= 6;

					object->faces = g_slist_append(object->faces, face);
				}
				break;

			default:
#if DEBUG > 0
				printf("D: skipping tag '%c%c%c%c @ 0x%08x'\n",
					(id << 24) & 0xFF, (id << 16) & 0xFF,
					(id << 8) & 0xFF, id & 0xFF,
					(guint32)g3d_stream_tell(stream));
#endif
				g3d_stream_skip(stream, len);
				dlen -= len;
				break;
		}
	} while((dlen > 0) && (id != G3D_IFF_MKID('G','E','N','D')));

	/* fix faces with normals and texture vertices */
	for(item = object->faces; item != NULL; item = item->next) {
		face = (G3DFace *)item->data;

		if(tex_vertices != NULL) {
			face->tex_image = material->tex_image;
			face->tex_vertex_count = 3;
			face->tex_vertex_data = g_new0(G3DFloat, 3 * 2);
			for(j = 0; j < 3; j ++) {
				index = face->vertex_indices[j];
				face->tex_vertex_data[j * 2 + 0] = tex_vertices[index * 2 + 0];
				face->tex_vertex_data[j * 2 + 1] = tex_vertices[index * 2 + 1];
			}
			if(face->tex_image != NULL)
				face->flags |= G3D_FLAG_FAC_TEXMAP;
		}

		if(normals != NULL) {
			face->normals = g_new0(G3DFloat, 3 * 3);
			for(j = 0; j < 3; j ++) {
				index = face->vertex_indices[j];
				face->normals[j * 3 + 0] = normals[index * 3 + 0];
				face->normals[j * 3 + 1] = normals[index * 3 + 1];
				face->normals[j * 3 + 2] = normals[index * 3 + 2];
			}
			face->flags |= G3D_FLAG_FAC_NORMALS;
		}
	}

	/* cleanup */
	if(tex_vertices != NULL)
		g_free(tex_vertices);

	if(normals != NULL)
		g_free(normals);

	return object;
}

G3DObject *ar_dof_load(G3DContext *context, G3DModel *model,
	G3DStream *stream)
{
	gint32 id, dlen, len = 0, nmat, nobj, i;
	G3DObject *object, *cobj;
	G3DMaterial *material;

	/* file is little-endian, but read IDs as big-endian to use
	 * G3D_IFF_MKID to compare */

	id = g3d_stream_read_int32_be(stream);
	if(id != G3D_IFF_MKID('D','O','F','1')) {
		g_warning("%s is not a DOF1 file\n", stream->uri);
		return NULL;
	}
	dlen = g3d_stream_read_int32_le(stream);

	object = g_new0(G3DObject, 1);
	object->name = g_strdup(stream->uri);
	model->objects = g_slist_append(model->objects, object);

	do {
		id = g3d_stream_read_int32_be(stream);
		if(id != G3D_IFF_MKID('E','D','O','F'))
			len = g3d_stream_read_int32_le(stream);
		dlen -= 8;

		switch(id) {
			case G3D_IFF_MKID('E','D','O','F'):
				/* end of DOF */
				break;

			case G3D_IFF_MKID('M','A','T','S'):
				nmat = g3d_stream_read_int32_le(stream);
				for(i = 0; i < nmat; i ++) {
					material = ar_dof_load_mat(context, model, stream);
					if(material)
						object->materials = g_slist_append(object->materials,
							material);
				}
				dlen -= len;
				break;

			case G3D_IFF_MKID('G','E','O','B'):
				nobj = g3d_stream_read_int32_le(stream);
				for(i = 0; i < nobj; i ++) {
					cobj = ar_dof_load_obj(context, model, stream);
					if(cobj)
						object->objects =
							g_slist_append(object->objects, cobj);
				}
				dlen -= len;
				break;

			default:
				g_warning("DOF: unknown ID '%c%c%c%c' @ 0x%08x",
					(id >> 24) & 0xFF, (id >> 16) & 0xFF,
					(id >> 8) & 0xFF, id  & 0xFF,
					(guint32)g3d_stream_tell(stream) - 8);
				g3d_stream_skip(stream, len);
				dlen -= len;
				break;
		}
	} while((dlen > 0) &&
		(id != G3D_IFF_MKID('E','D','O','F')) &&
		(!g3d_stream_eof(stream)));

	return object;
}

