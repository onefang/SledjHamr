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

#include <g3d/config.h>

#include <stdio.h>
#include <string.h>
#include <g3d/stream.h>
#include <g3d/read.h>
#include <g3d/iff.h>
#include <g3d/context.h>
#include <g3d/debug.h>

EAPI
gboolean g3d_iff_check(G3DStream *stream, guint32 *id, gsize *len)
{
	guint32 magic, form_bytes;

	magic = g3d_stream_read_int32_be(stream);
	if((magic != G3D_IFF_MKID('F','O','R','M')) &&
		(magic != G3D_IFF_MKID('F','O','R','4'))) {
		g_warning("IFF: %s is not a valid IFF stream", stream->uri);
		return FALSE;
	}
	form_bytes = g3d_stream_read_int32_be(stream);
	*id = g3d_stream_read_int32_be(stream);
	*len = form_bytes - 4;
	return TRUE;
}

#ifndef G3D_DISABLE_DEPRECATED
FILE *g3d_iff_open(const gchar *filename, guint32 *id, guint32 *len)
{
	FILE *f;
	guint32 form_bytes, magic;

	f = fopen(filename, "r");
	if(f == NULL)
	{
		g_critical("can't open file '%s'", filename);
		return NULL;
	}

	magic = g3d_read_int32_be(f);
	if((magic != G3D_IFF_MKID('F','O','R','M')) &&
		(magic != G3D_IFF_MKID('F','O','R','4')))
	{
		g_critical("file %s is not an IFF file", filename);
		fclose(f);
		return NULL;
	}

	form_bytes = g3d_read_int32_be(f);
	*id = g3d_read_int32_be(f);
	form_bytes -= 4;
	*len = form_bytes;

	return f;
}
#endif /* !G3D_DISABLE_DEPRECATED */

EAPI
gsize g3d_iff_read_chunk(G3DStream *stream, guint32 *id, gsize *len,
	guint32 flags)
{
	*id = g3d_stream_read_int32_be(stream);
	if(flags & G3D_IFF_LEN16) {
		*len = (flags & G3D_IFF_LE) ?
			g3d_stream_read_int16_le(stream) :
			g3d_stream_read_int16_be(stream);
		return 6 + *len + (*len % 2);
	} else {
		*len = (flags & G3D_IFF_LE) ?
			g3d_stream_read_int32_le(stream) :
			g3d_stream_read_int32_be(stream);
		return 8 + *len + (*len % 2);
	}
}

#ifndef G3D_DISABLE_DEPRECATED
int g3d_iff_readchunk(FILE *f, guint32 *id, guint32 *len, guint32 flags)
{
	*id = g3d_read_int32_be(f);
	if(flags & G3D_IFF_LEN16)
	{
		*len = (flags & G3D_IFF_LE) ?
			g3d_read_int16_le(f) : g3d_read_int16_be(f);
		return 6 + *len + (*len % 2);
	}
	else
	{
		*len = (flags & G3D_IFF_LE) ?
			g3d_read_int32_le(f) : g3d_read_int32_be(f);
		return 8 + *len + (*len % 2);
	}
}
#endif /* G3D_DISABLE_DEPRECATED */

EAPI
gchar *g3d_iff_id_to_text(guint32 id)
{
	gchar *tid;

	tid = g_new0(gchar, 5);

	tid[0] = (id >> 24) & 0xFF;
	tid[1] = (id >> 16) & 0xFF;
	tid[2] = (id >> 8) & 0xFF;
	tid[3] = id & 0xFF;

	return tid;
}

EAPI
gboolean g3d_iff_chunk_matches(guint32 id, gchar *tid)
{
	if(((id >> 24) & 0xFF) != tid[0]) return FALSE;
	if(((id >> 16) & 0xFF) != tid[1]) return FALSE;
	if(((id >> 8) & 0xFF) != tid[2]) return FALSE;
	return (id & 0xFF) == tid[3];
}

EAPI
G3DIffChunkInfo *g3d_iff_get_chunk_info(G3DIffChunkInfo *chunks,
	guint32 chunk_id)
{
	guint32 i = 0;

	while(chunks[i].id && !g3d_iff_chunk_matches(chunk_id, chunks[i].id))
		i ++;

	if(chunks[i].id)
		return &(chunks[i]);

	return NULL;
}

