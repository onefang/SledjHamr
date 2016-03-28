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

#include <math.h>
#include <string.h>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

#include <g3d/types.h>
#include <g3d/vector.h>
#include <g3d/primitive.h>
#include <g3d/face.h>

EAPI
G3DObject *g3d_primitive_cube(G3DFloat width, G3DFloat height, G3DFloat depth,
	G3DMaterial *material)
{
	g_warning("g3d_primitive_cube is deprecated - please update your "
		"sources to use g3d_primitive_box instead");
	return g3d_primitive_box(width, height, depth, material);
}

EAPI
G3DObject *g3d_primitive_box(G3DFloat width, G3DFloat height, G3DFloat depth,
	G3DMaterial *material)
{
	G3DObject *object;
	G3DFace *face;
	G3DVector nx, ny, nz;
	gint32 faces[6][4] = {
		{ 0, 1, 2, 3 },
		{ 4, 5, 6, 7 },
		{ 0, 1, 5, 4 },
		{ 2, 3, 7, 6 },
		{ 1, 2, 6, 5 },
		{ 0, 4, 7, 3 }};
	gint32 i, j;

	object = g_new0(G3DObject, 1);

	object->vertex_count = 8;
	object->vertex_data = g3d_vector_new(3, object->vertex_count);

	object->vertex_data[0 * 3 + 0] = -(width / 2);
	object->vertex_data[0 * 3 + 1] = -(height / 2);
	object->vertex_data[0 * 3 + 2] = -(depth / 2);

	object->vertex_data[1 * 3 + 0] = -(width / 2);
	object->vertex_data[1 * 3 + 1] = -(height / 2);
	object->vertex_data[1 * 3 + 2] = (depth / 2);

	object->vertex_data[2 * 3 + 0] = (width / 2);
	object->vertex_data[2 * 3 + 1] = -(height / 2);
	object->vertex_data[2 * 3 + 2] = (depth / 2);

	object->vertex_data[3 * 3 + 0] = (width / 2);
	object->vertex_data[3 * 3 + 1] = -(height / 2);
	object->vertex_data[3 * 3 + 2] = -(depth / 2);

	object->vertex_data[4 * 3 + 0] = -(width / 2);
	object->vertex_data[4 * 3 + 1] = (height / 2);
	object->vertex_data[4 * 3 + 2] = -(depth / 2);

	object->vertex_data[5 * 3 + 0] = -(width / 2);
	object->vertex_data[5 * 3 + 1] = (height / 2);
	object->vertex_data[5 * 3 + 2] = (depth / 2);

	object->vertex_data[6 * 3 + 0] = (width / 2);
	object->vertex_data[6 * 3 + 1] = (height / 2);
	object->vertex_data[6 * 3 + 2] = (depth / 2);

	object->vertex_data[7 * 3 + 0] = (width / 2);
	object->vertex_data[7 * 3 + 1] = (height / 2);
	object->vertex_data[7 * 3 + 2] = -(depth / 2);

	for(i = 0; i < 6; i ++) {
		face = g_new0(G3DFace, 1);
		face->vertex_count = 4;
		face->vertex_indices = g_new0(guint32, 4);
		face->normals = g3d_vector_new(3, 4);
		face->tex_vertex_count = 4;
		face->tex_vertex_data = g3d_vector_new(2, 4);
		for(j = 0; j < 4; j ++) {
			face->vertex_indices[j] = faces[i][j];
		}

		/* add normals */
		g3d_face_get_normal(face, object, &nx, &ny, &nz);
		for(j = 0; j < 4; j ++) {
			face->normals[j * 3 + 0] = nx;
			face->normals[j * 3 + 1] = ny;
			face->normals[j * 3 + 2] = nz;
		}

		face->flags |= G3D_FLAG_FAC_NORMALS;

		/* add default texture coordinates */
		face->tex_vertex_data[0 * 2 + 0] = 0;
		face->tex_vertex_data[0 * 2 + 1] = 0;
		face->tex_vertex_data[1 * 2 + 0] = 1;
		face->tex_vertex_data[1 * 2 + 1] = 0;
		face->tex_vertex_data[2 * 2 + 0] = 1;
		face->tex_vertex_data[2 * 2 + 1] = 1;
		face->tex_vertex_data[3 * 2 + 0] = 0;
		face->tex_vertex_data[3 * 2 + 1] = 1;

		face->material = material;
		object->faces = g_slist_append(object->faces, face);
	}

	return object;
}

