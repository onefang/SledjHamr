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

#include "imp_acf_def.h"

#define ACF_DEBUG_TYPES 2

AcfFile *acf_def_read(G3DStream *stream, const AcfDef *def,
	gboolean bigendian)
{
	AcfFile *acf;
	AcfValue *value;
	gint32 i, j;

	acf = g_new0(AcfFile, 1);
	acf->db = g_hash_table_new(g_str_hash, g_str_equal);

	for(i = 0; def[i].type != XEOF; i ++) {
		value = g_new0(AcfValue, 1);
		value->name = g_strdup(def[i].description);
		value->type = def[i].type;
		value->num = def[i].num;
		switch(value->type) {
			case XCHR:
				value->xchr = g_new0(gchar, value->num + 1);
				g3d_stream_read(stream, value->xchr, value->num);
#if DEBUG > ACF_DEBUG_TYPES
				g_debug("ACF: XCHR: %s = %s", value->name, value->xchr);
#endif
				break;
			case XINT:
				value->xint = g_new0(gint32, value->num);
				for(j = 0; j < value->num; j ++)
					value->xint[j] = bigendian ?
						g3d_stream_read_int32_be(stream) :
						g3d_stream_read_int32_le(stream);
#if DEBUG > ACF_DEBUG_TYPES
				g_debug("ACF: XINT: %s(1/%d) = %i", value->name, value->num,
					value->xint[0]);
#endif
				break;
			case XFLT:
				value->xflt = g_new0(G3DFloat, value->num);
				for(j = 0; j < value->num; j ++)
					value->xflt[j] = bigendian ?
						g3d_stream_read_float_be(stream) :
						g3d_stream_read_float_le(stream);
#if DEBUG > ACF_DEBUG_TYPES
				g_debug("ACF: XFLT: %s(1/%d) = %f", value->name, value->num,
					value->xflt[0]);
#endif
				break;
			case XEOF:
				/* should never happen, just make compiler happy */
				break;
		}
		g_hash_table_insert(acf->db, def[i].description, value);
	}

	return acf;
}

static gboolean acf_def_remove_value_cb(gpointer key, gpointer hashvalue,
	gpointer data)
{
	AcfValue *value = hashvalue;

	if(value->xchr)
		g_free(value->xchr);
	if(value->xint)
		g_free(value->xint);
	if(value->xflt)
		g_free(value->xflt);
	g_free(value->name);
	g_free(value);
	return TRUE;
}

void acf_def_free(AcfFile *acf)
{
	g_hash_table_foreach_remove(acf->db, acf_def_remove_value_cb, NULL);
	g_free(acf);
}

AcfValue *acf_def_lookup(AcfFile *acf, const gchar *name)
{
	return g_hash_table_lookup(acf->db, name);
}

void acf_def_dump(AcfValue *value)
{
	gint32 i;

	if(value->type == XCHR) {
		g_debug("ACF: %s: %s", value->name, value->xchr);
		return;
	}

	g_debug("ACF: %s: dumping %d %s items", value->name, value->num,
		(value->type == XINT) ? "XINT" : "XFLT");
	for(i = 0; i < value->num; i ++) {
		if(value->type == XINT)
			g_debug("\tXINT: %s[%i] = %i", value->name, i, value->xint[i]);
		else if(value->type == XFLT)
			g_debug("\tXFLT: %s[%i] = %f", value->name, i, value->xflt[i]);
	}
}
