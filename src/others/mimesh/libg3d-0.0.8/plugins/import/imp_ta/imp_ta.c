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

#include <glib.h>
#include <g3d/types.h>

#include "imp_ta_3do.h"
#include "imp_ta_3do_palette.h"

EAPI
gpointer plugin_init(G3DContext *context)
{
	G3DMaterial *materials;
	guint32 i;

#if DEBUG > 0
	g_debug("TA: initializing 3DO palette");
#endif

	materials = g_new0(G3DMaterial, 256);
	for(i = 0; i < 256; i ++)
	{
		materials[i].r = ((G3DFloat)ta_3do_palette[i * 3 + 0]) / 255.0;
		materials[i].g = ((G3DFloat)ta_3do_palette[i * 3 + 1]) / 255.0;
		materials[i].b = ((G3DFloat)ta_3do_palette[i * 3 + 2]) / 255.0;
		materials[i].a = 1.0;
	}

	return materials;
}

EAPI
void plugin_cleanup(gpointer user_data)
{
	/* (G3DMaterials *) */
	g_free(user_data);
}

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	return ta_3do_load_object(context, stream, model,
		(G3DMaterial *)user_data);
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("Total Annihilation unit models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("3do", ":", 0);
}

