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

#ifndef _G3D_MATRIX_H
#define _G3D_MATRIX_H

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:matrix
 * @short_description: Matrix manipulation and calculation
 * @include: g3d/matrix.h
 *
 * Matrices in libg3d have the following layout:
 *
 * G3DMatrix matrix[16]:
 *
 * matrix[col * 4 + row] = f;
 */

/**
 * g3d_matrix_identity:
 * @matrix: 4x4 matrix (float[16])
 *
 * Sets the given matrix to the identity matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_matrix_identity(G3DMatrix *matrix);

/**
 * g3d_matrix_new:
 *
 * Create a new matrix. It is also set to the identity matrix.
 *
 * Returns: the new matrix
 */
_G3D_STATIC_INLINE G3DMatrix *g3d_matrix_new(void) {
	G3DMatrix *matrix = g_new(G3DMatrix, 16);
	g3d_matrix_identity(matrix);
	return matrix;
}

/**
 * g3d_matrix_free:
 * @matrix: the matrix to free
 *
 * Free the memory allocated by a matrix.
 */
_G3D_STATIC_INLINE void g3d_matrix_free(G3DMatrix *matrix) {
	g_free(matrix);
}

/**
 * g3d_matrix_multiply:
 * @m1: first matrix
 * @m2: second matrix
 * @rm: resulting matrix
 *
 * Multiplies the matrixes.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_matrix_multiply(G3DMatrix *m1, G3DMatrix *m2, G3DMatrix *rm);

/**
 * g3d_matrix_translate:
 * @x: x translation
 * @y: y translation
 * @z: z translation
 * @rm: resulting matrix
 *
 * Adds a translation to the the matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_matrix_translate(G3DFloat x, G3DFloat y, G3DFloat z,
	G3DMatrix *rm);

/**
 * g3d_matrix_rotate:
 * @angle: rotation angle
 * @ax: x component of rotation axis
 * @ay: y component of rotation axis
 * @az: z component of rotation axis
 * @rm: resulting matrix
 *
 * Adds a rotation to the matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_matrix_rotate(G3DFloat angle, G3DFloat ax, G3DFloat ay,
	G3DFloat az, G3DMatrix *rm);

/**
 * g3d_matrix_rotate_xyz
 * @rx: rotation around x axis
 * @ry: rotation around y axis
 * @rz: rotation around z axis
 * @rm: resulting matrix
 *
 * Adds a rotation around the 3 coordinate system axes to the matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_matrix_rotate_xyz(G3DFloat rx, G3DFloat ry, G3DFloat rz,
	G3DMatrix *rm);

/**
 * g3d_matrix_scale:
 * @x: x factor
 * @y: y factor
 * @z: z factor
 * @rm: resulting matrix
 *
 * Adds a scaling to the matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_matrix_scale(G3DFloat x, G3DFloat y, G3DFloat z, G3DMatrix *rm);

/**
 * g3d_matrix_transpose:
 * @matrix: the matrix
 *
 * Transposes the matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_matrix_transpose(G3DMatrix *matrix);

/**
 * g3d_matrix_determinant:
 * @matrix: the matrix
 *
 * Calculate the determinant of the matrix (FIXME: not verified).
 *
 * Returns: the determinant.
 */
EAPI
G3DFloat g3d_matrix_determinant(G3DMatrix *matrix);

/**
 * g3d_matrix_dump:
 * @matrix: the matrix
 *
 * If debugging is enabled, this function dump the matrix to stderr.
 *
 * Returns: TRUE if matrix is dumped, FALSE else
 */
EAPI
gboolean g3d_matrix_dump(G3DMatrix *matrix);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* _G3D_MATRIX_H */

