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

#include <g3d/types.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>
#include <g3d/face.h>
#include <g3d/texture.h>
#include <g3d/object.h>

EAPI
void g3d_object_free(G3DObject *object)
{
	GSList *slist, *snext;
	G3DMaterial *mat;
	G3DFace *face;

	if(object->name != NULL)
		g_free(object->name);

	/* materials */
	slist = object->materials;
	while(slist != NULL)
	{
		mat = (G3DMaterial*)slist->data;
		snext = slist->next;
		g_slist_free_1(slist);
		slist = snext;
	}

	/* faces */
	slist = object->faces;
	while(slist != NULL)
	{
		face = (G3DFace*)slist->data;
		g3d_face_free(face);
		snext = slist->next;
		g_slist_free_1(slist);
		slist = snext;
	}

	/* vertices */
	if(object->vertex_data != NULL) g_free(object->vertex_data);
	if(object->tex_vertex_data != NULL) g_free(object->tex_vertex_data);
	if(object->_normals != NULL) g_free(object->_normals);
	if(object->_indices != NULL) g_free(object->_indices);
	if(object->_materials != NULL) g_free(object->_materials);
	if(object->_flags != NULL) g_free(object->_flags);

	g_free(object);
}

EAPI
gdouble g3d_object_radius(G3DObject *object)
{
	guint32 i;
	gdouble max_radius = 0.0, r;
	G3DVector *v;

	for(i = 0; i < object->vertex_count; i ++)
	{
		v = &object->vertex_data[i * 3];
		r = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		if(r > max_radius)
			max_radius = r;
	}

	return sqrt(max_radius);
}

EAPI
gboolean g3d_object_scale(G3DObject *object, G3DFloat scale)
{
	guint32 i;
	G3DMatrix *matrix;

	for(i = 0; i < object->vertex_count; i ++)
	{
		object->vertex_data[i * 3 + 0] *= scale;
		object->vertex_data[i * 3 + 1] *= scale;
		object->vertex_data[i * 3 + 2] *= scale;
	}

	if(object->transformation)
	{
		matrix = object->transformation->matrix;
		g3d_matrix_translate(
			- (matrix[12] - (matrix[12] * scale)),
			- (matrix[13] - (matrix[13] * scale)),
			- (matrix[14] - (matrix[14] * scale)),
			matrix);
	}

	return TRUE;
}

EAPI
gboolean g3d_object_transform_normals(G3DObject *object, G3DMatrix *matrix)
{
	gint32 i;
	G3DFace *face;
	GSList *fitem;

	fitem = object->faces;
	while(fitem)
	{
		face = (G3DFace *)fitem->data;

		if(face->flags & G3D_FLAG_FAC_NORMALS)
		{
			for(i = 0; i < face->vertex_count; i ++)
			{
				g3d_vector_transform(
					&(face->normals[i * 3 + 0]),
					&(face->normals[i * 3 + 1]),
					&(face->normals[i * 3 + 2]),
					matrix);
				g3d_vector_unify(
					&(face->normals[i * 3 + 0]),
					&(face->normals[i * 3 + 1]),
					&(face->normals[i * 3 + 2]));
			}
		}

		fitem = fitem->next;
	}
	return TRUE;
}

#define G3D_OBJECT_TRANSFORM_NORMALS 0

EAPI
gboolean g3d_object_transform(G3DObject *object, G3DMatrix *matrix)
{
	guint32 i;
	GSList *oitem;

	/* transform vertices */
	for(i = 0; i < object->vertex_count; i ++)
	{
		g3d_vector_transform(
			&(object->vertex_data[i * 3 + 0]),
			&(object->vertex_data[i * 3 + 1]),
			&(object->vertex_data[i * 3 + 2]),
			matrix);
	}

#if G3D_OBJECT_TRANSFORM_NORMALS
	g3d_object_transform_normals(object, matrix);
#endif

	/* transform sub-objects */
	for(oitem = object->objects; oitem != NULL; oitem = oitem->next)
		g3d_object_transform((G3DObject *)oitem->data, matrix);

	return TRUE;
}

EAPI
G3DObject *g3d_object_duplicate(G3DObject *object)
{
	G3DObject *new, *sub, *newsub;
	G3DFace *face, *oface;
	GSList *litem;

	new = g_new0(G3DObject, 1);

	/* name */
	if(object->name)
		new->name = g_strdup(object->name);

	/* vertices */
	new->vertex_count = object->vertex_count;
	new->vertex_data = g3d_vector_new(3, new->vertex_count);
	memcpy(new->vertex_data, object->vertex_data,
		new->vertex_count * 3 * sizeof(G3DVector));

	/* texture stuff */
	/* TODO: implement? */

	/* faces */
	for(litem = object->faces; litem != NULL; litem = litem->next) {
		oface = litem->data;

		face = g_new0(G3DFace, 1);
		face->material = oface->material;
		face->vertex_count = oface->vertex_count;
		face->vertex_indices = g_new0(guint32, face->vertex_count);
		memcpy(face->vertex_indices, oface->vertex_indices,
			face->vertex_count * sizeof(guint32));
		face->flags = oface->flags;
		if(face->flags & G3D_FLAG_FAC_NORMALS)
		{
			face->normals = g3d_vector_new(3, face->vertex_count);
			memcpy(face->normals, oface->normals,
				face->vertex_count * 3 * sizeof(G3DVector));
		}
		if(face->flags & G3D_FLAG_FAC_TEXMAP)
		{
			face->tex_image = oface->tex_image;
			face->tex_vertex_count = oface->tex_vertex_count;
			face->tex_vertex_data = g3d_vector_new(2, face->tex_vertex_count);
			memcpy(face->tex_vertex_data, oface->tex_vertex_data,
				face->tex_vertex_count * 2 * sizeof(G3DVector));
		}

		new->faces = g_slist_prepend(new->faces, face);
	}

	/* sub-objects */
	for(litem = object->objects; litem != NULL; litem = litem->next) {
		sub = litem->data;
		newsub = g3d_object_duplicate(sub);
		new->objects = g_slist_append(new->objects, newsub);
	}

	return new;
}

