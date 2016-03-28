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

#include <g3d/stream.h>
#include <g3d/texture.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>

#include "imp_3ds_callbacks.h"

#define X3DS_FLAG_TENSION       0x01
#define X3DS_FLAG_CONTINUITY    0x02
#define X3DS_FLAG_BIAS          0x04
#define X3DS_FLAG_EASE_TO       0x08
#define X3DS_FLAG_EASE_FROM     0x10

gboolean x3ds_cb_0x0002(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 version;

	version = g3d_stream_read_int32_le(global->stream);
	parent->nb -= 4;
#if DEBUG > 0
	g_debug("[3DS] M3D version %d", version);
#endif
	return TRUE;
}

/* color float */
gboolean x3ds_cb_0x0010(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DFloat r, g, b;

	r = g3d_stream_read_float_le(global->stream);
	g = g3d_stream_read_float_le(global->stream);
	b = g3d_stream_read_float_le(global->stream);
	parent->nb -= 12;

	switch(parent->id)
	{
		case 0x1200: /* SOLID_BGND */
			g3d_context_set_bgcolor(global->context, r, g, b, 1.0);
			break;

		case 0xA010: /* ambient color */
			break;

		case 0xA020: /* diffuse color */
			material = (G3DMaterial *)parent->object;
			g_return_val_if_fail(material, FALSE);

			material->r = r;
			material->g = g;
			material->b = b;
			break;

		case 0xA030: /* specular color */
			material = (G3DMaterial *)parent->object;
			g_return_val_if_fail(material, FALSE);

			material->specular[0] = r;
			material->specular[1] = g;
			material->specular[2] = b;
			material->specular[3] = 0.25;
			break;

		default:
#if DEBUG > 0
			g_warning("[3DS] unhandled COLOR_F in 0x%04X", parent->id);
#endif
			break;
	}

	return TRUE;
}

/* color 24 */
gboolean x3ds_cb_0x0011(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	gint32 r, g, b;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	r = g3d_stream_read_int8(global->stream);
	g = g3d_stream_read_int8(global->stream);
	b = g3d_stream_read_int8(global->stream);
	parent->nb -= 3;

	switch(parent->id)
	{

		case 0xA010: /* ambient color */
			break;

		case 0xA020: /* diffuse color */
			material->r = (G3DFloat)r / 255.0;
			material->g = (G3DFloat)g / 255.0;
			material->b = (G3DFloat)b / 255.0;
			break;

		case 0xA030: /* specular color */
			material->specular[0] = (GLfloat)r / 255.0;
			material->specular[1] = (GLfloat)g / 255.0;
			material->specular[2] = (GLfloat)b / 255.0;
			material->specular[3] = 0.25;
			break;

		default:
#if DEBUG > 0
			g_warning("[3DS] unhandled COLOR_24 in 0x%04X", parent->id);
#endif
			break;
	}

	return TRUE;
}

/* short percentage */
gboolean x3ds_cb_0x0030(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	gint32 percent;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	percent = g3d_stream_read_int16_le(global->stream);
	parent->nb -= 2;

	switch(parent->id)
	{
		case 0xA040: /* shininess */
			material->shininess = (G3DFloat)percent / 100.0;
			break;

		case 0xA041: /* shininess (2) */
			/* TODO: do something here? */
			break;

		case 0xA050: /* transparency */
			material->a = 1.0 - ((G3DFloat)percent / 100.0);
			break;

		case 0xA052: /* fallthrough */
			/* TODO: do something here? */
			break;

		case 0xA053: /* blur */
			/* TODO: do something here? */
			break;

		case 0xA084: /* self illumination */
			/* TODO: do something here? */
			break;

		case 0xA200: /* texture map */
			/* TODO: do something here? */
			break;

		case 0xA210: /* opacity map */
			/* TODO: do something here? */
			g_debug("[3DS] opacity percentage: %d%%\n", percent);
			break;

		case 0xA220: /* reflection map */
			/* TODO: do something here? */
			break;

		case 0xA230: /* bump map */
			/* TODO: do something here? */
			break;

		default:
#if DEBUG > 0
			g_warning("[3DS] unhandled INT_PERCENTAGE in 0x%04X",
				parent->id);
#endif
			break;
	}

	return TRUE;
}

