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

#ifndef IMP_LEOCAD_LIBRARY_H
#define IMP_LEOCAD_LIBRARY_H

#include <stdio.h>

#include <g3d/types.h>
#include <g3d/stream.h>

typedef struct
{
	gchar *name;
	gchar *description;
	gchar *moved_to;

	guint16 bounding_box[6];
	guint8 flags;
	guint32 default_group;
	guint32 offset_bin;
	guint32 info_size;

	G3DObject *object;
}
LeoCadPiece;

typedef struct
{
	G3DStream *pieces_bin;
	GHashTable *pieces;
	GSList *materials;
}
LeoCadLibrary;

LeoCadLibrary *leocad_library_load(const gchar *libdir);
void leocad_library_free(LeoCadLibrary *library);
guint8 leocad_library_convert_color(guint8 n);
G3DObject *leocad_library_get_piece(LeoCadLibrary *library, const gchar *name);
G3DMaterial *leocad_library_get_nth_material(LeoCadLibrary *library, guint8 n);

#endif /* IMP_LEOCAD_LIBRARY_H */