EAPI
gboolean g3d_object_merge(G3DObject *o1, G3DObject *o2)
{
	G3DFace *face;
	GSList *fitem;
	guint32 i, voff, foff = 0;

	/* copy vertices */
	voff = o1->vertex_count;
	o1->vertex_count += o2->vertex_count;

	o1->vertex_data = g_realloc(o1->vertex_data,
		o1->vertex_count * 3 * sizeof(G3DVector));

	memcpy(o1->vertex_data + voff * 3, o2->vertex_data,
		o2->vertex_count * 3 * sizeof(G3DVector));

	/* attach faces to first object */
	fitem = o2->faces;
	while(fitem)
	{
		face = (G3DFace *)fitem->data;
		foff ++;

		if(face->vertex_count > 100)
		{
			g_warning("face->vertex_count > 100: %d (%dth)",
				face->vertex_count, foff);
		}

		for(i = 0; i < face->vertex_count; i ++)
			face->vertex_indices[i] += voff;

		o1->faces = g_slist_prepend(o1->faces, face);

		fitem = fitem->next;
	}

	/* FIXME: clean up o2 or copy faces */

	return TRUE;
}

EAPI
gboolean g3d_object_smooth(G3DObject *object)
{
	/* FIXME: implement */
	return FALSE;
}

EAPI
gboolean g3d_object_optimize(G3DObject *object)
{
	G3DFace *face;
	guint32 index = 0, i, j;
	GSList *fitem;
	G3DVector nx, ny, nz, su = 1.0, sv = 1.0;

	/* count number of faces (optimized) */
	object->_num_faces = 0;
	fitem = object->faces;
	while(fitem)
	{
		face = (G3DFace *)fitem->data;
		object->_num_faces += face->vertex_count - 2;

		fitem = fitem->next;
	}

	object->_normals = g3d_vector_new(3, 3 * object->_num_faces);
	object->_materials = g_new0(G3DMaterial *, object->_num_faces);
	object->_flags = g_new0(guint32, object->_num_faces);
	object->_indices = g_new0(guint32, object->_num_faces * 3);
	object->_tex_images = g_new0(guint32, object->_num_faces);
	object->_tex_coords = g3d_vector_new(2, object->_num_faces * 3);

	/* copy faces */
	fitem = object->faces;
	while(fitem)
	{
		face = (G3DFace *)fitem->data;

		/* generate default normal for face */
		if(!(face->flags & G3D_FLAG_FAC_NORMALS))
		{
			if(!g3d_face_get_normal(face, object, &nx, &ny, &nz))
			{
				fitem = fitem->next;
				object->_num_faces -= face->vertex_count - 2;
			}

			g3d_vector_unify(&nx, &ny, &nz);
		}

		for(i = 0; i < (face->vertex_count - 2); i ++)
		{
			object->_materials[index] = face->material;
			object->_flags[index] = face->flags;

			if(face->flags & G3D_FLAG_FAC_TEXMAP)
			{
#if 0
				g3d_texture_prepare(face->tex_image);
#endif
				object->_tex_images[index] = face->tex_image->tex_id;
				su = face->tex_image->tex_scale_u;
				sv = face->tex_image->tex_scale_v;
			}

			for(j = 0; j < 3; j ++)
			{
				/* vertex stuff */
				if(j == 0)
					object->_indices[index * 3] = face->vertex_indices[0];
				else
					object->_indices[index * 3+j] = face->vertex_indices[i+j];

				/* normal stuff */
				if(face->flags & G3D_FLAG_FAC_NORMALS)
				{
					object->_normals[(index * 3 + j) * 3 + 0] =
						face->normals[(i + j) * 3 + 0];
					object->_normals[(index * 3 + j) * 3 + 1] =
						face->normals[(i + j) * 3 + 1];
					object->_normals[(index * 3 + j) * 3 + 2] =
						face->normals[(i + j) * 3 + 2];
				}
				else
				{
					object->_normals[(index * 3 + j) * 3 + 0] = nx;
					object->_normals[(index * 3 + j) * 3 + 1] = ny;
					object->_normals[(index * 3 + j) * 3 + 2] = nz;
				}

				/* texture stuff */
				if(face->flags & G3D_FLAG_FAC_TEXMAP)
				{
					/* u */
					object->_tex_coords[(index * 3 + j) * 2 + 0] = su *
						((j == 0) ?
							face->tex_vertex_data[0] :
							face->tex_vertex_data[(i + j) * 2 + 0]);
					/* v */
					object->_tex_coords[(index * 3 + j) * 2 + 1] = sv *
						((j == 0) ?
							face->tex_vertex_data[1] :
							face->tex_vertex_data[(i + j) * 2 + 1]);
				}
			} /* j: 0 < 3 */

			index ++;
		} /* i: 0 < vertex_count - 2 */

		fitem = fitem->next;
	} /* while(fitem) */

	return TRUE;
}

