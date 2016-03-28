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
#include <glib.h>
#include <g3d/vector.h>

EAPI
gboolean g3d_matrix_identity(G3DMatrix *matrix)
{
	static G3DMatrix identity[16] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0 };

	memcpy(matrix, identity, sizeof(G3DMatrix) * 16);
	return TRUE;
}

EAPI
gboolean g3d_matrix_multiply(G3DMatrix *m1, G3DMatrix *m2, G3DMatrix *rm)
{
	guint32 i, j;
	G3DMatrix matrix[16];

	for(i = 0; i < 4; i ++)
		for(j = 0; j < 4; j ++)
#if 0
			matrix[i * 4 + j] =
				m2[0 * 4 + j] * m1[i * 4 + 0] +
				m2[1 * 4 + j] * m1[i * 4 + 1] +
				m2[2 * 4 + j] * m1[i * 4 + 2] +
				m2[3 * 4 + j] * m1[i * 4 + 3];
#else
			matrix[j * 4 + i] =
				m2[j * 4 + 0] * m1[0 * 4 + i] +
				m2[j * 4 + 1] * m1[1 * 4 + i] +
				m2[j * 4 + 2] * m1[2 * 4 + i] +
				m2[j * 4 + 3] * m1[3 * 4 + i];
#endif

	memcpy(rm, matrix, 16 * sizeof(G3DMatrix));
	return TRUE;
}

EAPI
gboolean g3d_matrix_rotate(G3DFloat angle, G3DVector ax, G3DVector ay,
	G3DVector az, G3DMatrix *rm)
{
	g3d_vector_unify(&ax, &ay, &az);
	g3d_matrix_identity(rm);

#if 0
	rm[0 * 4 + 0] = cos(angle) + (ax * ax) * (1 - cos(angle));
	rm[0 * 4 + 1] = ax * ay * (1 - cos(angle)) - az * sin(angle);
	rm[0 * 4 + 2] = ax * az * (1 - cos(angle)) + ay * sin(angle);

	rm[1 * 4 + 0] = ay * ax * (1 - cos(angle)) + az * sin(angle);
	rm[1 * 4 + 1] = cos(angle) + (ay * ay) * (1 - cos(angle));
	rm[1 * 4 + 2] = ay * az * (1 - cos(angle)) - ax * sin(angle);

	rm[2 * 4 + 0] = az * ax * (1 - cos(angle)) - ay * sin(angle);
	rm[2 * 4 + 1] = az * ay * (1 - cos(angle)) + ax * sin(angle);
	rm[2 * 4 + 2] = cos(angle) + (az * az) * (1 - cos(angle));
#else
	rm[0 * 4 + 0] = cos(angle) + (ax * ax) * (1 - cos(angle));
	rm[1 * 4 + 0] = ax * ay * (1 - cos(angle)) - az * sin(angle);
	rm[2 * 4 + 0] = ax * az * (1 - cos(angle)) + ay * sin(angle);

	rm[0 * 4 + 1] = ay * ax * (1 - cos(angle)) + az * sin(angle);
	rm[1 * 4 + 1] = cos(angle) + (ay * ay) * (1 - cos(angle));
	rm[2 * 4 + 1] = ay * az * (1 - cos(angle)) - ax * sin(angle);

	rm[0 * 4 + 2] = az * ax * (1 - cos(angle)) - ay * sin(angle);
	rm[1 * 4 + 2] = az * ay * (1 - cos(angle)) + ax * sin(angle);
	rm[2 * 4 + 2] = cos(angle) + (az * az) * (1 - cos(angle));
#endif

	return TRUE;
}

EAPI
gboolean g3d_matrix_rotate_xyz(G3DFloat rx, G3DFloat ry, G3DFloat rz,
	G3DMatrix *rm)
{
	G3DMatrix matrix[16];

	g3d_matrix_identity(rm);

	g3d_matrix_rotate(rx, 1.0, 0.0, 0.0, matrix);
	g3d_matrix_multiply(matrix, rm, rm);

	g3d_matrix_rotate(ry, 0.0, 1.0, 0.0, matrix);
	g3d_matrix_multiply(matrix, rm, rm);

	g3d_matrix_rotate(rz, 0.0, 0.0, 1.0, matrix);
	g3d_matrix_multiply(matrix, rm, rm);

	return TRUE;
}

