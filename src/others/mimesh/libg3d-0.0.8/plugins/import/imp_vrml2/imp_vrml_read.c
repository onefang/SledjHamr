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
#include <stdlib.h>

#include "imp_vrml_types.h"
#include "imp_vrml_read.h"

#include "imp_vrml2_nodes.h"
#include "imp_vrml2_types.h"

gboolean vrml_read_check_buffer(VrmlReader *reader)
{
	if(reader->bufsize > 0)
		return TRUE;
	if(g3d_stream_eof(reader->stream))
		return FALSE;
	memset(reader->buffer, '\0', reader->max_bufsize);
	g3d_stream_read_line(reader->stream, reader->buffer,
		reader->max_bufsize - 1);
	reader->line ++;
	reader->bufsize = strlen(reader->buffer);
	reader->bufp = reader->buffer;
	return (reader->bufsize > 0);
}

gboolean vrml_read_skip_ws(VrmlReader *reader)
{
	if(!vrml_read_check_buffer(reader))
		return FALSE;
	while(g_ascii_isspace(*(reader->bufp))) {
		vrml_read_dec(reader);
		if(!vrml_read_check_buffer(reader))
			return FALSE;
	}
	return TRUE;
}

gchar *vrml_read_token(VrmlReader *reader)
{
	gchar *s = NULL;
	gsize len = 0;

	if(!vrml_read_skip_ws(reader))
		return NULL;
	while(vrml_read_check_buffer(reader) &&
		!g_ascii_isspace(*(reader->bufp)) &&
		(*(reader->bufp) != ',')) {
		len ++;
		s = g_realloc(s, (len + 1) * sizeof(gchar));
		s[len - 1] = *(reader->bufp);
		s[len - 0] = '\0';
		vrml_read_dec(reader);
	}
#if DEBUG > 3
	g_debug("token: %s", s);
#endif
	return s;
}

gchar *vrml_read_numeric(VrmlReader *reader)
{
	gchar *s = NULL;
	gsize len = 0;
	gboolean first = TRUE;

	if(!vrml_read_skip_ws(reader))
		return NULL;
	while(vrml_read_check_buffer(reader) &&
		(g_ascii_isdigit(*(reader->bufp)) ||
		 strchr(".-e", *(reader->bufp)))) {

		if(first) {
			if(*(reader->bufp) == 'e')
				return NULL;
			first = FALSE;
		}

		len ++;
		s = g_realloc(s, (len + 1) * sizeof(gchar));
		s[len - 1] = *(reader->bufp);
		s[len - 0] = '\0';
		vrml_read_dec(reader);
	}
#if DEBUG > 3
	g_debug("numeric: %s", s);
#endif
	return s;
}

gchar *vrml_read_id(VrmlReader *reader)
{
	gchar *buf;
	gsize bufsize;

	if(!vrml_read_skip_ws(reader))
		return NULL;
	if(!vrml_read_check_buffer(reader))
		return NULL;
	bufsize = 2;
	buf = g_new(gchar, 2 * sizeof(gchar));
	buf[0] = *(reader->bufp);
	buf[1] = '\0';
	if(!(g_ascii_isalpha(buf[0]) || strchr("_", buf[0]))) {
		g_free(buf);
		return NULL;
	}
	vrml_read_dec(reader);
	if(!vrml_read_check_buffer(reader))
		return buf;
	while(g_ascii_isalnum(*(reader->bufp)) || strchr("_.", *(reader->bufp))) {
		bufsize ++;
		buf = g_realloc(buf, bufsize * sizeof(gchar));
		buf[bufsize - 2] = *(reader->bufp);
		buf[bufsize - 1] = '\0';
		vrml_read_dec(reader);
		if(!vrml_read_check_buffer(reader))
			return buf;
	}
	return buf;
}

gchar *vrml_read_string(VrmlReader *reader)
{
	gchar *buf = NULL;
	gsize bufsize = 0;

	if(!vrml_read_check_buffer(reader))
		return NULL;
	while(*(reader->bufp) != '"') {
		bufsize ++;
		buf = g_realloc(buf, (bufsize + 1) * sizeof(gchar));
		buf[bufsize - 1] = *(reader->bufp);
		buf[bufsize] = '\0';
		vrml_read_dec(reader);
		if(!vrml_read_check_buffer(reader))
			return buf;
	}
	vrml_read_dec(reader);
	return buf;
}