/* float percentage */
gboolean x3ds_cb_0x0031(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DFloat percent;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	percent = g3d_stream_read_float_le(global->stream);
	parent->nb -= 4;

	switch(parent->id)
	{
		case 0xA040: /* shininess */
			material->shininess = percent;
			break;

		case 0xA050: /* transparency */
			material->a = 1.0 - percent;
			break;

		default:
#if DEBUG > 0
			g_warning("[3DS] unhandled FLOAT_PERCENTAGE in 0x%04X",
				parent->id);
#endif
			break;
	}

	return TRUE;
}

/* master scale */
gboolean x3ds_cb_0x0100(x3ds_global_data *global, x3ds_parent_data *parent)
{
	global->scale = g3d_stream_read_float_le(global->stream);
	parent->nb -= 4;

	return TRUE;
}

/* named object */
gboolean x3ds_cb_0x4000(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gchar buffer[1024];

	parent->nb -= x3ds_read_cstr(global->stream, buffer);
	parent->object = x3ds_newobject(global->model, buffer);

	return TRUE;
}

/* point array */
gboolean x3ds_cb_0x4110(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i;

	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	object->vertex_count = g3d_stream_read_int16_le(global->stream);
	parent->nb -= 2;

	object->vertex_data = g_new0(G3DFloat, object->vertex_count * 3);
	for(i = 0; i < object->vertex_count; i ++)
	{
		object->vertex_data[i * 3 + 0] = g3d_stream_read_float_le(global->stream);
		object->vertex_data[i * 3 + 1] = g3d_stream_read_float_le(global->stream);
		object->vertex_data[i * 3 + 2] = g3d_stream_read_float_le(global->stream);

		parent->nb -= 12;

		if((i % 1000) == 0) x3ds_update_progress(global, parent->level);
	}
	return TRUE;
}

/* face array */
gboolean x3ds_cb_0x4120(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 i, flags, nfaces;
#define X3DS_REORDER_FACES
#ifdef X3DS_REORDER_FACES
	gint32 p1 = -1, p2 = -1, bottle;
#endif
	G3DFace *face;
	G3DObject *object;

	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	nfaces = g3d_stream_read_int16_le(global->stream);
	parent->nb -= 2;

	for(i = 0; i < nfaces; i ++)
	{
		face = g_new0(G3DFace, 1);

		face->vertex_count = 3;
		face->vertex_indices = g_malloc(3 * sizeof(guint32));

		face->vertex_indices[0] = g3d_stream_read_int16_le(global->stream);
		face->vertex_indices[1] = g3d_stream_read_int16_le(global->stream);
		face->vertex_indices[2] = g3d_stream_read_int16_le(global->stream);
		flags = g3d_stream_read_int16_le(global->stream);
		parent->nb -= 8;

#ifdef X3DS_REORDER_FACES
		/* try to put all faces in the same direction */
		if((p1 == face->vertex_indices[0]) && (p2 == face->vertex_indices[1]))
		{
			bottle = face->vertex_indices[0];
			face->vertex_indices[0] = face->vertex_indices[2];
			face->vertex_indices[2] = bottle;
		}

		p1 = face->vertex_indices[0];
		p2 = face->vertex_indices[1];
#endif

		face->material = g_slist_nth_data(object->materials, 0);

		object->faces = g_slist_append(object->faces, face);

		if((i % 1000) == 0) x3ds_update_progress(global, parent->level);
	}

	return TRUE;
}