EAPI
gboolean g3d_matrix_translate(G3DVector x, G3DVector y, G3DVector z,
	G3DMatrix *rm)
{
	guint32 i;

#if 0
	for(i = 0; i < 4; i ++)
		rm[i * 4 + 3] =
			rm[i * 4 + 0] * x +
			rm[i * 4 + 1] * y +
			rm[i * 4 + 2] * z +
			rm[i * 4 + 3];
#else
	for(i = 0; i < 4; i ++)
		rm[3 * 4 + i] =
			rm[0 * 4 + i] * x +
			rm[1 * 4 + i] * y +
			rm[2 * 4 + i] * z +
			rm[3 * 4 + i];
#endif
	return TRUE;
}

EAPI
gboolean g3d_matrix_scale(G3DVector x, G3DVector y, G3DVector z, G3DMatrix *rm)
{
	G3DMatrix sm[16];

	g3d_matrix_identity(sm);
	sm[0] = x;
	sm[5] = y;
	sm[10] = z;

	g3d_matrix_multiply(rm, sm, rm);

	return TRUE;
}

EAPI
gboolean g3d_matrix_transpose(G3DMatrix *matrix)
{
	G3DMatrix tmp[16];
	gint32 i, j;

	memcpy(tmp, matrix, 16 * sizeof(G3DMatrix));

	for(i = 0; i < 4; i ++)
		for(j = 0; j < 4; j ++)
			matrix[i * 4 + j] = tmp[j * 4 + i];

	return TRUE;
}

static G3DFloat det2x2(G3DFloat a1, G3DFloat a2, G3DFloat b1, G3DFloat b2)
{
	return a1 * b2 - a2 * b1;
}

static G3DFloat det3x3(G3DFloat a1, G3DFloat a2, G3DFloat a3,
	G3DFloat b1, G3DFloat b2, G3DFloat b3,
	G3DFloat c1, G3DFloat c2, G3DFloat c3)
{
	return
		a1 * det2x2(b2, b3, c2, c3) -
		b1 * det2x2(a2, a3, c2, c3) +
		c1 * det2x2(a2, a3, b2, b3);
}

EAPI
G3DFloat g3d_matrix_determinant(G3DMatrix *matrix)
{
	G3DFloat a1, a2, a3, a4;
	G3DFloat b1, b2, b3, b4;
	G3DFloat c1, c2, c3, c4;
	G3DFloat d1, d2, d3, d4;
#if 0
	a1 = matrix[0 * 4 + 0];
	a2 = matrix[0 * 4 + 1];
	a3 = matrix[0 * 4 + 2];
	a4 = matrix[0 * 4 + 3];

	b1 = matrix[1 * 4 + 0];
	b2 = matrix[1 * 4 + 1];
	b3 = matrix[1 * 4 + 2];
	b4 = matrix[1 * 4 + 3];

	c1 = matrix[2 * 4 + 0];
	c2 = matrix[2 * 4 + 1];
	c3 = matrix[2 * 4 + 2];
	c4 = matrix[2 * 4 + 3];

	d1 = matrix[3 * 4 + 0];
	d2 = matrix[3 * 4 + 1];
	d3 = matrix[3 * 4 + 2];
	d4 = matrix[3 * 4 + 3];
#else
	a1 = matrix[0 * 4 + 0];
	b1 = matrix[0 * 4 + 1];
	c1 = matrix[0 * 4 + 2];
	d1 = matrix[0 * 4 + 3];

	a2 = matrix[1 * 4 + 0];
	b2 = matrix[1 * 4 + 1];
	c2 = matrix[1 * 4 + 2];
	d2 = matrix[1 * 4 + 3];

	a3 = matrix[2 * 4 + 0];
	b3 = matrix[2 * 4 + 1];
	c3 = matrix[2 * 4 + 2];
	d3 = matrix[2 * 4 + 3];

	a4 = matrix[3 * 4 + 0];
	b4 = matrix[3 * 4 + 1];
	c4 = matrix[3 * 4 + 2];
	d4 = matrix[3 * 4 + 3];
#endif
	return
		a1 * det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4) -
		b1 * det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4) +
		c1 * det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4) -
		d1 * det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);
}

gboolean g3d_matrix_dump(G3DMatrix *matrix)
{
#if DEBUG > 0
	gint32 row;

	for(row = 0; row < 4; row ++)
	{
		g_debug("[Matrix] % 2.2f  % 2.2f  % 2.2f  % 2.2f",
			matrix[0 * 4 + row], matrix[1 * 4 + row],
			matrix[2 * 4 + row], matrix[3 * 4 + row]);
	}

	return TRUE;
#else
	return FALSE;
#endif
}
