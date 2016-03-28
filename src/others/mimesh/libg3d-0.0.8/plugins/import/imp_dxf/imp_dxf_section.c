/* $Id: imp_dxf_section.c 312 2008-11-17 18:28:56Z mmmaddd $ */

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
#include <math.h>

#include <g3d/context.h>

#include "imp_dxf.h"
#include "imp_dxf_prop.h"
#include "imp_dxf_chunks.h"
#include "imp_dxf_entities.h"

static DxfChunkInfo *dxf_get_chunk_info(DxfChunkInfo *chunks, gint32 id)
{
	guint32 i;

	for(i = 0; chunks[i].id != DXF_CODE_INVALID; i ++)
		if(chunks[i].id == id)
			return &(chunks[i]);
	return NULL;
}

static DxfEntityInfo *dxf_get_entity_info(const gchar *str)
{
	guint32 i;

	for(i = 0; dxf_entities[i].name != NULL; i ++)
		if(strcmp(dxf_entities[i].name, str) == 0)
			return &(dxf_entities[i]);
#if DEBUG > 0
	g_debug("unknown entity: %s", str);
#endif
	return NULL;
}

static gboolean dxf_entity_finalize(DxfGlobalData *global, guint32 sid,
	DxfEntityData *edata, DxfEntityInfo *einfo, DxfEntityProps *eprop)
{
	DxfLocalData *local;

	if(einfo->callback) {
		local = g_new0(DxfLocalData, 1);
		local->sid = sid;
		local->eid = einfo->id;
		local->edata = edata;
		local->eprop = eprop;

		einfo->callback(global, local);

		g_free(local);
	}
	return TRUE;
}

static gboolean dxf_read_chunk(DxfGlobalData *global, DxfChunkInfo *cinfo,
	DxfEntityProps *eprop)
{
	gint32 i;
	gdouble dbl;
	gchar str[DXF_MAX_LINE + 1];

	switch(cinfo->type) {
		case DXF_T_UNKNOWN:
			if(cinfo->id == 9) /* variable name */
				return dxf_debug_var(global, NULL);
			break;
		case DXF_T_EMPTY:
			return TRUE;
			break;
		case DXF_T_INT16:
			i = dxf_read_int16(global);
			dxf_prop_set_int(eprop, cinfo->id, i);
			break;
		case DXF_T_INT32:
			i = dxf_read_int32(global);
			dxf_prop_set_int(eprop, cinfo->id, i);
			break;
		case DXF_T_FLOAT64:
			dbl = dxf_read_float64(global);
			dxf_prop_set_dbl(eprop, cinfo->id, dbl);
			break;
		case DXF_T_STRING:
			dxf_read_string(global, str);
			dxf_prop_set_str(eprop, cinfo->id, str);
			break;
	}
	return TRUE;
}

static gboolean dxf_parse_chunks(DxfGlobalData *global, DxfChunkInfo *chunks,
	gint32 parentid, const gchar *section)
{
	gint32 key;
	DxfChunkInfo *chunk_info;
	DxfEntityData *edata;
	DxfEntityInfo *einfo = NULL;
	DxfEntityProps *eprop = NULL;
	gchar str[DXF_MAX_LINE + 1];
	G3DFloat pcnt, prev_pcnt = 0.0;

#if DEBUG > 0
	g_debug("\\[%s]", section);
#endif

	edata = g_new0(DxfEntityData, 1);

	if((strcmp(section, "ENTITIES") == 0) ||
		(strcmp(section, "BLOCKS") == 0))	{
		edata->object = g_slist_nth_data(global->model->objects, 0);
		edata->material = g_slist_nth_data(edata->object->materials, 0);
	}

	while(TRUE) {
		key = dxf_read_code(global);
		chunk_info = dxf_get_chunk_info(chunks, key);

		if(key == DXF_CODE_INVALID) {
			g_free(edata);
			return FALSE;
		}

		if(key == 0) { /* new entity or end of section */
			if(einfo) {
				dxf_entity_finalize(global, parentid, edata, einfo, eprop);
				dxf_prop_cleanup(eprop);
				eprop = NULL;
			}
			dxf_read_string(global, str);
			DXF_TEST_ENDSEC(str);
			einfo = dxf_get_entity_info(str);
			eprop = dxf_prop_create();
#if DEBUG > 0
			g_debug("|  entity: %s", str);
#endif
		}

#if DEBUG > 0
		if(chunk_info)
			g_debug("\\ %s[%+4d]: %s%s%s (line %d)",
				key ? " " : "",
				key, chunk_info->description,
				key ? "" : ": ",
				key ? "" : str,
				g3d_stream_line(global->stream));
		else
			g_warning("unknown chunk type %d in line %d", key,
				g3d_stream_line(global->stream));
#endif

		if(chunk_info) {
			dxf_read_chunk(global, chunk_info, eprop);
		} /* chunk_info */
		else {
			DXF_HANDLE_UNKNOWN(global, key, str, section);
		}

		pcnt = (G3DFloat)g3d_stream_tell(global->stream) /
			(G3DFloat)g3d_stream_size(global->stream);
		if((pcnt - prev_pcnt) > 0.01) {
			prev_pcnt = pcnt;
			g3d_context_update_progress_bar(global->context, pcnt, TRUE);
		}
		g3d_context_update_interface(global->context);
	} /* endless loop */

	g_free(edata);
	return FALSE;
}

gboolean dxf_section_HEADER(DxfGlobalData *global)
{
#if 0
	return dxf_parse_chunks(global, dxf_chunks, DXF_ID_HEADER, "HEADER");
#else
	return dxf_skip_section(global);
#endif
}

gboolean dxf_section_ENTITIES(DxfGlobalData *global)
{
	return dxf_parse_chunks(global, dxf_chunks, DXF_ID_ENTITIES, "ENTITIES");
}

gboolean dxf_section_BLOCKS(DxfGlobalData *global)
{
	return dxf_parse_chunks(global, dxf_chunks, DXF_ID_BLOCKS, "BLOCKS");
}

gboolean dxf_section_TABLES(DxfGlobalData *global)
{
#if 0
	return dxf_parse_chunks(global, dxf_chunks, DXF_ID_TABLES, "TABLES");
#else
	return dxf_skip_section(global);
#endif
}

gboolean dxf_section_OBJECTS(DxfGlobalData *global)
{
#if 0
	return dxf_parse_chunks(global, dxf_chunks, DXF_ID_OBJECTS, "OBJECTS");
#else
	return dxf_skip_section(global);
#endif
}

gboolean dxf_section_CLASSES(DxfGlobalData *global)
{
#if 0
	return dxf_parse_chunks(global, dxf_chunks, DXF_ID_CLASSES, "TABLES");
#else
	return dxf_skip_section(global);
#endif
}