EAPI
void g3d_iff_debug_chunk(G3DIffChunkInfo *info, guint32 chunk_id,
	guint32 chunk_len, gchar chunk_type, guint32 pos, guint32 level)
{
	gchar *tid;
	gchar *padding = "                                   ";

	tid = g3d_iff_id_to_text(chunk_id);
	g_debug("\\%s(%d)[%s][%c%c%c] %s (%d) @ 0x%08x - %d bytes left",
		padding + (strlen(padding) - level), level,
		tid,
		chunk_type,
		info->container ? 'c' : ' ',
		info->callback ? 'f' : ' ',
		info->description,
		chunk_len,
		pos,
		chunk_len);
	g_free(tid);
}

static goffset g3d_iff_pos(G3DIffGlobal *global)
{
	if(global->stream)
		return g3d_stream_tell(global->stream);
	else
#ifndef G_DISABLE_DEPRECATED
		return ftell(global->f);
#else
		return -1;
#endif
}

EAPI
gpointer g3d_iff_handle_chunk(G3DIffGlobal *global, G3DIffLocal *plocal,
	G3DIffChunkInfo *chunks, guint32 flags)
{
	gpointer object = NULL;
	guint32 chunk_id;
	gsize chunk_len;
	G3DIffLocal *sublocal;
	G3DIffChunkInfo *info;

	/* read info for one chunk */
	if(global->stream)
		g3d_iff_read_chunk(global->stream, &chunk_id, &chunk_len, 0);
	else
#ifndef G3D_DISABLE_DEPRECATED
	{
		guint32 chunk_len32;
		g3d_iff_readchunk(global->f, &chunk_id, &chunk_len32, 0);
		chunk_len = chunk_len32;
	}
#else
		return NULL;
#endif

	plocal->nb -= 8;

	sublocal = g_new0(G3DIffLocal, 1);
	sublocal->parent_id = plocal->id;
	sublocal->id = chunk_id;
	sublocal->object = plocal->object;
	sublocal->level = plocal->level + 1;
	sublocal->nb = ((guint32)chunk_len);
	plocal->nb -= sublocal->nb;

	/* call function */
	info = g3d_iff_get_chunk_info(chunks, chunk_id);
	if(info)
	{
		g3d_iff_debug_chunk(info, chunk_id, chunk_len, 'X',
			(guint32)g3d_iff_pos(global) - 8, plocal->level);

		if(info->callback)
			info->callback(global, sublocal);

		if(sublocal->nb > 0) {
			if(global->stream) {
				g3d_stream_skip(global->stream, sublocal->nb);
			} else {
#ifndef G_DISABLE_DEPRECATED
				fseek(global->f, sublocal->nb, SEEK_CUR);
#else
				return NULL;
#endif
			}
		}
	}

	object = sublocal->level_object;
	g_free(sublocal);

	return object;
}