/* mesh mat group */
gboolean x3ds_cb_0x4130(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i, j, facenum, nfaces;
	gchar buffer[512];
	G3DMaterial *material = NULL, *mat;
	G3DFace *face;
	GSList *mlist;

	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	/* name of material */
	parent->nb -= x3ds_read_cstr(global->stream, buffer);

	/* find material in list */
	mlist = global->model->materials;
	while(mlist != NULL)
	{
		mat = (G3DMaterial*)mlist->data;
		if(strcmp(mat->name, buffer) == 0)
		{
			material = mat;
			break;
		}
		mlist = mlist->next;
	}

	nfaces = g3d_stream_read_int16_le(global->stream);
	parent->nb -= 2;

	for(i = 0; i < nfaces; i ++)
	{
		facenum = g3d_stream_read_int16_le(global->stream);
		parent->nb -= 2;

		if(material != NULL)
		{
			face = (G3DFace*)g_slist_nth_data(object->faces, facenum);
			if(face == NULL) continue;

			face->material = material;

			if(face->material->tex_image && object->tex_vertex_data)
			{
				face->flags |= G3D_FLAG_FAC_TEXMAP;
				face->tex_image = face->material->tex_image;
				face->tex_vertex_count = 3;
				face->tex_vertex_data = g_new0(G3DFloat, 6);
				for(j = 0; j < 3; j ++)
				{
					face->tex_vertex_data[j * 2 + 0] = object->tex_vertex_data[
						face->vertex_indices[j] * 2 + 0];
					face->tex_vertex_data[j * 2 + 1] = object->tex_vertex_data[
						face->vertex_indices[j] * 2 + 1];
				}
			} /* textured face */
		} /* material != NULL */

		if((i % 1000) == 0) x3ds_update_progress(global, parent->level);
	} /* 0..nfaces */

	return TRUE;
}

/* texture vertices */
gboolean x3ds_cb_0x4140(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i;

	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	object->tex_vertex_count = g3d_stream_read_int16_le(global->stream);
	parent->nb -= 2;

	object->tex_vertex_data = g_new0(G3DFloat, object->tex_vertex_count * 2);

	for(i = 0; i < object->tex_vertex_count; i ++)
	{
		object->tex_vertex_data[i * 2 + 0] = g3d_stream_read_float_le(global->stream);
		object->tex_vertex_data[i * 2 + 1] = g3d_stream_read_float_le(global->stream);
		parent->nb -= 8;

		if((i % 1000) == 0) x3ds_update_progress(global, parent->level);
	}

	return TRUE;
}

/* smoothing groups */
gboolean x3ds_cb_0x4150(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	G3DFace *face;
	GSList *oface;
	gint32 i, j, k, n=0, polynum, group;
	guint32 *smooth_list;
	G3DFloat *pnormal_list, *vertex_normal_buf;
	G3DFloat a[3],b[3], *p0,*p1,*p2,*r;

	/* read data */
	object = (G3DObject *)parent->object;
	g_return_val_if_fail(object, FALSE);

	oface=object->faces;
	polynum=0;
	for(oface = object->faces; oface != NULL ; oface = oface->next)
		polynum++; /* count polygons */

	/* polygon normal list */
	pnormal_list = g_new(float, 3 * polynum);
	/* normals per vertice */
	vertex_normal_buf = g_new0(float, 3 * object->vertex_count);

	smooth_list = g_new(guint32, polynum);

	for(i = 0 ; i < polynum ; i ++)
		smooth_list[i] = g3d_stream_read_int32_le(global->stream);

	parent->nb -= polynum * 4;
	/* first, we calculate the normal by the polygon vertices (just vector
	 * product) */
	i = 0;
	for(oface = object->faces; oface != NULL ; oface=oface->next)
	{
		face = (G3DFace *)oface->data;
		r = &(pnormal_list[i*3]);
		p0 = &(object->vertex_data[3 * face->vertex_indices[0]]);
		p1 = &(object->vertex_data[3 * face->vertex_indices[1]]);
		p2 = &(object->vertex_data[3 * face->vertex_indices[2]]);

		a[0]=p1[0] - p0[0];
		a[1]=p1[1] - p0[1];
		a[2]=p1[2] - p0[2];
		b[0]=p2[0] - p0[0];
		b[1]=p2[1] - p0[1];
		b[2]=p2[2] - p0[2];

		g3d_vector_normal(a[0], a[1], a[2], b[0], b[1], b[2],
			&r[0], &r[1], &r[2]);

		g3d_vector_unify(&r[0], &r[1], &r[2]);

		face->flags |= G3D_FLAG_FAC_NORMALS;
		i ++;
	}

	do {
		/* find a suitable group. -1 means we've already taken care */
		group = -1;
		for(i = 0; i < polynum; i ++)
			if((group = smooth_list[i]) != -1) /* found a group */
				break;
		/* handle this group */
		if(group != -1)
		{
			/* SMOOTH
			 *  we add normals of the polygons's vertices so each vertex will
			 *  finally have
			 *  the sum of the polygons normals where the vertex is part of.
             *
			 * run0: clear the vertex_normal_buf for this group */
			for(i = 0; i < object->vertex_count * 3; i ++)
				vertex_normal_buf[i] = 0.0;
			/* run1: add normals on themselves into the vertex_normal_buf */
			i = 0;
			for(oface = object->faces; oface != NULL ; oface = oface->next)
			{
				face = (G3DFace *) oface->data;
				if(smooth_list[i] == group)
				{
					/* for all 3 vertices of the polygon */
					for(j = 0; j < 3; j ++)
					{
						k = face->vertex_indices[j];
						for(n = 0; n < 3; n ++)
							vertex_normal_buf[k * 3 + n] +=
								pnormal_list[i * 3 + n];
					}
				}
				i ++;
			}
			i = 0;
			/* run2: apply to the final vertex buffer */
			for(oface = object->faces; oface != NULL ; oface = oface->next)
			{
				face = (G3DFace *)oface->data;
				if(smooth_list[i] == group)
				{
					face->normals = g_new(G3DFloat, 9);
					for(j = 0; j < 3; j ++)
					{
						k = face->vertex_indices[j];

						g3d_vector_unify(
							&(vertex_normal_buf[k * 3 + 0]),
							&(vertex_normal_buf[k * 3 + 1]),
							&(vertex_normal_buf[k * 3 + 2]));

						if(vertex_normal_buf[k * 3 + 0] != 0.0F)
							/* finally, we save the normal in our normal
							 * buffer */
							memcpy(face->normals + j * 3,
								vertex_normal_buf + k * 3,
								sizeof(G3DFloat) * 3);
						else
							/* use the pbuf normal */
							memcpy(face->normals + j * 3,
								pnormal_list + i * 3,
								sizeof(G3DFloat) * 3);

					}
					smooth_list[i] = -1; /* finished this polygon */
				}
				i++;
			}
			/* SMOOTH END */
		}
	} while (group != -1);

	g_free(pnormal_list);
	g_free(vertex_normal_buf);
	g_free(smooth_list);
	return TRUE;
}

