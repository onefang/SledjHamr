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
#include <math.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/object.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>
#include <g3d/primitive.h>

typedef struct {
	guint32 length;
	guint32 *ids;
} OSMNodeTransList;

typedef struct {
	const gchar *name;
	gdouble r, g, b, a;
} OSMMaterial;

static OSMMaterial osm_materials[] = {
	{ "default",                   0.7, 0.7, 0.7, 0.7 },
	{ "highway:primary",           1.0, 0.8, 0.1, 1.0 },
	{ "highway:secondary",         1.0, 0.2, 0.2, 1.0 },
	{ "highway:footway",           0.2, 1.0, 0.2, 1.0 },
	{ "waterway:canal",            0.0, 0.1, 1.0, 0.7 },
	{ "waterway:river",            0.0, 0.4, 1.0, 0.7 },
	{ "waterway:riverbank",        0.0, 0.4, 1.0, 0.7 },
	{ NULL,                        0.0, 0.0, 0.0, 0.0 }
};

static int osm_input_read_cb(gpointer ctx, gchar *buffer, gint len);
static void osm_add_node(G3DObject *object, OSMNodeTransList *translist,
	xmlNodePtr node);
static void osm_add_way(G3DObject *object, OSMNodeTransList *translist,
	xmlNodePtr node, GHashTable *materials);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model)
{
	xmlDocPtr xmldoc;
	xmlNodePtr rootnode, node;
	OSMNodeTransList *translist;
	G3DObject *object;
	GHashTable *materials;
	G3DMaterial *material;
	OSMMaterial *mentry;

	setlocale(LC_NUMERIC, "C");

	xmlInitParser();

	xmldoc = xmlReadIO(osm_input_read_cb, NULL, stream, stream->uri, NULL, 0);
	if(xmldoc == NULL) {
		g_warning("OSM: failed to parse XML file '%s'", stream->uri);
		xmlCleanupParser();
		return FALSE;
	}

	/* create material table */
	materials = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	for(mentry = osm_materials; mentry->name != NULL; mentry ++) {
		material = g3d_material_new();
		material->name = g_strdup(mentry->name);
		material->r = mentry->r;
		material->g = mentry->g;
		material->b = mentry->b;
		material->a = mentry->a;
		model->materials = g_slist_append(model->materials, material);
		g_hash_table_insert(materials, g_strdup(material->name), material);
	}

	translist = g_new0(OSMNodeTransList, 1);
	object = g_new0(G3DObject, 1);
	object->name = g_strdup("OpenStreetMap object");
	model->objects = g_slist_append(model->objects, object);

	rootnode = xmlDocGetRootElement(xmldoc);
	for(node = rootnode->children; node != NULL; node = node->next) {
		/* skip non-element nodes */
		if(node->type != XML_ELEMENT_NODE)
			continue;

		if(xmlStrcmp((xmlChar *)"node", node->name) == 0) {
			/* "node" nodes */
			osm_add_node(object, translist, node);
		} else if(xmlStrcmp((xmlChar *)"way", node->name) == 0) {
			/* "way" nodes */
			osm_add_way(object, translist, node, materials);
		} else if(xmlStrcmp((xmlChar *)"relation", node->name) == 0) {
			/* "relation" nodes */
		}
	}

	/* clean up */
	if(object->vertex_data) {
		/* reference points not needed anymore */
		g_free(object->vertex_data);
		object->vertex_data = NULL;
		object->vertex_count = 0;
	}
	g_hash_table_destroy(materials);
	if(translist->ids)
		g_free(translist->ids);
	g_free(translist);
	xmlFreeDoc(xmldoc);
	xmlCleanupParser();

	return TRUE;
}

EAPI
char *plugin_description(void)
{
	return g_strdup("OpenStreetMap maps.");
}

EAPI
char **plugin_extensions(void)
{
	return g_strsplit("osm", ":", 0);
}

/*****************************************************************************/
/* helper functions
 *****************************************************************************/

static int osm_input_read_cb(gpointer ctx, gchar *buffer, gint len)
{
	return g3d_stream_read((G3DStream *)ctx, buffer, len);
}

#if 0
#define G(i) (gdouble)(i)

static gdouble misc_angle(gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
	gdouble at;

	if(x1 == x2) {
		if(y1 > y2)
			return G_PI * 1.5;
		else
			return G_PI / 2.0;
	}
	at = atan((G(y2) - G(y1)) / (G(x2) - G(x1)));
	if(x2 < x1)
		return G_PI + at;
	else
		return at;
}

static gdouble misc_delta(gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
	gdouble a, b;

	a = ABS(G(x2) - G(x1));
	b = ABS(G(y2) - G(y1));
	return sqrt(a * a + b * b);
}
#endif

/*****************************************************************************/
/* OSM specific functions
 *****************************************************************************/

static gint32 osm_translist_lookup(OSMNodeTransList *translist, guint32 id)
{
	gint32 i;

	for(i = 0; i < translist->length; i ++)
		if(translist->ids[i] == id)
			return i;
	return -1;
}

