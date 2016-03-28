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
#include <g3d/stream.h>
#include <g3d/matrix.h>
#include <g3d/object.h>

#include "imp_ldraw_types.h"
#include "imp_ldraw_part.h"
#include "imp_ldraw_library.h"
#include "imp_ldraw_mpd.h"

static gboolean ldraw_load_simple(G3DStream *stream, G3DModel *model,
	LDrawLibrary *lib);


EAPI
gpointer plugin_init(G3DContext *context)
{
	return ldraw_library_init();
}

EAPI
void plugin_cleanup(gpointer user_data)
{
	LDrawLibrary *lib = user_data;

	ldraw_library_cleanup(lib);
}

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	LDrawLibrary *lib = user_data;

	g_return_val_if_fail(lib != NULL, FALSE);
	if(lib->libdir == NULL) {
		g_warning("LDraw: library not found, please set LDRAWDIR");
		return FALSE;
	}

	lib->context = context;

	if(g_ascii_strcasecmp(stream->uri + strlen(stream->uri) - 4, ".mpd") == 0)
		return ldraw_mpd_load(stream, model, lib);
	else
		return ldraw_load_simple(stream, model, lib);

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("LDraw models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("ldr:mpd:dat", ":", 0);
}

/*****************************************************************************/

static gboolean ldraw_load_simple(G3DStream *stream, G3DModel *model,
	LDrawLibrary *lib)
{
	LDrawPart *part;
	G3DObject *object;
	G3DFloat m[16];

	part = g_new0(LDrawPart, 1);
	part->name = g_path_get_basename(stream->uri);
	part->stream = stream;
	part->master = TRUE;

	object = ldraw_part_get_object(part, lib);
	if(object == NULL)
		return FALSE;

	g3d_matrix_identity(m);
	g3d_matrix_rotate_xyz(0.0, 0.0, G_PI, m);
	g3d_object_transform(object, m);
	model->objects = g_slist_append(model->objects, object);
	return TRUE;
}

