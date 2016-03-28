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
#include <g3d/types.h>

EAPI
gboolean g3d_vector_normal(G3DVector ax, G3DVector ay, G3DVector az,
	G3DVector bx, G3DVector by, G3DVector bz,
	G3DVector *nx, G3DVector *ny, G3DVector *nz)
{
	*nx = ay * bz - az * by;
	*ny = az * bx - ax * bz;
	*nz = ax * by - ay * bx;

	return TRUE;
}

EAPI
gboolean g3d_vector_unify(G3DVector *nx, G3DVector *ny, G3DVector *nz)
{
	G3DFloat r;

	r = sqrt(*nx * *nx + *ny * *ny + *nz * *nz);
	if(r == 0.0F)
		*nx = *ny = *nz = 0.0F;
	else {
		*nx /= r;
		*ny /= r;
		*nz /= r;
	}

	return TRUE;
}

EAPI
gboolean g3d_vector_transform(G3DVector *x, G3DVector *y, G3DVector *z,
	G3DMatrix *matrix)
{
	G3DVector vector[4], result[4];
	guint32 i, k;

	vector[0] = *x;
	vector[1] = *y;
	vector[2] = *z;
	vector[3] = 1.0;

	for(i = 0; i < 4; i ++) {
		result[i] = 0.0;

		for(k = 0; k < 4; k ++)
#if 0
			result[i] += matrix[i * 4 + k] * vector[k];
#else
			result[i] += matrix[k * 4 + i] * vector[k];
#endif
	}

	*x = result[0];
	*y = result[1];
	*z = result[2];

	return TRUE;
}