gboolean vrml_read_skip_unknown(VrmlReader *reader)
{
	gchar c, *s;
#if DEBUG > 0
	g_debug("vrml: skipping unknown stuff in line %d", reader->line);
#endif

	while(vrml_read_check_buffer(reader)) {
		c = *(reader->bufp);
		vrml_read_dec(reader);
		switch(c) {
			case '"':
				s = vrml_read_string(reader);
				if(s) {
					g_debug("found string '%s'", s);
					g_free(s);
				}
				break;
			case '{':
			case '[':
				vrml_read_skip_unknown(reader);
				break;
			case '}':
			case ']':
				return TRUE;
				break;
			default:
				break;
		}
	}
	return FALSE;
}

gchar *vrml_read_nodeid(VrmlReader *reader)
{
	gchar *id;
	id = vrml_read_id(reader);
	if(id == NULL)
		return NULL;
	if(!g_ascii_isupper(id[0])) {
		g_free(id);
		return NULL;
	}
	return id;
}

gboolean vrml_read_expect(VrmlReader *reader, const gchar c)
{
	gchar b;
	if(!vrml_read_skip_ws(reader))
		return FALSE;
	if(!vrml_read_check_buffer(reader))
		return FALSE;
	b = *(reader->bufp);
	vrml_read_dec(reader);
	return (b == c);
}

gboolean vrml_read_list(VrmlReader *reader, vrml_read_list_item_callback cb,
	gpointer user_data)
{
	if(!vrml_read_expect(reader, '['))
		return FALSE;

	while(vrml_read_skip_ws(reader)) {
		if(*(reader->bufp) == ']') {
			vrml_read_dec(reader);
			return TRUE;
		}
		if(!cb(reader, user_data))
			return FALSE;

		if(!vrml_read_skip_ws(reader))
			return FALSE;
		if(*(reader->bufp) == ',')
			vrml_read_dec(reader);
	}
	/* should not be reached */
	return vrml_read_expect(reader, ']');
}

/*****************************************************************************/

static VrmlObject *vrml_lookup_object(VrmlReader *reader, const gchar *id)
{
	VrmlObject *object;

	object = g_new0(VrmlObject, 1);
	object->defid = g_strdup(id);
	object->name = g_strdup("object from library");

	return object;
}

static gboolean vrml_read_float_n(VrmlReader *reader, G3DFloat *a, guint32 n)
{
	gint i;
	gchar *s;

	for(i = 0; i < n; i ++) {
		s = vrml_read_numeric(reader);
		if(s) {
			a[i] = atof(s);
			g_free(s);
		} else
			return FALSE;
	}
	return TRUE;
}

static gboolean vrml_read_token_n(VrmlReader *reader, VrmlProperty *property,
	guint32 n)
{
	gchar *token;
	gint32 i;

	for(i = 0; i < n; i ++) {
		token = vrml_read_token(reader);
#if DEBUG > 2
		g_debug("Token: %s", token);
#endif
		g_free(token);
	}

	return TRUE;
}

/* property list callbacks */

gboolean vrml_read_list_object_cb(VrmlReader *reader, gpointer data)
{
	VrmlProperty *property = data;

	property->n_items ++;
	property->v_object = g_realloc(property->v_object,
		property->n_items * sizeof(VrmlObject *));
	property->v_object[property->n_items - 1] =
		vrml_read_object(reader, property->level + 1);
	return (property->v_object[property->n_items - 1] != NULL);
}

gboolean vrml_read_list_string_cb(VrmlReader *reader, gpointer data)
{
	VrmlProperty *property = data;

	if(!vrml_read_expect(reader, '"'))
		return FALSE;

	property->n_items ++;
	property->v_str = g_realloc(property->v_str,
		property->n_items * sizeof(gchar *));
	property->v_str[property->n_items - 1] =
		vrml_read_string(reader);
	return TRUE;
}

gboolean vrml_read_list_int_cb(VrmlReader *reader, gpointer data)
{
	VrmlProperty *property = data;
	gchar *s;

	s = vrml_read_numeric(reader);
	if(s) {
		property->n_items ++;
		property->v_int = g_realloc(property->v_int,
			property->n_items * sizeof(gint32));
		property->v_int[property->n_items - 1] = atoi(s);
		g_free(s);
		return TRUE;
	}
	return FALSE;
}

gboolean vrml_read_list_float_cb(VrmlReader *reader, gpointer data)
{
	VrmlProperty *property = data;

	property->n_items ++;
	property->v_float = g_realloc(property->v_float,
		property->n_items * sizeof(G3DFloat));
	return vrml_read_float_n(reader,
		property->v_float + (property->n_items - 1), 1);
}

