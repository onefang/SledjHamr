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
#include <g3d/stream.h>
#include <g3d/debug.h>

static gboolean c4d_load_v5(G3DContext *context, G3DStream *stream,
	G3DModel *model);
static gboolean c4d_load_v6(G3DContext *context, G3DStream *stream,
	G3DModel *model);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	gchar magic[5];

	g3d_stream_read(stream, magic, 4);
	magic[4] = '\0';
	if(strncmp(magic + 1, "C4D", 3) == 0) {
		g3d_stream_read(stream, magic, 4);
		if(strncmp(magic, "C4D6", 4) == 0)
			return c4d_load_v6(context, stream, model);
		else {
			g_warning("unknown C4D magic: %s", magic);
			return FALSE;
		}
	} else if(strncmp(magic + 1, "C50", 3) == 0) {
		g_debug("C4D v5: %s", magic);
		return c4d_load_v5(context, stream, model);
	} else {
		g_warning("C4D: unknown magic %s, not a C4D file?", magic);
		return FALSE;
	}
	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("Cinema4D models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("c4d", ":", 0);
}

/*****************************************************************************/

static gchar *c4d_read_wchar(G3DStream *stream, gsize *n_bytes)
{
	guint32 len;
	gunichar2 *u16text;
	gchar *text;
	gint32 i;
	GError *error = NULL;

	len = g3d_stream_read_int32_be(stream);
	*n_bytes += 4 + len;
	u16text = g_new0(gunichar2, len + 1);
	for(i = 0; i < len / 2; i ++)
		u16text[i] = g3d_stream_read_int16_be(stream);
	text = g_utf16_to_utf8(u16text, len, NULL, NULL, &error);
	if(error != NULL) {
		g_warning("UTF-16 to UTF-8 conversion failed: %s", error->message);
		g_error_free(error);
	}
	g_free(u16text);
	return text;
}

#define C4D_DEBUG_OPCODE 0