EAPI
G3DObject *g3d_primitive_cylinder(G3DFloat radius, G3DFloat height,
	guint32 sides, gboolean top, gboolean bottom, G3DMaterial *material)
{
	G3DObject *object;
	G3DFace *face;
	guint32 i;

	if(sides < 3)
		return NULL;

	object = g_new0(G3DObject, 1);

	/* vertices */
	object->vertex_count = sides * 2 + 2;
	object->vertex_data = g3d_vector_new(3, object->vertex_count);

	/* 2 rings */
	for(i = 0; i < sides; i ++)
	{
		object->vertex_data[i * 3 + 0] =
		object->vertex_data[(sides + i) * 3 + 0] =
			radius * cos(M_PI * 2 * i / sides);
		object->vertex_data[i * 3 + 1] =
		object->vertex_data[(sides + i) * 3 + 1] =
			radius * sin(M_PI * 2 * i / sides);

		object->vertex_data[i * 3 + 2] = 0.0;
		object->vertex_data[(sides + i) * 3 + 2] = height;
	}

	/* center top & bottom */
	object->vertex_data[sides * 2 * 3 + 0] = 0.0;
	object->vertex_data[sides * 2 * 3 + 1] = 0.0;
	object->vertex_data[sides * 2 * 3 + 2] = 0.0;

	object->vertex_data[sides * 2 * 3 + 3] = 0.0;
	object->vertex_data[sides * 2 * 3 + 4] = 0.0;
	object->vertex_data[sides * 2 * 3 + 5] = height;

	/* ring faces */
	for(i = 0; i < sides; i ++)
	{
		face = g_new0(G3DFace, 1);
		face->material = material;
		face->vertex_count = 4;
		face->vertex_indices = g_new0(guint32, 4);

		face->vertex_indices[0] = i;
		face->vertex_indices[1] = i + sides;

		if(i == (sides - 1))
		{
			face->vertex_indices[2] = sides;
			face->vertex_indices[3] = 0;
		}
		else
		{
			face->vertex_indices[2] = i + sides + 1;
			face->vertex_indices[3] = i + 1;
		}

		/* normals */
		face->flags |= G3D_FLAG_FAC_NORMALS;
		face->normals = g3d_vector_new(3, 4);

		face->normals[0 * 3 + 0] =
		face->normals[1 * 3 + 0] =
			object->vertex_data[i * 3 + 0];
		face->normals[0 * 3 + 1] =
		face->normals[1 * 3 + 1] =
			object->vertex_data[i * 3 + 1];
		face->normals[0 * 3 + 2] = 0.0;
		face->normals[1 * 3 + 2] = 0.0;

		g3d_vector_unify(
			&(face->normals[0 * 3 + 0]),
			&(face->normals[0 * 3 + 1]),
			&(face->normals[0 * 3 + 2]));
		g3d_vector_unify(
			&(face->normals[1 * 3 + 0]),
			&(face->normals[1 * 3 + 1]),
			&(face->normals[1 * 3 + 2]));

		face->normals[2 * 3 + 0] =
		face->normals[3 * 3 + 0] =
			object->vertex_data[face->vertex_indices[2] * 3 + 0];
		face->normals[2 * 3 + 1] =
		face->normals[3 * 3 + 1] =
			object->vertex_data[face->vertex_indices[2] * 3 + 1];
		face->normals[2 * 3 + 2] = 0.0;
		face->normals[3 * 3 + 2] = 0.0;

		g3d_vector_unify(
			&(face->normals[2 * 3 + 0]),
			&(face->normals[2 * 3 + 1]),
			&(face->normals[2 * 3 + 2]));
		g3d_vector_unify(
			&(face->normals[3 * 3 + 0]),
			&(face->normals[3 * 3 + 1]),
			&(face->normals[3 * 3 + 2]));

		object->faces = g_slist_append(object->faces, face);
	}

	if(top)
	{
		for(i = 0; i < sides; i ++)
		{
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 3;
			face->vertex_indices = g_new0(guint32, 3);

			face->vertex_indices[0] = sides + i;
			face->vertex_indices[1] = sides * 2 + 1; /* top center */
			if(i == (sides - 1))
				face->vertex_indices[2] = sides;
			else
				face->vertex_indices[2] = sides + i + 1;

			object->faces = g_slist_append(object->faces, face);
		}
	}

	if(bottom)
	{
		for(i = 0; i < sides; i ++)
		{
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 3;
			face->vertex_indices = g_new0(guint32, 3);

			face->vertex_indices[0] = i;
			face->vertex_indices[1] = sides * 2; /* bottom center */
			if(i == (sides - 1))
				face->vertex_indices[2] = 0;
			else
				face->vertex_indices[2] = i + 1;

			object->faces = g_slist_append(object->faces, face);
		}
	}

	return object;
}