gboolean vrml_read_list_float2_cb(VrmlReader *reader, gpointer data)
{
	VrmlProperty *property = data;

	property->n_items += 2;
	property->v_float = g_realloc(property->v_float,
		property->n_items * sizeof(G3DFloat));
	return vrml_read_float_n(reader,
		property->v_float + (property->n_items - 2), 2);
}

gboolean vrml_read_list_float3_cb(VrmlReader *reader, gpointer data)
{
	VrmlProperty *property = data;

	property->n_items += 3;
	property->v_float = g_realloc(property->v_float,
		property->n_items * sizeof(G3DFloat));
	return vrml_read_float_n(reader,
		property->v_float + (property->n_items - 3), 3);
}

gboolean vrml_read_list_float4_cb(VrmlReader *reader, gpointer data)
{
	VrmlProperty *property = data;

	property->n_items += 4;
	property->v_float = g_realloc(property->v_float,
		property->n_items * sizeof(G3DFloat));
	return vrml_read_float_n(reader,
		property->v_float + (property->n_items - 4), 4);
}

gboolean vrml_read_list_floatx_cb(VrmlReader *reader, gpointer data)
{
	VrmlProperty *property = data;
	gchar *s;

	s = vrml_read_numeric(reader);
	while(s) {
		property->n_items ++;
		property->v_float = g_realloc(property->v_float,
			property->n_items * sizeof(G3DFloat));
		property->v_float[property->n_items - 1] = atof(s);
		g_free(s);
		s = vrml_read_numeric(reader);
	}
	return TRUE;
}

/* property reader */

VrmlProperty *vrml_read_property(VrmlReader *reader, guint32 level)
{
	VrmlProperty *property;
	VrmlTypeDef *type = NULL;
	gint i;
	gchar *id, *s;

	id = vrml_read_id(reader);
	if(id == NULL)
		return NULL;

	/* look up object */
	for(i = 0; vrml2_types[i].name != NULL; i ++)
		if(strcmp(id, vrml2_types[i].name) == 0) {
			type = &(vrml2_types[i]);
			break;
		}
	if(type == NULL) {
		g_warning("VRML: unknown type %s in line %d", id, reader->line);
		return NULL;
	}
	property = g_new0(VrmlProperty, 1);
	property->name = id;
	if(type->detect != NULL)
		property->id = type->detect(reader);
	else
		property->id = type->id;
	property->level = level;

#if DEBUG > 0
	g_debug("\\%sproperty '%s', type %d",
		vrml_read_padding + strlen(vrml_read_padding) - 1 - level,
		id, property->id);
#endif

	switch(property->id) {
		case T_OBJECT:
			property->n_items = 1;
			property->v_object = g_new0(VrmlObject *, 1);
			property->v_object[0] = vrml_read_object(reader, level + 1);
			break;
		case T_TOKEN2:
			vrml_read_token_n(reader, property, 2);
			break;
		case T_TOKEN3:
			vrml_read_token_n(reader, property, 3);
			break;
		case T_STRING:
			property->n_items = 1;
			if(!vrml_read_expect(reader, '"'))
				return FALSE;
			property->v_str = g_new0(gchar *, 1);
			property->v_str[0] = vrml_read_string(reader);
			break;
		case T_BOOLEAN:
			s = vrml_read_token(reader);
			if(s) {
				if(strcmp(s, "TRUE") == 0)
					property->v_boolean = TRUE;
				g_free(s);
			}
			break;
		case T_INT:
			s = vrml_read_numeric(reader);
			if(s) {
				property->n_items = 1;
				property->v_int = g_new0(gint32, 1);
				property->v_int[0] = atoi(s);
				g_free(s);
			}
			break;
		case T_FLOAT:
			property->n_items = 1;
			property->v_float = g_new0(G3DFloat, 1);
			vrml_read_float_n(reader, property->v_float, 1);
			break;
		case T_FLOAT2:
			property->n_items = 2;
			property->v_float = g_new0(G3DFloat, 2);
			vrml_read_float_n(reader, property->v_float, 2);
			break;
		case T_FLOAT3:
			property->n_items = 3;
			property->v_float = g_new0(G3DFloat, 3);
			vrml_read_float_n(reader, property->v_float, 3);
			break;
		case T_FLOAT4:
			property->n_items = 4;
			property->v_float = g_new0(G3DFloat, 4);
			vrml_read_float_n(reader, property->v_float, 4);
			break;
		case T_FLOAT_X:
			s = vrml_read_numeric(reader);
			while(s) {
				property->n_items ++;
				property->v_float = g_realloc(property->v_float,
					property->n_items * sizeof(G3DFloat));
				property->v_float[property->n_items - 1] = atof(s);
#if DEBUG > 2
				g_debug("T_FLOAT_X: %i: %.2f", property->n_items,
					property->v_float[property->n_items - 1]);
#endif
				g_free(s);
				s = vrml_read_numeric(reader);
			}
			break;
		case T_LIST_OBJECT:
			vrml_read_list(reader, vrml_read_list_object_cb, property);
			break;
		case T_LIST_STRING:
			vrml_read_list(reader, vrml_read_list_string_cb, property);
			break;
		case T_LIST_INT:
			vrml_read_list(reader, vrml_read_list_int_cb, property);
			break;
		case T_LIST_FLOAT:
			vrml_read_list(reader, vrml_read_list_float_cb, property);
			break;
		case T_LIST_FLOAT2:
			vrml_read_list(reader, vrml_read_list_float2_cb, property);
			break;
		case T_LIST_FLOAT3:
			vrml_read_list(reader, vrml_read_list_float3_cb, property);
			break;
		case T_LIST_FLOAT4:
			vrml_read_list(reader, vrml_read_list_float4_cb, property);
			break;
		case T_LIST_FLOAT_X:
			vrml_read_list(reader, vrml_read_list_floatx_cb, property);
			break;

		default:
			g_free(property->name);
			g_free(property);
			return NULL;
			break;
	}

	return property;
}

