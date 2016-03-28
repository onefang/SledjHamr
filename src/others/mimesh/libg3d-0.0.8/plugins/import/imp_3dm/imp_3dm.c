/* $Id$ */

/*
    libg3d - 3D object loading library

    Copyright (C) 2005 - 2009  Markus Dahms <mad@automagically.de>

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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/debug.h>

#include "imp_3dm.h"
#include "imp_3dm_chunks.h"

static gboolean tdm_read_container(TdmGlobal *global, gpointer object,
	goffset nb, guint32 level);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	TdmGlobal *global;
	G3DFloat version;
	gchar magic[33], *pver;
	gboolean retval;
	guint32 max_len;

	memset(magic, '\0', 33);
	g3d_stream_read(stream, magic, 32);
	if(strncmp(magic, "3D Geometry File Format ", 24) != 0) {
		g_warning("%s is not an OpenNURBS 3dm file", stream->uri);
		return FALSE;
	}
	pver = magic + 24;
	while(isspace(*pver))
		pver ++;
	version = strtod(pver, NULL);
#if DEBUG > 0
	g_debug("loading %s, version %.1f", stream->uri, version);
#endif

	global = g_new0(TdmGlobal, 1);
	global->context = context;
	global->stream = stream;
	global->model = model;
	max_len = g3d_stream_size(stream);
	if(!max_len)
		max_len = (guint32) -1;
	retval = tdm_read_container(global, NULL, max_len - 32, 0);
	g_free(global);

	return retval;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("OpenNURBS models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("3dm", ":", 0);
}

/*****************************************************************************/

static TdmChunkInfo *tdm_get_chunk_info(guint32 tcode)
{
	gint32 i;

	for(i = 0; tdm_chunks[i].tcode <= tcode; i ++)
		if(tdm_chunks[i].tcode == tcode)
			return &(tdm_chunks[i]);
	return NULL;
}

static gboolean tdm_read_container(TdmGlobal *global, gpointer object,
	goffset nb, guint32 level)
{
	TdmChunkInfo *chunkinfo;
	TdmLocal *local;
	guint32 tcode;
	guint32 len;
	goffset off;

	while(TRUE) {
		off = g3d_stream_tell(global->stream);
		tcode = g3d_stream_read_int32_le(global->stream);
		len = g3d_stream_read_int32_le(global->stream);
		nb -= 8;
		if(tcode == 0)
			return TRUE;

		/* mask out DATA and CRC */
		chunkinfo = tdm_get_chunk_info(tcode & 0x7FFF7FFF);

#if DEBUG > 0
		g_debug("\\%s[0x%08x][%c%c%c] %s (%d bytes @ 0x%08x)",
			debug_pad(level), tcode,
			(chunkinfo && chunkinfo->container) ? 'c' : ' ',
			(chunkinfo && chunkinfo->endofcnt) ? 'e' : ' ',
			(chunkinfo && chunkinfo->callback) ? 'f' : ' ',
			chunkinfo ? chunkinfo->description : "unknown chunk",
			(tcode & TCODE_DATA) ? 0 : len,
			(guint32)off);
#endif
		if(chunkinfo && chunkinfo->endofcnt)
			return TRUE;

#if DEBUG > 0
		if(tcode & TCODE_DATA)
			g_debug("|%sdata: 0x%08x", debug_pad(level + 1), len);
#endif

		if(chunkinfo) {
			if(chunkinfo->callback) {
				local = g_new0(TdmLocal, 1);
				local->tcode = tcode;
				local->len = (tcode & TCODE_DATA) ? 0 : len;
				local->data = (tcode & TCODE_DATA) ? len : 0;
				local->level = level;
				local->object = object;
				chunkinfo->callback(global, local);
				len = local->len;
				object = local->object;
				g_free(local);
			}
			if(chunkinfo->container) {
				if(!tdm_read_container(global, object, len, level + 1))
					return FALSE;
				nb -= len;
				len = 0;
				if(tcode & TCODE_CRC) {
					g3d_stream_read_int32_le(global->stream);
					nb -= 4;
				}
			}
		}

		if(tcode & TCODE_DATA)
			continue;

		if(len > 0)
			g3d_stream_skip(global->stream, len);
	}

	return TRUE;
}