EAPI
G3DObject *g3d_primitive_tube(G3DFloat r_in, G3DFloat r_out, G3DFloat height,
	guint32 sides, gboolean top, gboolean bottom, G3DMaterial *material)
{
	G3DObject *object;
	G3DFace *face;
	guint32 i, j;

	if(sides < 3)
		return NULL;

	object = g_new0(G3DObject, 1);

	/* vertices */
	object->vertex_count = sides * 4;
	object->vertex_data = g3d_vector_new(3, object->vertex_count);

	/*
	 * outer lower     0
	 * outer upper     sides
	 * inner lower     sides * 2
	 * inner upper     sides * 3
	 */

	/* 4 rings */
	for(i = 0; i < sides; i ++)
	{
		/* outer rings */
		object->vertex_data[i * 3 + 0] =
		object->vertex_data[(sides + i) * 3 + 0] =
			r_out * cos(M_PI * 2 * i / sides);
		object->vertex_data[i * 3 + 1] =
		object->vertex_data[(sides + i) * 3 + 1] =
			r_out * sin(M_PI * 2 * i / sides);

		object->vertex_data[i * 3 + 2] = 0.0;
		object->vertex_data[(sides + i) * 3 + 2] = height;

		/* inner rings */
		object->vertex_data[(sides * 2 + i) * 3 + 0] =
		object->vertex_data[(sides * 3 + i) * 3 + 0] =
			r_in * cos(M_PI * 2 * i / sides);
		object->vertex_data[(sides * 2 + i) * 3 + 1] =
		object->vertex_data[(sides * 3 + i) * 3 + 1] =
			r_in * sin(M_PI * 2 * i / sides);

		object->vertex_data[(sides * 2 + i) * 3 + 2] = 0.0;
		object->vertex_data[(sides * 3 + i) * 3 + 2] = height;
	}

	/* ring faces */
	for(i = 0; i < sides; i ++)
	{
		/* outer and inner faces */
		for(j = 0; j < 2; j ++)
		{
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);

			face->vertex_indices[0] = i + sides * (j * 2 + 0);
			face->vertex_indices[1] = i + sides * (j * 2 + 1);

			if(i == (sides - 1))
			{
				face->vertex_indices[2] = sides * (j * 2 + 1);
				face->vertex_indices[3] = sides * (j * 2 + 0);
			}
			else
			{
				face->vertex_indices[2] = i + sides * (j * 2 + 1) + 1;
				face->vertex_indices[3] = i + sides * (j * 2 + 0) + 1;
			}

			/* normals */
			face->flags |= G3D_FLAG_FAC_NORMALS;
			face->normals = g3d_vector_new(3, 4);

			face->normals[0 * 3 + 0] =
			face->normals[1 * 3 + 0] =
				object->vertex_data[face->vertex_indices[0] * 3 + 0] *
				(j ? -1 : 1);
			face->normals[0 * 3 + 1] =
			face->normals[1 * 3 + 1] =
				object->vertex_data[face->vertex_indices[0] * 3 + 1] *
				(j ? -1 : 1);
			face->normals[0 * 3 + 2] = 0.0;
			face->normals[1 * 3 + 2] = 0.0;

			g3d_vector_unify(
				&(face->normals[0 * 3 + 0]),
				&(face->normals[0 * 3 + 1]),
				&(face->normals[0 * 3 + 2]));
			g3d_vector_unify(
				&(face->normals[1 * 3 + 0]),
				&(face->normals[1 * 3 + 1]),
				&(face->normals[1 * 3 + 2]));

			face->normals[2 * 3 + 0] =
			face->normals[3 * 3 + 0] =
				object->vertex_data[face->vertex_indices[2] * 3 + 0] *
				(j ? -1 : 1);
			face->normals[2 * 3 + 1] =
			face->normals[3 * 3 + 1] =
				object->vertex_data[face->vertex_indices[2] * 3 + 1] *
				(j ? -1 : 1);
			face->normals[2 * 3 + 2] = 0.0;
			face->normals[3 * 3 + 2] = 0.0;

			g3d_vector_unify(
				&(face->normals[2 * 3 + 0]),
				&(face->normals[2 * 3 + 1]),
				&(face->normals[2 * 3 + 2]));
			g3d_vector_unify(
				&(face->normals[3 * 3 + 0]),
				&(face->normals[3 * 3 + 1]),
				&(face->normals[3 * 3 + 2]));

			object->faces = g_slist_append(object->faces, face);
		}
	}

	/* top/bottom faces if requested */
	for(i = 0; i < sides; i ++)
	{
		for(j = (bottom ? 0 : 1); j < (top ? 2 : 1); j ++)
		{
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);

			face->vertex_indices[0] = sides * (2 + j) + i; /* inner */
			face->vertex_indices[1] = sides * (0 + j) + i; /* outer */

			if(i == (sides - 1))
			{
				face->vertex_indices[2] = sides * (0 + j); /* outer first */
				face->vertex_indices[3] = sides * (2 + j); /* inner first */
			}
			else
			{
				face->vertex_indices[2] = sides * (0 + j) + i + 1;
				face->vertex_indices[3] = sides * (2 + j) + i + 1;
			}

			object->faces = g_slist_append(object->faces, face);
		}
	}

	return object;
}

