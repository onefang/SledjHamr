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

#ifndef _G3D_FACE_H
#define _G3D_FACE_H

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:face
 * @short_description: Face manipulation
 * @include: g3d/face.h
 *
 * A face is plane bordered by at least 3 vertices.
 */


/**
 * g3d_face_free:
 * @face: the face to free
 *
 * Frees all memory allocated for this face.
 */
EAPI
void g3d_face_free(G3DFace *face);

/**
 * g3d_face_get_normal:
 * @face: face to calculate normal of
 * @object: object containing vertices of face
 * @nx: x component of resulting normal
 * @ny: y component of resulting normal
 * @nz: z component of resulting normal
 *
 * calculates the normal of a face.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_face_get_normal(G3DFace *face, G3DObject *object,
	G3DFloat *nx, G3DFloat *ny, G3DFloat *nz);

/**
 * g3d_face_new_tri:
 * @material: material to use for face
 * @i1: index of first vertex
 * @i2: index of second vertex
 * @i3: index of third vertex
 *
 * creates a new triangle face.
 *
 * Returns: a new face
 */
_G3D_STATIC_INLINE G3DFace *g3d_face_new_tri(G3DMaterial *material,
	guint32 i1, guint32 i2, guint32 i3)
{
	G3DFace *face;

	face = g_new0(G3DFace, 1);
	face->material = material;
	face->vertex_count = 3;
	face->vertex_indices = g_new0(guint32, 3);
	face->vertex_indices[0] = i1;
	face->vertex_indices[1] = i2;
	face->vertex_indices[2] = i3;

	return face;
}

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* _G3D_FACE_H */

