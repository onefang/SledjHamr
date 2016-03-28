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
#include <g3d/vector.h>

#define VERTEX_ITEM(a, b) \
	object->vertex_data[face->vertex_indices[a] * 3 + (b)]

EAPI
void g3d_face_free(G3DFace *face)
{
	if(face->vertex_indices)
		g_free(face->vertex_indices);
	g_free(face);
}

EAPI
gboolean g3d_face_get_normal(G3DFace *face, G3DObject *object,
	G3DVector *nx, G3DVector *ny, G3DVector *nz)
{
	guint32 n = face->vertex_count - 1;

	return g3d_vector_normal(
		/* ax, ay, az */
		VERTEX_ITEM(1, 0) - VERTEX_ITEM(0, 0),
		VERTEX_ITEM(1, 1) - VERTEX_ITEM(0, 1),
		VERTEX_ITEM(1, 2) - VERTEX_ITEM(0, 2),
		/* bx, by, bz */
		VERTEX_ITEM(n, 0) - VERTEX_ITEM(0, 0),
		VERTEX_ITEM(n, 1) - VERTEX_ITEM(0, 1),
		VERTEX_ITEM(n, 2) - VERTEX_ITEM(0, 2),
		/* nx, ny, nz */
		nx, ny, nz);
}

#undef VERTEX_ITEM

