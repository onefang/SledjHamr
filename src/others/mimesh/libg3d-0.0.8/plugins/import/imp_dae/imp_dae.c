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
#include <locale.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <g3d/stream.h>
#include <g3d/types.h>

#include "imp_dae_xml.h"
#include "imp_dae_library.h"
#include "imp_dae_cb.h"

static int dae_input_read_cb(gpointer ctx, gchar *buffer, gint len)
{
	return g3d_stream_read((G3DStream *)ctx, buffer, len);
}

static gboolean dae_load_scene(G3DContext *context, G3DStream *stream,
	G3DModel *model, DaeLibrary *lib, xmlDocPtr xmldoc);

/*****************************************************************************/

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	xmlDocPtr xmldoc;
	DaeLibrary *lib;
	gboolean retval = FALSE;

	setlocale(LC_NUMERIC, "C");
	xmlInitParser();

	xmldoc = xmlReadIO(dae_input_read_cb, NULL, stream, stream->uri, NULL, 0);
	if(xmldoc) {
		lib = dae_library_load(xmldoc);
		retval = dae_load_scene(context, stream, model, lib, xmldoc);

		dae_library_cleanup(lib);
		xmlFreeDoc(xmldoc);
	}

	xmlCleanupParser();
	return retval;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("COLLADA models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("dae", ":", 0);
}

/*****************************************************************************/
/* COLLADA specific stuff                                                    */

static gboolean dae_load_scene(G3DContext *context, G3DStream *stream,
	G3DModel *model, DaeLibrary *lib, xmlDocPtr xmldoc)
{
	DaeGlobalData *global;
	DaeLocalData *local;
	xmlNodePtr scenenode, node = NULL, instance = NULL;
	gchar *name;

	scenenode = dae_xml_get_child_by_tagname(
		xmlDocGetRootElement(xmldoc), "scene");

	if(scenenode == NULL) {
		g_warning("DAE: could not get scene node");
		return FALSE;
	}

	global = g_new0(DaeGlobalData, 1);
	global->context = context;
	global->stream = stream;
	global->model = model;
	global->xmldoc = xmldoc;
	global->lib = lib;

	while(dae_xml_next_child(lib, scenenode, &node, &instance, &name)) {
#if DEBUG > 2
		g_debug("DAE: got node %s", name);
#endif
		if(strcmp(name, "visual_scene") == 0) {
			local = g_new0(DaeLocalData, 1);
			local->node = node;
			local->instance = instance;
			local->parent = scenenode;
			dae_cb_visual_scene(global, local);
			g_free(local);
		}
		g_free(name);
	}

	g_free(global);

	return TRUE;
}
