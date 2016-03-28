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
#include <libxml/tree.h>

#include "imp_dae_xml.h"
#include "imp_dae_library.h"

struct _DaeLibrary {
	GHashTable *ids;
	GSList *libs;
};

typedef struct {
	GHashTable *ids;
	GSList *nodes;
} DaeLibraryNodes;

static gchar *dae_library_names[][2] = {
	{ "library_animations",     "animation"     },
	{ "library_cameras",        "camera"        },
	{ "library_controllers",    "controller"    },
	{ "library_effects",        "effect"        },
	{ "library_geometries",     "geometry"      },
	{ "library_images",         "image"         },
	{ "library_lights",         "light"         },
	{ "library_materials",      "material"      },
	{ "library_nodes",          "node"          },
	{ "library_physics_scenes", "physics_scene" },
	{ "library_visual_scenes",  "visual_scene"  },
	{ NULL, NULL }
};

static gboolean dae_add_library(DaeLibrary *lib, xmlNodePtr libnode,
	guint32 entryid)
{
	xmlNodePtr node;
	DaeLibraryNodes *nodelib;
	gchar *id;

	nodelib = g_new0(DaeLibraryNodes, 1);
	nodelib->ids = g_hash_table_new_full(
		g_str_hash, g_str_equal, g_free, NULL);
	g_hash_table_insert(lib->ids, g_strdup(dae_library_names[entryid][1]),
		nodelib);
	lib->libs = g_slist_append(lib->libs, nodelib);

	node = libnode->children;
	while(node != NULL) {
		if((node->type == XML_ELEMENT_NODE) &&
			(xmlStrcmp(node->name,
				(const xmlChar *)dae_library_names[entryid][1]) == 0)) {
			/* found library entry */
			id = dae_xml_get_attr(node, "id");
			if(id != NULL) {
#if DEBUG > 2
				g_debug("\t%s id=\"%s\"", dae_library_names[entryid][1], id);
#endif
				g_hash_table_insert(nodelib->ids, id, node);
				nodelib->nodes = g_slist_append(nodelib->nodes, node);
			}
		}
		node = node->next;
	}
	return TRUE;
}

gboolean dae_library_add(DaeLibrary *lib, const gchar *libname,
	const gchar *id, xmlNodePtr node)
{
	DaeLibraryNodes *nodelib;

	g_return_val_if_fail(lib != NULL, FALSE);

	nodelib = g_hash_table_lookup(lib->ids, libname);
	if(nodelib == NULL) {
		nodelib = g_new0(DaeLibraryNodes, 1);
		nodelib->ids = g_hash_table_new_full(
			g_str_hash, g_str_equal, g_free, NULL);
		g_hash_table_insert(lib->ids, g_strdup(libname), nodelib);
		lib->libs = g_slist_append(lib->libs, nodelib);
	}
	g_hash_table_insert(nodelib->ids, g_strdup(id), node);
	nodelib->nodes = g_slist_append(nodelib->nodes, node);
	return TRUE;
}

DaeLibrary *dae_library_load(xmlDocPtr xmldoc)
{
	DaeLibrary *lib;
	gint i = 0;
	xmlNodePtr rootnode, node;

	lib = g_new0(DaeLibrary, 1);
	lib->ids = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	rootnode = xmlDocGetRootElement(xmldoc);
	g_return_val_if_fail(rootnode != NULL, NULL);

	while(dae_library_names[i][0] != NULL) {
#if DEBUG > 2
		g_debug("loading library %s", dae_library_names[i][0]);
#endif
		node = rootnode->children;
		while(node != NULL) {
			if((node->type == XML_ELEMENT_NODE) &&
				(xmlStrcmp(node->name,
					(const xmlChar *)dae_library_names[i][0]) == 0)) {
				dae_add_library(lib, node, i);
				break;
			}
			node = node->next;
		}
		i ++;
	}
	return lib;
}

xmlNodePtr dae_library_lookup(DaeLibrary *library, const gchar *tagname,
	const gchar *id)
{
	DaeLibraryNodes *nodelib;

	nodelib = g_hash_table_lookup(library->ids, tagname);
	if(nodelib == NULL) {
		g_warning("DAE: failed to lookup library for '%s'", tagname);
		return NULL;
	}
	return (xmlNodePtr)g_hash_table_lookup(nodelib->ids, id);
}

void dae_library_cleanup(DaeLibrary *library)
{
	DaeLibraryNodes *nodelib;
	GSList *item;

	for(item = library->libs; item != NULL; item = item->next) {
		nodelib = (DaeLibraryNodes *)item->data;
		g_hash_table_destroy(nodelib->ids);
		g_slist_free(nodelib->nodes);
	}

	g_hash_table_destroy(library->ids);
	g_slist_free(library->libs);
	g_free(library);
}