static VrmlObject *vrml_read_route_object(VrmlReader *reader, guint32 level)
{
	VrmlObject *object;
	gchar *id;
#if DEBUG > 2
	g_debug("ROUTE object");
#endif
	id = vrml_read_id(reader); /* source */
	if(id == NULL) {
		g_warning("ROUTE: could not get source");
		return NULL;
	}
	g_free(id);
	id = vrml_read_id(reader); /* TO */
	if(id == NULL) {
		g_warning("ROUTE: could not get TO");
		return NULL;
	}
	g_free(id);
	id = vrml_read_id(reader); /* destination */
	if(id == NULL) {
		g_warning("ROUTE: could not get destination");
		return NULL;
	}
	g_free(id);
	object = g_new0(VrmlObject, 1);
	object->name = g_strdup("ROUTE object");
	object->level = level;
	return object;
}

VrmlObject *vrml_read_object(VrmlReader *reader, guint32 level)
{
	VrmlObject *object;
	VrmlProperty *property;
	gchar *id, *defid = NULL;
	gboolean define = FALSE;

	id = vrml_read_id(reader);
	if(id == NULL)
		return NULL;
	if(strcmp(id, "NULL") == 0) {
		return NULL;
	} else if(strcmp(id, "DEF") == 0) {
		define = TRUE;
		defid = vrml_read_id(reader);
		if(defid == NULL)
			return NULL;
		id = vrml_read_id(reader);
		if(id == NULL)
			return NULL;
	} else if(strcmp(id, "USE") == 0) {
		defid = vrml_read_id(reader);
#if DEBUG > 2
		g_debug("looking up '%s'", defid);
#endif
		return vrml_lookup_object(reader, defid);
	} else if(strcmp(id, "ROUTE") == 0) {
		return vrml_read_route_object(reader, level);
	}

	if(!vrml_read_expect(reader, '{')) {
		g_warning("vrml_read_object: expected '{' in line %d", reader->line);
		g_free(id);
		if(defid)
			g_free(defid);
		return FALSE;
	}
#if DEBUG > 0
	g_debug("\\%sobject '%s'",
		vrml_read_padding + strlen(vrml_read_padding) - 1 - level,
		id);
#endif

	object = g_new0(VrmlObject, 1);
	object->name = id;
	object->defid = defid;
	object->define = define;
	object->level = level;

	/* read properties */
	while(vrml_read_skip_ws(reader)) {
#if DEBUG > 2
		g_debug("O: '%c'", *(reader->bufp));
#endif
		if(*(reader->bufp) == '}') {
			vrml_read_dec(reader);
			return object;
		}
		property = vrml_read_property(reader, level + 1);
		if(property != NULL)
			object->properties = g_slist_append(object->properties, property);
		else {
			vrml_read_skip_unknown(reader);
			return object;
		}
	}
	vrml_read_expect(reader, '}');
	return object;
}

gboolean vrml_read_global(VrmlReader *reader)
{
	while(vrml_read_skip_ws(reader)) {
		if(!vrml_read_object(reader, 0))
			return FALSE;
	}
	return TRUE;
}