/* mesh matrix */
gboolean x3ds_cb_0x4160(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DFloat matrix[16];
	gint32 i;
	G3DFloat det;

	g3d_matrix_identity(matrix);
	for(i = 0; i < 12; i ++)
		matrix[(i / 3) * 4 + (i % 3)] = g3d_stream_read_float_le(global->stream);
	parent->nb -= 48;

	det = g3d_matrix_determinant(matrix);

	g3d_matrix_dump(matrix);
	g_debug("det: %f", det);


	if(det < 0.0) {
#if 0
		G3DFloat scale[16];
		g3d_matrix_identity(scale);
		g3d_matrix_scale(-1.0, 1.0, 1.0, scale);
		g3d_matrix_multiply(scale, matrix, matrix);
		g3d_matrix_dump(matrix);
#endif
#define X3DS_MESH_TRANSFORM 0
#if X3DS_MESH_TRANSFORM
		object = parent->object;
		if(object) {
			object->transformation = g_new0(G3DTransformation, 1);
			memcpy(object->transformation->matrix, matrix, 16 * sizeof(G3DFloat));
		}

#if 0
		if(parent->object) {
			g3d_object_transform(parent->object, matrix);
		}
#endif
#endif
	}
	return TRUE;
}

/* material name */
gboolean x3ds_cb_0xA000(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	gchar buffer[1024];

	g_return_val_if_fail(parent->object, FALSE);

	parent->nb -= x3ds_read_cstr(global->stream, buffer);
	material = (G3DMaterial *)(parent->object);

	material->name = g_strdup(buffer);

	return TRUE;
}

/* two sided material */
gboolean x3ds_cb_0xA081(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	material->flags |= G3D_FLAG_MAT_TWOSIDE;

	return TRUE;
}

/* texture map name */
gboolean x3ds_cb_0xA300(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DImage *image;
	gchar buffer[512];

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	parent->nb -= x3ds_read_cstr(global->stream, buffer);

	switch(parent->id)
	{
		case 0xA200: /* texture map */
			material->tex_image = g3d_texture_load_cached(global->context,
				global->model, buffer);
			if(material->tex_image)
			{
				g3d_texture_flip_y(material->tex_image);
				material->tex_image->tex_id = ++ global->max_tex_id;
			}
			break;

		case 0xA210: /* opacity map */
			image = g3d_texture_load(global->context, buffer);
			if(image != NULL)
			{
				g3d_texture_flip_y(image);
				material->tex_image = g3d_texture_merge_alpha(
					material->tex_image, image);
				g3d_texture_free(image);
			}
			break;

		case 0xA220: /* reflection map */
			/* TODO: implement */
			break;

		case 0xA230: /* bump map */
			/* TODO: implement */
			break;

		default:
#if DEBUG > 0
			g_warning("[3DS] unhandled texture name in 0x%04X", parent->id);
#endif

			break;
	}

	return TRUE;
}