static gboolean c4d_handle_opcode(guint8 opcode, G3DStream *stream,
	G3DModel *model, gsize *n_bytes, guint32 *level)
{
	guint8 u1, *data;
	guint16 w1, w2, w3;
	guint32 x1, x3;
	gsize x2;
	G3DFloat f1, f2, f3;
	gchar *str;
	gint32 i;

	switch(opcode) {
		case 0x01: /* 64 bit something */
			*level += 1;
			x1 = g3d_stream_read_int32_be(stream);
			x2 = g3d_stream_read_int32_be(stream);
			*n_bytes += 8;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s01: 0x%08x 0x%08x", debug_pad(*level), x1, x2);
#endif
			break;
		case 0x02: /* no payload */
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s02", debug_pad(*level));
#endif
			if(*level == 0) {
				g_warning("E: 02: level == 0)");
			} else
				*level -= 1;
			break;
		case 0x0C: /* 8 bit something */
			u1 = g3d_stream_read_int8(stream);
			*n_bytes += 1;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s0C: 0x%02x", debug_pad(*level), u1);
#endif
			break;
		case 0x0F: /* 32 bit something */
			x1 = g3d_stream_read_int32_be(stream);
			*n_bytes += 4;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s0F: 0x%08x", debug_pad(*level), x1);
#endif
			break;
		case 0x10: /* 32 bit something */
			x1 = g3d_stream_read_int32_be(stream);
			*n_bytes += 4;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s10: 0x%08x", debug_pad(*level), x1);
#endif
			break;
		case 0x13: /* float */
			f1 = g3d_stream_read_float_be(stream);
			*n_bytes += 4;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s13: %.3f", debug_pad(*level), f1);
#endif
			break;
		case 0x15: /* 8 bit something */
			u1 = g3d_stream_read_int8(stream);
			*n_bytes += 1;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s15: 0x%02x", debug_pad(*level), u1);
#endif
			break;
		case 0x16: /* 2 x float */
			f1 = g3d_stream_read_float_be(stream);
			f2 = g3d_stream_read_float_be(stream);
			*n_bytes += 8;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s16: %.3f, %.3f", debug_pad(*level), f1, f2);
#endif
			break;
		case 0x17: /* 3 x float */
			f1 = g3d_stream_read_float_be(stream);
			f2 = g3d_stream_read_float_be(stream);
			f3 = g3d_stream_read_float_be(stream);
			*n_bytes += 12;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s17: %.3f, %.3f, %.3f", debug_pad(*level), f1, f2, f3);
#endif
			break;
		case 0x19: /* 12 x float */
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s19:", debug_pad(*level));
#endif
			for(i = 0; i < 4; i ++) {
				f1 = g3d_stream_read_float_be(stream);
				f2 = g3d_stream_read_float_be(stream);
				f3 = g3d_stream_read_float_be(stream);
				*n_bytes += 12;
#if DEBUG > C4D_DEBUG_OPCODE
				g_debug("\\%s    %.3f, %.3f, %.3f", debug_pad(*level),
					f1, f2, f3);
#endif
			}
			break;
		case 0x80: /* data */
			x1 = g3d_stream_read_int32_be(stream);
			*n_bytes += 4 + x1;
			data = g_new0(guint8, x1);
			g3d_stream_read(stream, data, x1);
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s80: %d bytes of data", debug_pad(*level), x1);
#endif
			g_free(data);
			break;
		case 0x81: /* embedded file */
			x1 = g3d_stream_read_int32_be(stream);
			*n_bytes += 4;
			g3d_stream_skip(stream, x1);
			*n_bytes += x1;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s81: skipped %d bytes", debug_pad(*level), x1);
#endif
			break;
		case 0x82: /* wide-char string */
			str = c4d_read_wchar(stream, n_bytes);
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s82: %s", debug_pad(*level), str);
#endif
			if(str)
				g_free(str);
			break;
		case 0x83: /* file name */
			str = c4d_read_wchar(stream, n_bytes);
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s83: %s", debug_pad(*level), str);
#endif
			if(str)
				g_free(str);
			break;
		case 0x84: /* typed content */
			x1 = g3d_stream_read_int32_be(stream);
			*n_bytes += 4;
			x2 = 0;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s84: 0x%08x", debug_pad(*level), x1);
#endif
			*level += 1;
			while(x2 < x1) {
				u1 = g3d_stream_read_int8(stream);
				x2 ++;
				if(!c4d_handle_opcode(u1, stream, model, &x2, level))
					return FALSE;
				x3 = g3d_stream_read_int32_be(stream);
				x2 += 4;
				u1 = g3d_stream_read_int8(stream);
				x2 += 1;
			}
			*level -= 1;
			*n_bytes += x1;
			break;
		case 0x85: /* n * 3 * float */
			x1 = g3d_stream_read_int32_be(stream);
			*n_bytes += 4;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s85:", debug_pad(*level));
#endif
			for(i = 0; i < (x1 / 12); i ++) {
				f1 = g3d_stream_read_float_be(stream);
				f2 = g3d_stream_read_float_be(stream);
				f3 = g3d_stream_read_float_be(stream);
				*n_bytes += 12;
#if DEBUG > C4D_DEBUG_OPCODE
				g_debug("\\%s    %.3f, %.3f, %.3f", debug_pad(*level),
					f1, f2, f3);
#endif
			}
			break;
		case 0x86: /* n * int16 */
			x1 = g3d_stream_read_int32_be(stream);
			*n_bytes += 4;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s86:", debug_pad(*level));
#endif
			for(i = 0; i < (x1 / 2); i ++) {
				w1 = g3d_stream_read_int16_be(stream);
				*n_bytes += 2;
#if DEBUG > C4D_DEBUG_OPCODE
				g_debug("\\%s    0x%04x", debug_pad(*level), w1);
#endif
			}
			break;
		case 0x87: /* n * 3 * int16 */
			x1 = g3d_stream_read_int32_be(stream);
			*n_bytes += 4;
#if DEBUG > C4D_DEBUG_OPCODE
			g_debug("\\%s87:", debug_pad(*level));
#endif
			for(i = 0; i < (x1 / 6); i ++) {
				w1 = g3d_stream_read_int16_be(stream);
				w2 = g3d_stream_read_int16_be(stream);
				w3 = g3d_stream_read_int16_be(stream);
				*n_bytes += 6;
#if DEBUG > C4D_DEBUG_OPCODE
				g_debug("\\%s    0x%04x 0x%04x 0x%04x", debug_pad(*level),
					w1, w2, w3);
#endif
			}
			break;
		default:
			if(g3d_stream_eof(stream))
				return TRUE;
#if DEBUG > 0
			g_debug("%sunknown opcode 0x%02X @ 0x%08x", debug_pad(*level),
				opcode, (guint32)g3d_stream_tell(stream));
#endif
			return FALSE;
	}
	return TRUE;
}

static gboolean c4d_read_v5_cntr(G3DContext *context, G3DStream *stream,
	G3DModel *model, goffset *n_bytes, guint32 level)
{
	guint32 id;
	goffset size;

	while(!g3d_stream_eof(stream) && (*n_bytes > 0)) {
		id = g3d_stream_read_int32_be(stream);
		size = g3d_stream_read_int32_be(stream);
		*n_bytes -= 8;
		if(*n_bytes < 0)
			return FALSE;
#if DEBUG > 0
		g_debug("\\%s0x%08x @ 0x%08x (%u bytes)", debug_pad(level), id,
			(guint32)g3d_stream_tell(stream), (guint32)size);
#endif
		*n_bytes -= size;
		if((id == 0x00001647) || (id == 0x0000139c)) {
			if(!c4d_read_v5_cntr(context, stream, model, &size, level + 1))
				return FALSE;
		} else {
			g3d_stream_skip(stream, size);
		}
	}
	return TRUE;
}

static gboolean c4d_load_v5(G3DContext *context, G3DStream *stream,
	G3DModel *model)
{
	goffset n_bytes;

	n_bytes = g3d_stream_read_int32_be(stream); /* file size */
	g3d_stream_read_int32_be(stream); /* DOK5 */
	g3d_stream_read_int32_be(stream); /* unknown 1 */

	return c4d_read_v5_cntr(context, stream, model, &n_bytes, 0);
}

static gboolean c4d_load_v6(G3DContext *context, G3DStream *stream,
	G3DModel *model)
{
	guint8 opcode;
	gsize n_bytes = 0;
	guint32 level = 0;

	while(!g3d_stream_eof(stream)) {
		opcode = g3d_stream_read_int8(stream);
		if(!c4d_handle_opcode(opcode, stream, model, &n_bytes, &level))
			return FALSE;
	}
	return TRUE;
}