EAPI
G3DObject *g3d_primitive_sphere(G3DFloat radius, guint32 vseg, guint32 hseg,
	G3DMaterial *material)
{
	G3DObject *object;
	G3DFace *face;
	GSList *flist;
	gint32 sh, sv, i;
	gdouble x, y, z, u;

	g_return_val_if_fail(vseg >= 2, NULL);
	g_return_val_if_fail(hseg >= 3, NULL);

	object = g_new0(G3DObject, 1);
	object->vertex_count = (vseg - 1) * hseg + 2;
	object->vertex_data = g3d_vector_new(3, object->vertex_count);

	for(sv = 1; sv < vseg; sv ++)
	{
		y = radius * cos(M_PI * sv / vseg);
		u = radius * sin(M_PI * sv / vseg);
		for(sh = 0; sh < hseg; sh ++)
		{
			x = radius * cos(M_PI * 2 * sh / hseg) * u;
			z = radius * sin(M_PI * 2 * sh / hseg) * u;

			object->vertex_data[((sv - 1) * hseg + sh) * 3 + 0] = x;
			object->vertex_data[((sv - 1) * hseg + sh) * 3 + 1] = y;
			object->vertex_data[((sv - 1) * hseg + sh) * 3 + 2] = z;

			if(sv > 1)
			{
				/* first triangle */
				face = g_new0(G3DFace, 1);
				face->material = material;
				face->vertex_count = 3;
				face->vertex_indices = g_new0(guint32, 3);
				face->vertex_indices[0] = (sv - 1) * hseg + sh;
				face->vertex_indices[1] = (sh == (hseg - 1)) ?
					(sv - 1) * hseg :
					(sv - 1) * hseg + sh + 1;
				face->vertex_indices[2] = (sv - 2) * hseg + sh;
				object->faces = g_slist_append(object->faces, face);

				/* second triangle */
				face = g_new0(G3DFace, 1);
				face->material = material;
				face->vertex_count = 3;
				face->vertex_indices = g_new0(guint32, 3);
				face->vertex_indices[0] = (sv - 2) * hseg + sh;
				face->vertex_indices[1] = (sh == (hseg - 1)) ?
					(sv - 1) * hseg :
					(sv - 1) * hseg + sh + 1;
				face->vertex_indices[2] = (sh == (hseg - 1)) ?
					(sv - 2) * hseg :
					(sv - 2) * hseg + sh + 1;
				object->faces = g_slist_append(object->faces, face);
			} /* sv > 1 */
		} /* hseg */
	} /* vseg */

	object->vertex_data[(object->vertex_count - 1) * 3 + 0] = 0;
	object->vertex_data[(object->vertex_count - 1) * 3 + 1] = radius;
	object->vertex_data[(object->vertex_count - 1) * 3 + 2] = 0;

	object->vertex_data[(object->vertex_count - 2) * 3 + 0] = 0;
	object->vertex_data[(object->vertex_count - 2) * 3 + 1] = -radius;
	object->vertex_data[(object->vertex_count - 2) * 3 + 2] = 0;

	for(sh = 0; sh < hseg; sh ++)
	{
		/* top */
		face = g_new0(G3DFace, 1);
		face->material = material;
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);

		face->vertex_indices[0] = object->vertex_count - 1;
		face->vertex_indices[1] = sh;
		face->vertex_indices[2] = (sh == (hseg - 1)) ? 0 : sh + 1;

		object->faces = g_slist_append(object->faces, face);

		/* bottom */
		face = g_new0(G3DFace, 1);
		face->material = material;
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);

		face->vertex_indices[2] = object->vertex_count - 2;
		face->vertex_indices[1] = (vseg - 2) * hseg + sh;
		face->vertex_indices[0] = (sh == (hseg - 1)) ?
			(vseg - 2) * hseg :
			(vseg - 2) * hseg + sh + 1;

		object->faces = g_slist_append(object->faces, face);
	}

	/* generate normals */
	flist = object->faces;
	while(flist)
	{
		face = (G3DFace *)flist->data;
		face->flags |= G3D_FLAG_FAC_NORMALS;
		face->normals = g3d_vector_new(3, face->vertex_count);
		for(i = 0; i < face->vertex_count; i ++)
		{
			face->normals[i * 3 + 0] =
				- object->vertex_data[face->vertex_indices[i] * 3 + 0];
			face->normals[i * 3 + 1] =
				- object->vertex_data[face->vertex_indices[i] * 3 + 1];
			face->normals[i * 3 + 2] =
				- object->vertex_data[face->vertex_indices[i] * 3 + 2];

			g3d_vector_unify(
				&(face->normals[i * 3 + 0]),
				&(face->normals[i * 3 + 1]),
				&(face->normals[i * 3 + 2]));
		}

		flist = flist->next;
	}

	return object;
}