static void osm_add_node(G3DObject *object, OSMNodeTransList *translist,
	xmlNodePtr node)
{
	gdouble lat, lon;

	lat = strtod((char *)xmlGetProp(node, (xmlChar *)"lat"), NULL);
	lon = strtod((char *)xmlGetProp(node, (xmlChar *)"lon"), NULL);

	translist->length ++;
	translist->ids = g_realloc(translist->ids,
		translist->length * sizeof(guint32));
	translist->ids[translist->length - 1] = strtoul(
		(char *)xmlGetProp(node, (xmlChar *)"id"), NULL, 10);

	object->vertex_count ++;
	object->vertex_data = g_realloc(object->vertex_data,
		object->vertex_count * sizeof(gdouble) * 3);
	object->vertex_data[(object->vertex_count - 1) * 3 + 0] =
		(lat * G_PI / 180) * cos(lon * G_PI / 180) * 180 / G_PI;
	object->vertex_data[(object->vertex_count - 1) * 3 + 1] = 0.0;
	object->vertex_data[(object->vertex_count - 1) * 3 + 2] =
		(lat * G_PI / 180) * sin(lon * G_PI / 180) * 180 / G_PI;
}

static void osm_add_street(G3DObject *object, OSMNodeTransList *translist,
	guint32 refcount, guint32 *refdata,
	GHashTable *tags, GHashTable *materials)
{
	gint32 i, n;
	gdouble *vdata;
	G3DFloat matrix[16];
	G3DObject *ostreet = NULL;
	gchar *name, *mname;
	G3DMaterial *material = NULL;

	/* lookup material */
	name = g_hash_table_lookup(tags, "highway");
	if(name != NULL) {
		mname = g_strdup_printf("highway:%s", name);
		material = g_hash_table_lookup(materials, mname);
		g_free(mname);
	} else {
		name = g_hash_table_lookup(tags, "waterway");
		if(name != NULL) {
			mname = g_strdup_printf("waterway:%s", name);
			material = g_hash_table_lookup(materials, mname);
			g_free(mname);
		}
	}
	if(material == NULL) {
		material = g_hash_table_lookup(materials, "default");
		g_return_if_fail(material != NULL);
	}

	/* create strip */
	vdata = g_new0(gdouble, refcount * 2);
	for(i = 0; i < refcount; i ++) {
		n = osm_translist_lookup(translist, refdata[i]);
		if(n == -1) {
			g_warning("OSM: looking up reference %d failed", refdata[i]);
			continue;
		}
		vdata[i * 2 + 0] = object->vertex_data[n * 3 + 0];
		vdata[i * 2 + 1] = object->vertex_data[n * 3 + 2];
	}
	ostreet = g3d_primitive_box_strip_2d(refcount, vdata, 0.00003, 0.0003,
		material);
	g_free(vdata);

	if(ostreet == NULL)
		return;

	/* bridge? */
	name = g_hash_table_lookup(tags, "bridge");
	if(name && (strcmp(name, "true") == 0)) {
		g3d_matrix_identity(matrix);
		g3d_matrix_translate(0.0, 0.00005, 0.0, matrix);
		g3d_object_transform(ostreet, matrix);
	}

	/* name? */
	name = g_hash_table_lookup(tags, "name");
	if(name == NULL)
		ostreet->name = g_strdup("unnamed street");
	else
		ostreet->name = g_strdup(name);
	object->objects = g_slist_append(object->objects, ostreet);
}

static void osm_add_way(G3DObject *object, OSMNodeTransList *translist,
	xmlNodePtr node, GHashTable *materials)
{
	guint32 refcount = 0;
	guint32 *refdata = NULL;
	GHashTable *tags;
	xmlNodePtr subnode;

	tags = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	/* parse subnodes */
	for(subnode = node->children; subnode != NULL; subnode = subnode->next) {
		if(subnode->type != XML_ELEMENT_NODE)
			continue;

		if(xmlStrcmp((xmlChar *)"nd", subnode->name) == 0) {
			refcount ++;
			refdata = g_realloc(refdata, refcount * sizeof(guint32));
			refdata[refcount - 1] = strtoul(
				(char *)xmlGetProp(subnode, (xmlChar *)"ref"), NULL, 10);
		} else if(xmlStrcmp((xmlChar *)"tag", subnode->name) == 0) {
			g_hash_table_insert(tags,
				g_strdup((char *)xmlGetProp(subnode, (xmlChar *)"k")),
				g_strdup((char *)xmlGetProp(subnode, (xmlChar *)"v")));
		} else {
			/* unknown "way" subnode */
			g_debug("OSM: 'way': unknown subnode '%s'",
				(gchar *)subnode->name);
		}
	}

	/* do something with the collected data */
	if((g_hash_table_lookup(tags, "highway") != NULL) ||
		(g_hash_table_lookup(tags, "waterway") != NULL))
		osm_add_street(object, translist, refcount, refdata, tags, materials);

	/* clean up */
	g_hash_table_destroy(tags);
	if(refdata)
		g_free(refdata);
}
