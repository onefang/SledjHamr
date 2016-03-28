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

#include <g3d/types.h>
#include <g3d/face.h>
#include <g3d/vector.h>

#include "imp_acf_airfoil.h"

G3DObject *acf_wing(G3DMaterial *material, const gchar *name,
	G3DFloat *msweep, G3DFloat *mdihed, G3DFloat *mtrans,
	G3DFloat *vrt, G3DFloat *vtp,
	AcfAirfoil *aflrt, AcfAirfoil *afltp,
	G3DFloat wrt, G3DFloat wtp, G3DFloat lf)
{
	G3DObject *object;
	G3DFace *face;
	guint32 nverts;
	gint32 i;
	G3DFloat vec[3];

	nverts = aflrt->vertex_count;

	object = g_new0(G3DObject, 1);
	object->name = g_strdup(name);
	object->vertex_count = nverts * 2;
	object->vertex_data = g_new0(G3DFloat, object->vertex_count * 3);

	/* vertices */
	for(i = 0; i < nverts; i ++) {
		/* wing root */
		vec[2] = aflrt->vertex_data[i * 2 + 0] * wrt;
		vec[1] = aflrt->vertex_data[i * 2 + 1] * wrt;
		vec[0] = 0.0;
		g3d_vector_transform(vec, vec + 1, vec + 2, mdihed);
		g3d_vector_transform(vec, vec + 1, vec + 2, mtrans);
		memcpy(object->vertex_data + i * 3, vec, sizeof(vec));

		/* wing tip */
		vec[2] = afltp->vertex_data[i * 2 + 0] * wtp;
		vec[1] = afltp->vertex_data[i * 2 + 1] * wtp;
		vec[0] = 0.0;
		g3d_vector_transform(vec, vec + 1, vec + 2, mdihed);
		g3d_vector_transform(vec, vec + 1, vec + 2, mtrans);
		vec[0] += lf * vtp[0];
		vec[1] += lf * vtp[1];
		vec[2] += lf * vtp[2];
		memcpy(object->vertex_data + (i + nverts) * 3, vec, sizeof(vec));
	}
	/* faces */
	for(i = 0; i < (nverts - 1); i ++) {
		face = g3d_face_new_tri(material, i + 1, i, i + nverts);
		object->faces = g_slist_prepend(object->faces, face);
		face = g3d_face_new_tri(material, i + 1, i + nverts + 1, i + nverts);
		object->faces = g_slist_prepend(object->faces, face);
	}

	return object;
}