EAPI
gboolean g3d_iff_read_ctnr(G3DIffGlobal *global, G3DIffLocal *local,
	G3DIffChunkInfo *chunks, guint32 flags)
{
	G3DIffLocal *sublocal;
	G3DIffChunkInfo *info;
	guint32 chunk_id, chunk_mod, chunk_type;
	gsize chunk_len;
	gchar *tid;
	gpointer level_object;
#ifndef G3D_DISABLE_DEPRECATED
	long int fpos;
#endif

	level_object = NULL;

#ifndef G3D_DISABLE_DEPRECATED
	if(global->max_fpos == 0)
		global->max_fpos = local->nb + 12;
#endif
	while(local->nb >= ((flags & G3D_IFF_LEN16) ? 6 : 8))
	{
		chunk_id = 0;

		if(global->stream)
			g3d_iff_read_chunk(global->stream, &chunk_id, &chunk_len, flags);
		else
#ifndef G3D_DISABLE_DEPRECATED
		{
			guint32 chunk_len32 = ((guint32)chunk_len);
			g3d_iff_readchunk(global->f, &chunk_id, &chunk_len32, flags);
			chunk_len = chunk_len32;
		}
#else
			return FALSE;
#endif
		local->nb -= ((flags & G3D_IFF_LEN16) ? 6 : 8);

		chunk_mod = flags & 0x0F;
		if(chunk_mod == 0) {
			g_warning("[IFF] mod = 0 (flags: 0x%02X\n)", flags);
			chunk_mod = 2;
		}
		chunk_type = ' ';

		/* handle special chunks */
		switch(chunk_id)
		{
			case 0:
			case 0xFFFFFFFF:
				g_warning(
					"[IFF] got invalid ID, skipping %d bytes @ 0x%08x",
					local->nb, (guint32)g3d_iff_pos(global));

				/* skip rest of parent chunk */
				if(local->nb > 0)
				{
					fseek(global->f, local->nb, SEEK_CUR);
					local->nb = 0;
				}
				return FALSE;
				break;

			case G3D_IFF_MKID('F','O','R','4'):
				if(global->stream)
					chunk_id = g3d_stream_read_int32_be(global->stream);
				else
#ifndef G3D_DISABLE_DEPRECATED
					chunk_id = g3d_read_int32_be(global->f);
#else
					return FALSE;
#endif
				chunk_len -= 4;
				chunk_mod = 4;
				chunk_type = 'F';
				local->nb -= 4;
				break;

			case G3D_IFF_MKID('L','I','S','4'):
				if(global->stream)
					chunk_id = g3d_stream_read_int32_be(global->stream);
				else
#ifndef G3D_DISABLE_DEPRECATED
					chunk_id = g3d_read_int32_be(global->f);
#else
					return FALSE;
#endif
				chunk_len -= 4;
				chunk_mod = 4;
				chunk_type = 'L';
				local->nb -= 4;
				break;

			default:
				break;
		}

		info = g3d_iff_get_chunk_info(chunks, chunk_id);

		if(info) {
			g3d_iff_debug_chunk(info, chunk_id, chunk_len, chunk_type,
				(guint32)g3d_iff_pos(global) - ((chunk_type == ' ') ? 8 : 12),
				local->level);

			sublocal = g_new0(G3DIffLocal, 1);
			sublocal->parent_id = local->id;
			sublocal->id = chunk_id;
			sublocal->object = local->object;
			sublocal->level = local->level + 1;
			sublocal->level_object = level_object;
			sublocal->nb = ((guint32)chunk_len);

			if(info->callback)
				info->callback(global, sublocal);

			if(info->container) {
				/* LWO has 16 bit length in subchunks */
				if(flags & G3D_IFF_SUBCHUNK_LEN16)
					g3d_iff_read_ctnr(global, sublocal, chunks,
						flags | G3D_IFF_LEN16);
				else
					g3d_iff_read_ctnr(global, sublocal, chunks, flags);
			}

			if(info->container && info->callback) {
				sublocal->finalize = TRUE;
				info->callback(global, sublocal);
			}

			if(sublocal->nb > 0) {
				if(global->stream) {
					g3d_stream_skip(global->stream, sublocal->nb);
				} else {
#ifndef G3D_DISABLE_DEPRECATED
					fseek(global->f, sublocal->nb, SEEK_CUR);
#else
					return FALSE;
#endif
				}
			}
			level_object = sublocal->level_object;

			g_free(sublocal);
		} else {
			tid = g3d_iff_id_to_text(chunk_id);
			g_warning("[IFF] unknown chunk type \"%s\" (%d) @ 0x%08x",
				tid, (guint32)chunk_len, (guint32)g3d_iff_pos(global) - 8);
			g_free(tid);
			if(global->stream)
				g3d_stream_skip(global->stream, chunk_len);
			else
#ifndef G3D_DISABLE_DEPRECATED
				fseek(global->f, chunk_len, SEEK_CUR);
#else
				return FALSE;
#endif
		}

		local->nb -= chunk_len;

		if(chunk_len % chunk_mod) {
			if(global->stream)
				g3d_stream_skip(global->stream,
					chunk_mod - (chunk_len % chunk_mod));
			else
#ifndef G3D_DISABLE_DEPRECATED
				fseek(global->f,
					chunk_mod - (chunk_len % chunk_mod), SEEK_CUR);
#else
				return FALSE;
#endif
			local->nb -= (chunk_mod - (chunk_len % chunk_mod));
		}

		if(global->stream) {
			g3d_context_update_progress_bar(global->context,
				(G3DFloat)g3d_stream_tell(global->stream) /
				(G3DFloat)g3d_stream_size(global->stream), TRUE);
		}
#ifndef G3D_DISABLE_DEPRECATED
		else {
			fpos = ftell(global->f);
			g3d_context_update_progress_bar(global->context,
				((G3DFloat)fpos / (G3DFloat)global->max_fpos), TRUE);
		}
#endif
	} /* nb >= 8/6 */

	if(local->nb > 0)
	{
		g_warning("[IFF] skipping %d bytes at the end of chunk",
			local->nb);
		if(global->stream) {
			g3d_stream_skip(global->stream, local->nb);
		} else {
#ifndef G3D_DISABLE_DEPRECATED
			fseek(global->f, local->nb, SEEK_CUR);
#else
			return FALSE;
#endif
		}
		local->nb = 0;
	}

	return TRUE;
}