/* texture map scale u */
gboolean x3ds_cb_0xA354(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DImage *image;
	G3DFloat scale;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	image = material->tex_image;
	g_return_val_if_fail(image, FALSE);

	scale = g3d_stream_read_float_le(global->stream);
	parent->nb -= 4;

	image->tex_scale_u = scale;
#if DEBUG > 3
	g_debug("[3DS] scale_u: %f", image->tex_scale_u);
#endif

	return TRUE;
}

/* texture map scale v */
gboolean x3ds_cb_0xA356(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DImage *image;
	G3DFloat scale;

	material = (G3DMaterial *)parent->object;
	g_return_val_if_fail(material, FALSE);

	image = material->tex_image;
	g_return_val_if_fail(image, FALSE);

	scale = g3d_stream_read_float_le(global->stream);
	parent->nb -= 4;

	image->tex_scale_v = scale;
#if DEBUG > 3
	g_debug("[3DS] scale_v: %f", image->tex_scale_v);
#endif

	return TRUE;
}

/* material */
gboolean x3ds_cb_0xAFFF(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DMaterial *material;
	G3DObject *object;

	material = g3d_material_new();

	if(parent->object)
	{
		object = (G3DObject *)parent->object;
		object->materials = g_slist_append(object->materials, material);
	}
	else
	{
		global->model->materials = g_slist_append(global->model->materials,
			material);
	}

	parent->object = material;

	return TRUE;
}

/* keyframe data header */
gboolean x3ds_cb_0xB00A(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 rev, len;
	gchar buffer[512];

	rev = g3d_stream_read_int16_le(global->stream);
	parent->nb -= 2;
	parent->nb -= x3ds_read_cstr(global->stream, buffer);
	len = g3d_stream_read_int16_le(global->stream);
	parent->nb -= 2;

#if DEBUG > 0
	g_debug("[3DS] keyframe data: r%d, %d frames, \"%s\"",
		rev, len, buffer);
#endif
	return TRUE;
}

/* node header */
gboolean x3ds_cb_0xB010(x3ds_global_data *global, x3ds_parent_data *parent)
{
	GSList *olist;
	G3DObject *object;
	gchar buffer[512];

	parent->nb -= x3ds_read_cstr(global->stream, buffer);
#if DEBUG > 3
	g_debug("[3DS] NODE_HDR: %s", buffer);
#endif

	/* find object by name */
	olist = global->model->objects;
	while(olist)
	{
		object = (G3DObject *)olist->data;
		if(strcmp(object->name, buffer) == 0)
		{
			parent->level_object = object;
			break;
		}
		olist = olist->next;
	}

	g3d_stream_read_int16_le(global->stream); /* flags 1 */
	g3d_stream_read_int16_le(global->stream); /* flags 2 */
	g3d_stream_read_int16_le(global->stream); /* ? */
	parent->nb -= 6;

	return TRUE;
}

/* pivot */
gboolean x3ds_cb_0xB013(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
#if 0
	gint32 i;
#endif
	G3DFloat x, y, z;

	object = parent->level_object;
	if(object == NULL) return FALSE;

	x = g3d_stream_read_float_le(global->stream);
	y = g3d_stream_read_float_le(global->stream);
	z = g3d_stream_read_float_le(global->stream);
	parent->nb -= 12;

#if DEBUG > 3
	g_debug("[3DS]: PIVOT: (%.2f,%.2f,%.2f)", x, y, z);
#endif

	return TRUE;
}

#define X3DS_ENABLE_POS_TRACK_TAG 0

