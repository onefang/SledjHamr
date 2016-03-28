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

#ifndef __G3D_VECTOR_H__
#define __G3D_VECTOR_H__

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:vector
 * @short_description: Vector manipulation and calculation
 * @include: g3d/vector.h
 *
 * A vector is a one-dimensional array of floating point data.
 *
 * Declare it as statically as:
 *
 * G3DVector vector[3];
 *
 * or allocate it dynamically with:
 *
 * G3DVector *vector = g3d_vector_new(3, 1);
 */

/**
 * g3d_vector_new:
 * @size: number of items in one vector
 * @n: number of vectors to allocate
 *
 * Allocate memory for a number of vectors.
 *
 * Returns: newly allocated vectors
 */
_G3D_STATIC_INLINE G3DVector *g3d_vector_new(guint32 size, guint32 n) {
	return g_new0(G3DVector, size * n);
}

/**
 * g3d_vector_free:
 * @vector: vector to free
 *
 * Free memory allocated for vector.
 */
_G3D_STATIC_INLINE void g3d_vector_free(G3DVector *vector) {
	g_free(vector);
}

/**
 * g3d_vector_normal:
 * @ax: x component first vector
 * @ay: y component first vector
 * @az: z component first vector
 * @bx: x component second vector
 * @by: y component second vector
 * @bz: z component second vector
 * @nx: x component resulting normal
 * @ny: y component resulting normal
 * @nz: z component resulting normal
 *
 * calculate the normal from a plane defined by two vectors
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_vector_normal(G3DFloat ax, G3DFloat ay, G3DFloat az,
	G3DFloat bx, G3DFloat by, G3DFloat bz,
	G3DFloat *nx, G3DFloat *ny, G3DFloat *nz);

/**
 * g3d_vector_unify:
 * @nx: x component of vector
 * @ny: y component of vector
 * @nz: z component of vector
 *
 * Transforms the given vector to the unit vector.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_vector_unify(G3DFloat *nx, G3DFloat *ny, G3DFloat *nz);

/**
 * g3d_vector_transform:
 * @x: x component of vector
 * @y: y component of vector
 * @z: z component of vector
 * @matrix: transformation matrix (4x4)
 *
 * Transforms the given vector corresponding to the given matrix
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_vector_transform(G3DFloat *x, G3DFloat *y, G3DFloat *z,
	G3DMatrix *matrix);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_VECTOR_H__ */

