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
#ifndef _IMP_LWO_H
#define _IMP_LWO_H

#include <stdio.h>
#include <g3d/types.h>

#define LWO_FLAG_LWO2          (1 << 0)

typedef struct {
	gint32 ntags;
	gchar **tags;

	gint32 nclips;
	guint32 *clips;
	gchar **clipfiles;

	G3DFloat *tex_vertices;

	G3DObject *object;
} LwoObject;

G3DObject *lwo_create_object(G3DStream *stream, G3DModel *model,
	guint32 flags);
gint lwo_read_string(G3DStream *stream, gchar *s);
guint32 lwo_read_vx(G3DStream *stream, guint *index);

#endif /* _IMP_LWO_H */