/* vdata: 2-dimensional coordinates: vcnt * 2 * gdouble */
EAPI
G3DObject *g3d_primitive_box_strip_2d(guint32 vcnt, gdouble *vdata,
	gdouble height, gdouble width, G3DMaterial *material)
{
	G3DObject *object;
	G3DFace *face;
	gint32 i, index;
	G3DVector *normals, normal[3];
	G3DFloat r;

	/* create object & helpers */
	object = g_new0(G3DObject, 1);
	normals = g3d_vector_new(3, vcnt);
	object->vertex_count = vcnt * 4;
	object->vertex_data = g3d_vector_new(3, object->vertex_count);

	/* generate normals */
	for(i = 0; i < vcnt; i ++) {
		/* normal data */
		g3d_vector_normal(
			/* vector 1 */
			vdata[(i + 1) * 2 + 0] - vdata[i * 2 + 0],
			0.0,
			vdata[(i + 1) * 2 + 1] - vdata[i * 2 + 1],
			/* vector 2 */
			0.0, 1.0, 0.0,
			/* resulting vector */
			&(normals[i * 3 + 0]),
			&(normals[i * 3 + 1]),
			&(normals[i * 3 + 2]));
		g3d_vector_unify(
			&(normals[i * 3 + 0]),
			&(normals[i * 3 + 1]),
			&(normals[i * 3 + 2]));
	}

	/* radius */
	r = width / 2.0;

	/* generate vertices */
	for(i = 0; i < vcnt; i ++) {
		/* average normal for vertex */
		if(i == 0) {
			normal[0] = normals[i * 3 + 0];
			normal[1] = normals[i * 3 + 1];
			normal[2] = normals[i * 3 + 2];
		} else {
			normal[0] = (normals[i * 3 + 0] + normals[(i - 1) * 3 + 0]) / 2.0;
			normal[1] = (normals[i * 3 + 1] + normals[(i - 1) * 3 + 1]) / 2.0;
			normal[2] = (normals[i * 3 + 2] + normals[(i - 1) * 3 + 2]) / 2.0;
			g3d_vector_unify(&(normals[0]), &(normals[1]), &(normals[2]));
		}

		/* vertex data */
		/* v0 */
		index = i * 4;
		object->vertex_data[index * 3 + 0] = vdata[i * 2 + 0] + normals[0] * r;
		object->vertex_data[index * 3 + 1] = height;
		object->vertex_data[index * 3 + 2] = vdata[i * 2 + 1] + normals[2] * r;
		/* v1 */
		index ++;
		object->vertex_data[index * 3 + 0] = vdata[i * 2 + 0] - normals[0] * r;
		object->vertex_data[index * 3 + 1] = height;
		object->vertex_data[index * 3 + 2] = vdata[i * 2 + 1] - normals[2] * r;
		/* v2 */
		index ++;
		object->vertex_data[index * 3 + 0] = vdata[i * 2 + 0] - normals[0] * r;
		object->vertex_data[index * 3 + 1] = 0.0;
		object->vertex_data[index * 3 + 2] = vdata[i * 2 + 1] - normals[2] * r;
		/* v3 */
		index ++;
		object->vertex_data[index * 3 + 0] = vdata[i * 2 + 0] + normals[0] * r;
		object->vertex_data[index * 3 + 1] = 0.0;
		object->vertex_data[index * 3 + 2] = vdata[i * 2 + 1] + normals[2] * r;

		if(i > 0) {
			/* generate faces */
			/* upper face */
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);
			face->vertex_indices[0] = (i * 4) + 0;
			face->vertex_indices[1] = (i * 4) + 1;
			face->vertex_indices[2] = (i * 4) - 3;
			face->vertex_indices[3] = (i * 4) - 4;
			object->faces = g_slist_prepend(object->faces, face);
			/* lower face */
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);
			face->vertex_indices[0] = (i * 4) + 3;
			face->vertex_indices[1] = (i * 4) - 1;
			face->vertex_indices[2] = (i * 4) - 2;
			face->vertex_indices[3] = (i * 4) + 2;
			object->faces = g_slist_prepend(object->faces, face);
			/* "front" face */
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);
			face->vertex_indices[0] = (i * 4) + 1;
			face->vertex_indices[1] = (i * 4) + 2;
			face->vertex_indices[2] = (i * 4) - 2;
			face->vertex_indices[3] = (i * 4) - 3;
			object->faces = g_slist_prepend(object->faces, face);
			/* "back" face */
			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);
			face->vertex_indices[0] = (i * 4) - 4;
			face->vertex_indices[1] = (i * 4) - 1;
			face->vertex_indices[2] = (i * 4) + 3;
			face->vertex_indices[3] = (i * 4) + 0;
			object->faces = g_slist_prepend(object->faces, face);
		}
	}
	/* clean up */
	g_free(normals);

	return object;
}