/* position tracking tag */
gboolean x3ds_cb_0xB020(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i, flags, fflags, nkeys, fnum;
#if X3DS_ENABLE_POS_TRACK_TAG
	gint32 j;
#endif
	G3DFloat x, y, z;

	object = parent->level_object;
	if(object == NULL) return FALSE;

	flags = g3d_stream_read_int16_le(global->stream);
	g3d_stream_skip(global->stream, 8);
	nkeys = g3d_stream_read_int32_le(global->stream);

	parent->nb -= 14;

	for(i = 0; i < nkeys; i ++)
	{
		fnum = g3d_stream_read_int32_le(global->stream);
		fflags = g3d_stream_read_int16_le(global->stream);
		parent->nb -= 6;

		if(fflags & X3DS_FLAG_TENSION)
		{
			g3d_stream_read_float_le(global->stream);
			parent->nb -= 4;
		}
		if(fflags & X3DS_FLAG_CONTINUITY)
		{
			g3d_stream_read_float_le(global->stream);
			parent->nb -= 4;
		}
		if(fflags & X3DS_FLAG_BIAS)
		{
			g3d_stream_read_float_le(global->stream);
			parent->nb -= 4;
		}
		if(fflags & X3DS_FLAG_EASE_TO)
		{
			g3d_stream_read_float_le(global->stream);
			parent->nb -= 4;
		}
		if(fflags & X3DS_FLAG_EASE_FROM)
		{
			g3d_stream_read_float_le(global->stream);
			parent->nb -= 4;
		}

		x = g3d_stream_read_float_le(global->stream);
		y = g3d_stream_read_float_le(global->stream);
		z = g3d_stream_read_float_le(global->stream);
		parent->nb -= 12;
#if DEBUG > 3
		g_debug("[3DS]: POS_TRACK_TAG: frame %d: (%.2f,%.2f,%.2f) (0x%X) "
			"object: %s",
			fnum, x, y, z, fflags, object->name);
#endif

#if X3DS_ENABLE_POS_TRACK_TAG
		if(fnum == 0)
		{
#if 1
			for(j = 0; j < object->vertex_count; j ++)
			{
				object->vertex_data[j * 3 + 0] -= x;
				object->vertex_data[j * 3 + 1] -= y;
				object->vertex_data[j * 3 + 2] -= z;
			}
#endif
		}
#endif
	}

	return TRUE;
}

/* rotation tracking tag */
gboolean x3ds_cb_0xB021(x3ds_global_data *global, x3ds_parent_data *parent)
{
	G3DObject *object;
	gint32 i, j, flags, nkeys, fnum;
	G3DFloat x, y, z, rot;
	G3DFloat matrix[16];

	object = parent->level_object;
	if(object == NULL) return FALSE;

	flags = g3d_stream_read_int16_le(global->stream);
	g3d_stream_skip(global->stream, 8);
	nkeys = g3d_stream_read_int16_le(global->stream);
	g3d_stream_read_int16_le(global->stream);
	parent->nb -= 14;

	for(i = 0; i < nkeys; i ++)
	{
		fnum = g3d_stream_read_int16_le(global->stream);
		g3d_stream_read_int32_le(global->stream);
		parent->nb -= 6;

		rot = g3d_stream_read_float_le(global->stream);
		x = g3d_stream_read_float_le(global->stream);
		y = g3d_stream_read_float_le(global->stream);
		z = g3d_stream_read_float_le(global->stream);
		parent->nb -= 16;
#if DEBUG > 3
		g_debug(
			"[3DS]: ROT_TRACK_TAG: frame %d: (%.2f,%.2f,%.2f), %.2f rad",
			fnum, x, y, z, rot);
#endif
		if(fnum == -1)
		{
#if 1
			g3d_matrix_identity(matrix);
			g3d_matrix_rotate(rot, x, y, z, matrix);

			for(j = 0; j < object->vertex_count; j ++)
			{
				g3d_vector_transform(
					&(object->vertex_data[j * 3 + 0]),
					&(object->vertex_data[j * 3 + 1]),
					&(object->vertex_data[j * 3 + 2]),
					matrix);
			}
#endif
		}
	}

	return TRUE;

}

/* node id */
gboolean x3ds_cb_0xB030(x3ds_global_data *global, x3ds_parent_data *parent)
{
	gint32 id;

	id = g3d_stream_read_int16_le(global->stream);
	parent->nb -= 2;
#if DEBUG > 3
	g_debug("[3DS] NODE_ID: %d", id);
#endif

	return TRUE;
}
