/* $Id:$ */

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

/*
 * some parts based on "trackball.c"
 *
 * (c) Copyright 1993, 1994, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

#include <math.h>

#include <g3d/types.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>

EAPI
gboolean g3d_quat_rotate(G3DQuat *q, G3DVector *axis, G3DFloat angle)
{
	gint32 i;

	g3d_vector_unify(axis + 0, axis + 1, axis + 2);
	for(i = 0; i < 3; i ++)
		q[i] = axis[i] * sin(angle / 2.0);
	q[3] = cos(angle / 2.0);

	return TRUE;
}

EAPI
gboolean g3d_quat_normalize(G3DQuat *q)
{
	gint32 i;
	G3DDouble div = 0;

	for(i = 0; i < 4; i ++)
		div += (q[i] * q[i]);
	for(i = 0; i < 4; i ++)
		q[i] /= div;

	return TRUE;
}

EAPI
gboolean g3d_quat_add(G3DQuat *qr, G3DQuat *q1, G3DQuat *q2)
{
	gint32 i;
	G3DQuat tr[4], t1[4], t2[4], t3[4];

	for(i = 0; i < 3; i ++)
		t1[i] = q1[i] * q2[3];
	for(i = 0; i < 3; i ++)
		t2[i] = q2[i] * q1[3];
	g3d_vector_normal(q2[0], q2[1], q2[2], q1[0], q1[1], q1[2],
		t3, t3 + 1, t3 + 2);
	for(i = 0; i < 3; i ++)
		tr[i] = t1[i] + t2[i] + t3[i];
	tr[3] = q1[3] * q2[3] -
		(q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2]);
	for(i = 0; i < 4; i ++)
		qr[i] = tr[i];

	return TRUE;
}

static inline G3DVector trackball_projection(G3DVector x, G3DVector y,
	G3DFloat r)
{
	G3DDouble d, t;

	d = sqrt(x * x + y * y);
	if(d < (r * 0.70710678118654752440))
		return sqrt(r * r - d * d);
	t = r / 1.41421356237309504880;
	return t * t / d;
}

EAPI
gboolean g3d_quat_trackball(G3DQuat *q, G3DFloat x1, G3DFloat y1,
	G3DFloat x2, G3DFloat y2, G3DFloat r)
{
	G3DVector axis[3], z1, z2, dx, dy, dz;
	G3DDouble angle, t;

	if((x1 == x2) && (y1 == y2)) {
		q[0] = q[1] = q[2] = 0.0;
		q[3] = 1.0;
		return TRUE;
	}

	z1 = trackball_projection(x1, y1, r);
	z2 = trackball_projection(x2, y2, r);

	g3d_vector_normal(x2, y2, z2, x1, y1, z1, axis, axis + 1, axis + 2);

	dx = x1 - x2;
	dy = y1 - y2;
	dz = z1 - z2;
	t = sqrt(dx * dx + dy * dy + dz * dz) / (2.0 * r);
	t = CLAMP(t, -1.0, 1.0);
	angle = 2.0 * asin(t);

	return g3d_quat_rotate(q, axis, angle);
}

EAPI
gboolean g3d_quat_to_matrix(G3DQuat *q, G3DMatrix *matrix)
{
	g3d_matrix_identity(matrix);

	matrix[0 * 4 + 0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
	matrix[0 * 4 + 1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
	matrix[0 * 4 + 2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);

	matrix[1 * 4 + 0] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
	matrix[1 * 4 + 1] = 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
	matrix[1 * 4 + 2] = 2.0 * (q[1] * q[2] - q[0] * q[3]);

	matrix[2 * 4 + 0] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
	matrix[2 * 4 + 1] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
	matrix[2 * 4 + 2] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);

	return TRUE;
}

EAPI
gboolean g3d_quat_to_rotation_xyz(G3DQuat *q, G3DFloat *rx, G3DFloat *ry,
	G3DFloat *rz)
{
	G3DFloat t;

	/* rx */
	t = 1 - 2 * (q[1] * q[1] + q[2] * q[2]);
	if(t != 0)
		*rx = atan(2 * (q[0] * q[1] + q[2] * q[3]) / t);
	else
		*rx = 0.0;

	/* ry */
	*ry = asin(2 * (q[0] * q[2] - q[1] * q[3]));

	/* rz */
	t = 1 - 2 * (q[2] * q[2] + q[3] * q[3]);
	if(t != 0)
		*rz = atan(2 * (q[0] * q[3] + q[1] * q[2]) / t);
	else
		*rz = 0.0;

	return TRUE;
}