EAPI
G3DObject *g3d_primitive_mesh(guint32 m, guint32 n, gboolean wrap_m,
	gboolean wrap_n, G3DMaterial *material)
{
	G3DObject *object;
	G3DFace *face;
	gint32 x, y;

	object = g_new0(G3DObject, 1);
	object->vertex_count = m * n;
	object->vertex_data = g3d_vector_new(3, object->vertex_count);

	for(y = 0; y < (n - 1); y ++) {
		for(x = 0; x < (m - 1); x ++) {
			face = g3d_face_new_tri(material,
				y * m + x, y * m + x + 1, (y + 1) * m + x);
			object->faces = g_slist_prepend(object->faces, face);

			face = g3d_face_new_tri(material,
				(y + 1) * m + x, y * m + x + 1, (y + 1) * m + x + 1);
			object->faces = g_slist_prepend(object->faces, face);
		}
		if(wrap_n) {
			face = g3d_face_new_tri(material,
				(y + 1) * m - 1, y * m, (y + 2) * m - 1);
			object->faces = g_slist_prepend(object->faces, face);

			face = g3d_face_new_tri(material,
				(y + 2) * m - 1, y * m, (y + 1) * m);
			object->faces = g_slist_prepend(object->faces, face);
		}
	}
	if(wrap_m) {
		for(x = 0; x < (m - 1); x ++) {
			face = g3d_face_new_tri(material,
				(n - 1) * m + x, (n - 1) * m + x + 1, x);
			object->faces = g_slist_prepend(object->faces, face);

			face = g3d_face_new_tri(material,
				x, (n - 1) * m + x + 1, x + 1);
			object->faces = g_slist_prepend(object->faces, face);
		}
		if(wrap_n) {
			face = g3d_face_new_tri(material,
				n * m - 1, (n - 1) * m, m - 1);
			object->faces = g_slist_prepend(object->faces, face);

			face = g3d_face_new_tri(material,
				m - 1, (n - 1) * m, 0);
			object->faces = g_slist_prepend(object->faces, face);
		}
	}
	return object;
}

