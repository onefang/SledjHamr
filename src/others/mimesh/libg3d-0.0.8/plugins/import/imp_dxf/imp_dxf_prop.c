/* $Id:$ */

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
#include "imp_dxf_types.h"

struct _DxfEntityProps {
	GHashTable *hash;
};

typedef struct {
	gint32 key;
	DxfChunkType type;
	union {
		gint32 ival;
		gdouble dval;
		gchar *strval;
	} u;
} DxfEProp;

DxfEntityProps *dxf_prop_create(void)
{
	DxfEntityProps *eprop;

	eprop = g_new0(DxfEntityProps, 1);
	eprop->hash = g_hash_table_new(g_int_hash, g_int_equal);

	return eprop;
}

static gboolean remove_p(gpointer key, gpointer value, gpointer user_data)
{
	DxfEProp *p = value;
	if(p->type == DXF_T_STRING)
		g_free(p->u.strval);
	g_free(p);
	return TRUE;
}

void dxf_prop_cleanup(DxfEntityProps *eprop)
{
	g_hash_table_foreach_remove(eprop->hash, remove_p, NULL);
	g_hash_table_destroy(eprop->hash);
	g_free(eprop);
}

static DxfEProp *prop_get(DxfEntityProps *eprop, gint32 key)
{
	DxfEProp *p;

	p = g_hash_table_lookup(eprop->hash, &key);
	if(p)
		return p;
	p = g_new0(DxfEProp, 1);
	p->key = key;
	g_hash_table_insert(eprop->hash, &(p->key), p);
	return p;
}

gboolean dxf_prop_set_int(DxfEntityProps *eprop, gint32 key, gint32 i)
{
	DxfEProp *p = prop_get(eprop, key);
	p->type = DXF_T_INT32;
	p->u.ival = i;
	return TRUE;
}

gboolean dxf_prop_set_dbl(DxfEntityProps *eprop, gint32 key, gdouble dbl)
{
	DxfEProp *p = prop_get(eprop, key);
	p->type = DXF_T_FLOAT64;
	p->u.dval = dbl;
	return TRUE;
}

gboolean dxf_prop_set_str(DxfEntityProps *eprop, gint32 key,
	const gchar *str)
{
	DxfEProp *p = prop_get(eprop, key);
	p->type = DXF_T_STRING;
	if(p->u.strval)
		g_free(p->u.strval);
	p->u.strval = g_strdup(str);
	return TRUE;
}

gint32 dxf_prop_get_int(DxfEntityProps *eprop, gint32 key, gint32 dfl)
{
	DxfEProp *p = g_hash_table_lookup(eprop->hash, &key);
	if(p)
		return p->u.ival;
	return dfl;
}

gdouble dxf_prop_get_dbl(DxfEntityProps *eprop, gint32 key, gdouble dfl)
{
	DxfEProp *p = g_hash_table_lookup(eprop->hash, &key);
	if(p)
		return p->u.dval;
	return dfl;
}

const gchar *dxf_prop_get_str(DxfEntityProps *eprop, gint32 key,
	const gchar *dfl)
{
	DxfEProp *p = g_hash_table_lookup(eprop->hash, &key);
	if(p)
		return p->u.strval;
	return dfl;
}

