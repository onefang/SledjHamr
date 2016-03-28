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

#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <g3d/iff.h>
#include <g3d/stream.h>

#include "imp_maya_chunks.h"

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	G3DIffGlobal *global;
	G3DIffLocal *local;
	guint32 id;
	gsize len;

	if(!g3d_iff_check(stream, &id, &len) ||
		(id != G3D_IFF_MKID('M','a','y','a'))) {
		g_warning("file is not an Maya file %s", stream->uri);
		return FALSE;
	}

	local = g_new0(G3DIffLocal, 1);
	global = g_new0(G3DIffGlobal, 1);

	global->context = context;
	global->model = model;
	global->stream = stream;

	local->parent_id = id;
	local->nb = len;

	g3d_iff_read_ctnr(global, local, maya_chunks, G3D_IFF_PAD4);

	g_free(local);
	g_free(global);

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("Maya models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("mb", ":", 0);
}

