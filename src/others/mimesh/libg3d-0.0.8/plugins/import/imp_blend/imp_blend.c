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

#include <g3d/types.h>
#include <g3d/stream.h>

#include "imp_blend_def.h"
#include "imp_blend_types.h"
#include "imp_blend_sdna.h"
#include "imp_blend_read.h"
#include "imp_blend_chunks.h"

static gboolean blend_read_file(BlendGlobal *global);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	gchar buf[BLEND_HEADER_SIZE + 1];
	guint32 flags = 0, version;
	BlendGlobal *global;

	if(g3d_stream_read(stream, buf, BLEND_HEADER_SIZE) != BLEND_HEADER_SIZE) {
		g_warning("Blend: %s: failed to read header", stream->uri);
		return FALSE;
	}
	buf[BLEND_HEADER_SIZE] = '\0';
	if(strncmp(buf, "BLENDER", 7) != 0) {
		g_warning("Blend: %s: not a blender file", stream->uri);
		return FALSE;
	}
	if(buf[7] == '_')
		flags |= FLAG_POINTERSIZE_4;
	if(buf[8] == 'v')
		flags |= FLAG_LITTLE_ENDIAN;
	version = atoi(buf + 9);

	g_debug("Blend: version %.2f, %s endian, pointer size is %d",
		version / 100.0,
		(flags & FLAG_LITTLE_ENDIAN) ? "little" : "big",
		(flags & FLAG_POINTERSIZE_4) ? 4 : 8);

	if(!(flags & FLAG_POINTERSIZE_4)) {
		g_warning("Blend: 8-byte pointers are not implemented as I lack a "
			"test file. Please send me one :).");
		return FALSE;
	}

	global = g_new0(BlendGlobal, 1);
	global->context = context;
	global->stream = stream;
	global->model = model;
	global->flags = flags;

	blend_read_file(global);

	g_free(global);

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("Blender models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("blend", ":", 0);
}

/*****************************************************************************/

static inline const BlendChunkInfo *blend_get_chunk_info(guint32 code,
	guint32 oid)
{
	gint32 i;

	for(i = 0; blend_chunks[i].code != 0; i ++)
		if((blend_chunks[i].code == code) && (blend_chunks[i].oid == oid))
			return &(blend_chunks[i]);
	if(oid != 0)
		return blend_get_chunk_info(code, 0);
	return NULL;
}

static gboolean blend_read_file(BlendGlobal *global)
{
	guint32 code, old, sdnanr, nr;
	gsize len;
	BlendLocal *local;
	const BlendChunkInfo *cinfo;
	const BlendSdnaStruct *sstruct;
	gint i;
	G3DObject *object = NULL;
	G3DObject *grpobject = NULL;
	guint32 object_id = 0;

	while(TRUE) {
		code = blend_read_uint(global->stream, global->flags);
		len = blend_read_uint(global->stream, global->flags);
		if(code == MKID('E','N','D','B'))
			return TRUE;

		old = blend_read_uint(global->stream, global->flags);
		sdnanr = blend_read_uint(global->stream, global->flags);
		nr = blend_read_uint(global->stream, global->flags);

		if((global->sdna == NULL) && (code != MKID('D','N','A','1'))) {
			g3d_stream_skip(global->stream, len);
			continue;
		}

		cinfo = blend_get_chunk_info(code, (code == ID_DATA) ? object_id : 0);
		if(cinfo == NULL)
			g_warning("unknown chunk: %c%c%c%c",
				blend_from_id(code, 0), blend_from_id(code, 1),
				blend_from_id(code, 2), blend_from_id(code, 3));

		g_debug("\\%c%c%c%c: %-20s - %8d bytes (0x%08x, %d, %d)",
			blend_from_id(code, 0), blend_from_id(code, 1),
			blend_from_id(code, 2), blend_from_id(code, 3),
			cinfo ? cinfo->description : "(unknown)",
			(int) len, old, sdnanr, nr);

		if(len == 0)
			return FALSE;

		/* on non-DATA nodes a new object starts */
		if(code != ID_DATA) {
			object_id = code;
		}

		local = g_new0(BlendLocal, 1);
		local->ndata = nr;
		local->data = g_new0(BlendSdnaData *, nr);

		if(global->sdna) {
			for(i = 0; i < nr; i ++) {
				sstruct = blend_sdna_get_struct_by_id(global->sdna, sdnanr);
				if(sstruct) {
#if DEBUG > BLEND_DEBUG_STRUCT
					g_debug("|struct %s /* %d */",
						sstruct->name, sstruct->size);
#endif
					local->data[i] = blend_sdna_data_read(global->sdna,
						sstruct, global, &len, 0);
				}
			}
		}

		local->object = object;
		local->grpobject = grpobject;
		local->object_id = object_id;
		local->len = len;

		if(cinfo && cinfo->callback)
			if(!cinfo->callback(global, local))
				return FALSE;

		len = local->len;
		object = local->object;
		grpobject = local->grpobject;

		for(i = 0; i < local->ndata; i ++)
			if(local->data[i])
				blend_sdna_data_free(local->data[i]);
		g_free(local->data);
		g_free(local);

		/* skip remaining data */
		g3d_stream_skip(global->stream, len);
	}

	blend_sdna_cleanup(global->sdna);
	return TRUE;
}
