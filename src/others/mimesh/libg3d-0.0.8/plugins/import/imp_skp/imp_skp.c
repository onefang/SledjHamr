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

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/read.h>
#include <g3d/stream.h>

#include "imp_skp.h"
#include "imp_skp_read.h"
#include "imp_skp_chunks.h"

static gboolean skp_parse_version_map(G3DStream *stream, guint32 *max_nlen,
	guint32 *max_version);
static gchar *skp_find_section(G3DStream *stream, guint32 max_nlen,
	guint32 max_version, guint32 *version);
static SkpChunkDesc *skp_get_chunk_desc(gchar *cname);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	gchar *smagic, *sversion, *stmp, *ssection;
	guint32 max_nlen = 0, max_version = 0, version = 0;
	SkpChunkDesc *desc;
	SkpGlobalData *global;
	SkpLocalData *local;

	smagic = skp_read_wchar(stream);
	if(smagic == NULL) {
		g_warning("not a valid .skp file: '%s'", stream->uri);
		return FALSE;
	}
	sversion = skp_read_wchar(stream);
	if(sversion == NULL) {
		g_warning("failed to read version from '%s'", stream->uri);
		g_free(smagic);
		return FALSE;
	}

	g_debug("SKP: magic: '%s', version: '%s'", smagic, sversion);
	g_free(smagic);
	g_free(sversion);

	g3d_stream_seek(stream, 16, G_SEEK_CUR);
	stmp = skp_read_wchar(stream);
	if(stmp != NULL)
		g_free(stmp);
	g3d_stream_read_int32_le(stream);

	ssection = skp_read_char(stream);
	if(ssection) {
		if(strcmp(ssection, "CVersionMap") == 0)
			g_debug("\\CVersionMap");
			skp_parse_version_map(stream, &max_nlen, &max_version);
		g_free(ssection);
	}

	global = g_new0(SkpGlobalData, 1);
	global->context = context;
	global->model = model;
	global->stream = stream;

	ssection = skp_find_section(stream, max_nlen, max_version, &version);
	while(ssection != NULL) {
		desc = skp_get_chunk_desc(ssection);
		if(desc == NULL) {
			g_warning("SKP: unknown chunk type '%s'", ssection);
		} else {
			if((version > desc->max_ver) || (version < desc->min_ver)) {
				g_warning("SKP: %s: unhandled version %u (%u - %u)",
					ssection, version, desc->min_ver, desc->max_ver);
			} else {
				if(desc->callback) {
					local = g_new0(SkpLocalData, 1);
					local->id = desc->id;
					local->version = version;

					desc->callback(global, local);

					g_free(local);
				}
			} /* version check */
		} /* has desc */

		ssection = skp_find_section(stream, max_nlen, max_version, &version);
	} /* sections */

	/* clean up */
	g_free(global);

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("SketchUp models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("skp", ":", 0);
}


/*****************************************************************************/

static gboolean skp_parse_version_map(G3DStream *stream, guint32 *max_nlen,
	guint32 *max_version)
{
	gchar *part;
	guint32 version;

	while(TRUE) {
		part = skp_read_wchar(stream);
		if(part == NULL)
			return FALSE;
		version = g3d_stream_read_int32_le(stream);
#if DEBUG > 1
		g_debug("\t%-30s %u", part, version);
#endif
		if(strcmp(part, "End-Of-Version-Map") == 0) {
			g_free(part);
			return TRUE;
		}

		if(version > *max_version)
			*max_version = version;
		if(strlen(part) > *max_nlen)
			*max_nlen = strlen(part);

		g_free(part);
	}
	return FALSE;
}

static gchar *skp_find_section(G3DStream *stream, guint32 max_nlen,
	guint32 max_version, guint32 *version)
{
	goffset offset;
	guint32 ver, nlen;
	guint16 w1;
	gchar *name;

	while(!g3d_stream_eof(stream) && (g3d_stream_read_int8(stream) != 0xFF));

	if(g3d_stream_eof(stream))
		return NULL;

	offset = g3d_stream_tell(stream);
	if(g3d_stream_read_int8(stream) != 0xFF) {
		g3d_stream_seek(stream, offset, G_SEEK_SET);
		return skp_find_section(stream, max_nlen, max_version, version);
	}

	ver = g3d_stream_read_int16_le(stream);
	if(ver > max_version) {
		g3d_stream_seek(stream, offset, G_SEEK_SET);
		return skp_find_section(stream, max_nlen, max_version, version);
	}

	nlen = g3d_stream_read_int16_le(stream);
	if(nlen > max_nlen) {
		g3d_stream_seek(stream, offset, G_SEEK_SET);
		return skp_find_section(stream, max_nlen, max_version, version);
	}

	name = g_new0(gchar, nlen + 1);
	g3d_stream_read(stream, name, nlen);
	if(name[0] != 'C') {
		g_free(name);
		g3d_stream_seek(stream, offset, G_SEEK_SET);
		return skp_find_section(stream, max_nlen, max_version, version);
	}
	*version = ver;

	w1 = g3d_stream_read_int16_le(stream);
	g3d_stream_seek(stream, -2, G_SEEK_CUR);

	g_debug("\\%-30s v%-2u @ 0x%08x (0x%04x)", name, ver,
		(guint32)g3d_stream_tell(stream), w1);

	return name;
}

static SkpChunkDesc *skp_get_chunk_desc(gchar *cname)
{
	guint32 i;

	for(i = 0; skp_chunks[i].id != NULL; i ++) {
		if(strcmp(cname, skp_chunks[i].id) == 0)
			return &(skp_chunks[i]);
	}
	return NULL;
}
