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
#include <stdio.h>

#include <g3d/types.h>
#include <g3d/matrix.h>
#include <g3d/object.h>

#include "imp_ldraw_types.h"
#include "imp_ldraw_part.h"
#include "imp_ldraw_library.h"

static inline void ldraw_create_subpart(LDrawLibrary *lib, gchar *name,
	gchar *buffer, GSList **partsp)
{
	G3DStream *substream;
	LDrawPart *part;

#if DEBUG > 2
	g_debug("LDraw: adding sub-file %s to library", name);
#endif

	substream = g3d_stream_from_buffer((guint8 *)buffer, strlen(buffer),
		name, TRUE);
	if(substream == NULL) {
		g_warning("LDraw: failed to create stream for %s", name);
		return;
	}
	part = g_new0(LDrawPart, 1);
	part->name = g_ascii_strup(name, -1);
	part->stream = substream;

	ldraw_library_insert(lib, part->name, part);
	*partsp = g_slist_append(*partsp, part);
}

gboolean ldraw_mpd_load(G3DStream *stream, G3DModel *model,
	LDrawLibrary *lib)
{
	G3DObject *object = NULL;
	GSList *parts = NULL, *item;
	LDrawPart *part;
	gchar buffer[1024], name[256], *streambuf = NULL;
	gsize size;
	goffset off;
	G3DFloat m[16];

	while(!g3d_stream_eof(stream)) {
		memset(buffer, 0, 1024);
		g3d_stream_read_line(stream, buffer, 1023);
		if(strncmp(buffer, "0 FILE ", 7) == 0) {
			if(streambuf) {
				ldraw_create_subpart(lib, name, streambuf, &parts);
				streambuf = NULL;
			}
			memset(name, 0, 256);
			if(sscanf(buffer + 7, "%255s", name) == 1)
				streambuf = g_strdup("");
		} else if(strncmp(buffer, "0 NOFILE", 8) == 0) {
			if(streambuf) {
				ldraw_create_subpart(lib, name, streambuf, &parts);
				streambuf = NULL;
			}
			break;
		} else if(streambuf) {
			/* append line to buffer */
			size = strlen(buffer) + strlen(streambuf) + 1;
			off = strlen(streambuf);
			streambuf = g_realloc(streambuf, size * sizeof(gchar));
			strcpy(streambuf + off, buffer);
		}
	} /* !eof */

	if(streambuf)
		ldraw_create_subpart(lib, name, streambuf, &parts);

	if(parts) {
		part = parts->data;
		part->master = TRUE;
		object = ldraw_part_get_object(part, lib);
		if(object != NULL) {
			g3d_matrix_identity(m);
			g3d_matrix_rotate_xyz(0.0, 0.0, G_PI, m);
			g3d_object_transform(object, m);
			model->objects = g_slist_append(model->objects, object);
		}
	}

#if 1
	/* close open streams */
	for(item = parts; item != NULL; item = item->next) {
		part = item->data;
		if(part->stream) {
			g3d_stream_close(part->stream);
			part->stream = NULL;
		}
	}
#endif
	return (object != NULL);
}

