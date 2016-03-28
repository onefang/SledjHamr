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
#include <g3d/types.h>
#include <g3d/material.h>

#include "imp_skp.h"
#include "imp_skp_callbacks.h"
#include "imp_skp_read.h"
#include "imp_skp_types.h"

gboolean skp_cb_arc_curve(SkpGlobalData *global, SkpLocalData *local)
{
	guint16 w1;

	w1 = g3d_stream_read_int16_le(global->stream);
	g_return_val_if_fail(w1 == 0x0000, FALSE);

	return TRUE;
}

/*****************************************************************************/

gboolean skp_cb_attribute_container(SkpGlobalData *global, SkpLocalData *local)
{
	guint16 w1;

	w1 = g3d_stream_read_int16_le(global->stream);
	g_return_val_if_fail(w1 == 0x0000, FALSE);

	return TRUE;
}

/*****************************************************************************/

gboolean skp_cb_attribute_named(SkpGlobalData *global, SkpLocalData *local)
{
	guint32 x1;
	guint16 w1, w2, w3;
	guint8 u1, u2;
	gdouble d1;
	gchar *name, *val, *tmp;

	w1 = g3d_stream_read_int16_le(global->stream);
	w2 = g3d_stream_read_int16_le(global->stream);
	w3 = g3d_stream_read_int16_le(global->stream);

	name = skp_read_wchar(global->stream);
	while(name != NULL) {
		tmp = skp_read_wchar(global->stream);
		if(tmp != NULL) {
			/* got a section */
#if DEBUG > 1
			g_debug("\t%s", name);
#endif
			g_free(name);
			name = tmp;
			continue;
		}
		u1 = g3d_stream_read_int8(global->stream);
		switch(u1) {
			case 0x00: /* end of CAttributeNamed */
				g_free(name);
				return TRUE;
				break;
			case 0x04: /* 32-bit */
				x1 = g3d_stream_read_int32_le(global->stream);
#if DEBUG > 1
				g_debug("\t\t%-20s: 0x%08x", name, x1);
#endif
				break;
			case 0x06: /* double */
				d1 = g3d_stream_read_double_le(global->stream);
#if DEBUG > 1
				g_debug("\t\t%-20s: %.4f", name, d1);
#endif
				break;
			case 0x07: /* boolean */
				u2 = g3d_stream_read_int8(global->stream);
#if DEBUG > 1
				g_debug("\t\t%-20s: %s", name, (u2 ? "true" : "false"));
#endif
				break;
			case 0x09: /* end of section? */
				g3d_stream_seek(global->stream, 7, G_SEEK_CUR);
#if DEBUG > 1
				g_debug("\t\t%-20s: EOS", name);
#endif
				break;
			case 0x0A: /* string */
				val = skp_read_wchar(global->stream);
				g_return_val_if_fail(val != NULL, FALSE);
#if DEBUG > 1
				g_debug("\t\t%-20s: %s", name, val);
#endif
				g_free(val);
				break;
			default:
				g_warning(
					"SKP: CAttributeNamed: unknown attribute type: 0x%02X",
					u1);
				return FALSE;
				break;
		}

		g_free(name);
		name = skp_read_wchar(global->stream);
	}
	return TRUE;
}

/*****************************************************************************/

gboolean skp_cb_component_definition(SkpGlobalData *global,
	SkpLocalData *local)
{
	SkpComponent *comp;
	guint32 x1, w1;
	gchar *s;
	gdouble d1, d2;

	w1 = g3d_stream_read_int16_le(global->stream);
	g_return_val_if_fail(w1 == 0x0000, FALSE);

	skp_read_10b(global->stream);
	skp_read_10b(global->stream);

	x1 = g3d_stream_read_int32_le(global->stream);
	g_debug("CComponentDefinition: %d", x1);

	w1 = g3d_stream_read_int16_le(global->stream);
	if(!(w1 & 0x8000)) {
		g_warning("CComponentDefinition: layerid=0x%04x", w1);
		return FALSE;
	}
	w1 &= 0x7FFF;
	g3d_stream_read_int16_le(global->stream); /* 0000 */

	s = skp_read_wchar(global->stream);
	if(s == NULL) {
		g_warning("CComponentDefinition: s[0] == NULL");
		return FALSE;
	}
	g3d_stream_seek(global->stream, 3, G_SEEK_CUR);
	s = skp_read_wchar(global->stream);
	if(s == NULL) {
		g_warning("CComponentDefinition: s[1] == NULL");
		return FALSE;
	}
	g3d_stream_seek(global->stream, 6, G_SEEK_CUR);
	s = skp_read_wchar(global->stream);
	if(s == NULL) {
		g_warning("CComponentDefinition: s[2] == NULL");
		return FALSE;
	}

	d1 = g3d_stream_read_double_le(global->stream);
	d2 = g3d_stream_read_double_le(global->stream);
	g_debug("CComponentDefinition: d1=%.2f, d2=%.2f", d1, d2);

	g3d_stream_seek(global->stream, 5, G_SEEK_CUR);

	comp = g_new0(SkpComponent, 1);
	comp->layerid = w1;

	comp->id0 = g3d_stream_read_int16_le(global->stream);
	g3d_stream_read_int32_le(global->stream);
	g3d_stream_read_int8(global->stream);
	comp->id1 = g3d_stream_read_int16_le(global->stream);

	g_debug("Component: 0x%02x, 0x%02x (0x%02x)",
		comp->id0, comp->id1, comp->layerid);

	global->components = g_slist_append(global->components, comp);

	return TRUE;
}

/*****************************************************************************/

static gboolean skp_read_5b(G3DStream *stream)
{
	guint32 w1, w2, u1;

	w1 = g3d_stream_read_int16_le(stream);
	u1 = g3d_stream_read_int8(stream);
	w2 = g3d_stream_read_int16_le(stream);
	g_debug("\tread 5b: %04x %02x %04x", w1, u1, w2);
	return TRUE;
}

gboolean skp_cb_edge_use(SkpGlobalData *global, SkpLocalData *local)
{
	guint16 w1, w2;
	guint32 x1;
	gdouble d1, d2, d3, d4;
	gboolean handled;
	GSList *item;
	SkpComponent *comp;

	w1 = g3d_stream_read_int16_le(global->stream);
	g_return_val_if_fail(w1 == 0x0000, FALSE);

	skp_read_5b(global->stream);

	do {
		w1 = g3d_stream_read_int16_le(global->stream);
		handled = FALSE;

		g_debug("opcode: 0x%04x", w1);
		if(w1 == 0xFFFF) {
			g3d_stream_seek(global->stream, -2, G_SEEK_CUR);
			return TRUE;
		}
		if(w1 & 0x8000) {
			g3d_stream_read_int16_le(global->stream);
			w1 &= 0x7FFF;
		}
		if(w1 == 0x0000) {
			handled = TRUE;
		}

		for(item = global->components;
			(handled == FALSE) && (item != NULL);
			item = item->next) {

			comp = item->data;

			switch(w1 - comp->id0) {
				case 1:
					skp_read_10b(global->stream);
					w2 = g3d_stream_read_int16_le(global->stream);
					handled = TRUE;
					g_debug("\tw2=0x%04x", w2);
					break;
				case 3:
					skp_read_dbl3(global->stream, &d1, &d2, &d3);
					g_debug("\tvertex: %.4f, %.4f, %.4f", d1, d2, d3);
					handled = TRUE;
					break;
				case 4:
				case 5:
				case 7:
					g_debug("\tempty");
					handled = TRUE;
					break;
				default:
					break;
			}
			if(handled)
				break;
			switch(w1 - comp->id1) {
				case 0:
					skp_read_10b(global->stream);
					d1 = g3d_stream_read_double_le(global->stream);
					d2 = g3d_stream_read_double_le(global->stream);
					d3 = g3d_stream_read_double_le(global->stream);
					d4 = g3d_stream_read_double_le(global->stream);
					x1 = g3d_stream_read_int32_le(global->stream);
					g_debug("\tid1+4: dbl4: %.2f, %.2f, %.2f, %.2f (%i)",
						d1, d2, d3, d4, x1);
					handled = TRUE;
					break;
				case 2:
					w2 = g3d_stream_read_int16_le(global->stream);
					g_debug("\ti16: 0x%04x", w2);
					handled = TRUE;
					break;
				case 4:
					skp_read_5b(global->stream);
					handled = TRUE;
					break;
				case 5:
				case 9:
				case 17:
				case 19:
					g_debug("\tempty");
					handled = TRUE;
					break;
				default:
					break;
			}
		} /* loop through known layers */

		if(handled == FALSE) {
			g_debug("vertex: unknown opcode %#04x (layer0: 0x%04x, 0x%04x)",
				w1,
				global->components ?
					((SkpComponent *)(global->components->data))->id0 : -1,
				global->components ?
					((SkpComponent *)(global->components->data))->id1 : -1);
		}
	} while(TRUE);

	return FALSE;

}

/*****************************************************************************/

gboolean skp_cb_face_texture_coords(SkpGlobalData *global, SkpLocalData *local)
{
#if DEBUG > 1
	gint32 i;
	guint16 w1;
	G3DFloat f1, f2, f3, f4;

	w1 = g3d_stream_read_int16_le(global->stream);
	g_return_val_if_fail(w1 == 0x0000, FALSE);

	for(i = 0; i < 24; i ++) {
		f1 = g3d_stream_read_float_le(global->stream);
		f2 = g3d_stream_read_float_le(global->stream);
		f3 = g3d_stream_read_float_le(global->stream);
		f4 = g3d_stream_read_float_le(global->stream);
		g_debug("\tf: %.4f, %.4f, %.4f, %.4f", f1, f2, f3, f4);
	}
#endif
	return TRUE;
}

/*****************************************************************************/

gboolean skp_cb_layer(SkpGlobalData *global, SkpLocalData *local)
{
	guint32 x1;
	guint16 w1;
	guint8 u1, u2, u3;
	gchar *s1, *s2;

	do {
		w1 = g3d_stream_read_int16_le(global->stream);
		g_return_val_if_fail(w1 == 0x0000, FALSE);

		s1 = skp_read_wchar(global->stream);
		u1 = g3d_stream_read_int8(global->stream);
		u2 = g3d_stream_read_int8(global->stream);
		u3 = g3d_stream_read_int8(global->stream);

		g_debug("\\ %s (0x%02X, 0x%02X, 0x%02X)", s1, u1, u2, u3);

		s2 = skp_read_wchar(global->stream);
		u1 = g3d_stream_read_int8(global->stream);
		u2 = g3d_stream_read_int8(global->stream);
		g_return_val_if_fail(u2 == 0x01, FALSE);

		/* layer color? */
		x1 = g3d_stream_read_int32_le(global->stream);
		g_debug("\\ %s (0x%02X, 0x%02X, 0x%08x)", s2, u1, u2, x1);

		x1 = g3d_stream_read_int32_le(global->stream);
		if(x1 == 0x00fffeff)
			g3d_stream_seek(global->stream, 21, G_SEEK_CUR);
		else if(x1 == 0xfffffeff)
			g3d_stream_seek(global->stream, 20, G_SEEK_CUR);
		else {
			g_warning("CLayer: unexpected value 0x%08x @ 0x%08x", x1,
				(guint32)g3d_stream_tell(global->stream) - 4);
			return FALSE;
		}
		w1 = g3d_stream_read_int16_le(global->stream);
		global->layers = g_slist_append(global->layers,
			GINT_TO_POINTER(w1 & 0x7FFF));
		g_debug("CLayer ID: 0x%2x", w1 & 0x7FFF);

		if(s1)
			g_free(s1);
		if(s2)
			g_free(s2);

	} while(w1 & 0x8000);

	x1 = g3d_stream_read_int32_le(global->stream);
	g_debug("CLayer: last 0x%08x", x1);

	return TRUE;
}

/*****************************************************************************/

gboolean skp_cb_material(SkpGlobalData *global, SkpLocalData *local)
{
	gchar *name, *tmp;
	guint8 u1, u2, u3;
	guint32 x1, type, size;
	G3DFloat r, g, b, a;
	G3DMaterial *material;

	g3d_stream_read_int16_le(global->stream);

	name = skp_read_wchar(global->stream);
	while(name) {
		tmp = NULL;
		size = 0;

		material = g3d_material_new();
		material->name = g_strdup(name);
		global->model->materials = g_slist_append(global->model->materials,
			material);

		type = g3d_stream_read_int16_le(global->stream);
		switch(type) {
			case 0x0000: /* simple color */
				r = g3d_stream_read_int8(global->stream);
				g = g3d_stream_read_int8(global->stream);
				b = g3d_stream_read_int8(global->stream);
				a = g3d_stream_read_int8(global->stream);

				material->r = r / 255.0;
				material->g = g / 255.0;
				material->b = b / 255.0;
				material->a = a / 255.0;
#if DEBUG > 1
				g_debug(
					"\tmaterial: %-30s 0x%04x, "
					"color (%.1f, %.1f, %.1f)",
					name, type, material->r, material->g, material->b);
#endif
				break;

			case 0x0001: /* texture */
				u1 = g3d_stream_read_int8(global->stream);
				u2 = g3d_stream_read_int8(global->stream);
				u3 = g3d_stream_read_int8(global->stream);
				if(u3 == 0x80) {
					/* number of textures? */
					x1 = g3d_stream_read_int32_le(global->stream);
					if(x1 > 0) {
						size = g3d_stream_read_int32_le(global->stream);
						g3d_stream_seek(global->stream, size, G_SEEK_CUR);
					}
				} else {
					x1 = 0x0004;
				}
				switch(x1) {
					case 0x0000:
						g3d_stream_seek(global->stream, 12, G_SEEK_CUR);
						break;
					case 0x0001:
						g3d_stream_seek(global->stream, 20, G_SEEK_CUR);
						break;
					case 0x0002:
						g3d_stream_seek(global->stream, 16, G_SEEK_CUR);
						break;
					case 0x0004:
						g3d_stream_seek(global->stream, 16, G_SEEK_CUR);
						break;
					default:
						g3d_stream_seek(global->stream, 16, G_SEEK_CUR);
						g_debug("SKP: mat0001: x1=%x: "
							"%02X%02X %02X%02X %02X%02X %02X%02X", x1,
							g3d_stream_read_int8(global->stream),
							g3d_stream_read_int8(global->stream),
							g3d_stream_read_int8(global->stream),
							g3d_stream_read_int8(global->stream),
							g3d_stream_read_int8(global->stream),
							g3d_stream_read_int8(global->stream),
							g3d_stream_read_int8(global->stream),
							g3d_stream_read_int8(global->stream));
						break;
				}
				tmp = skp_read_wchar(global->stream);
				g3d_stream_seek(global->stream, 8, G_SEEK_CUR);
#if DEBUG > 1
				g_debug(
					"\tmaterial: %-30s 0x%04x, %02X, %02X, %02X, 0x%08x\n"
					"\t\ttexture (%d bytes, 0x%08x):\n\t\t%s",
					name, type, u1, u2, u3, x1, size,
					(guint32)g3d_stream_tell(global->stream), tmp);
#endif

				break;

			case 0x0101: /* texture */
				/* number of textures? */
				x1 = g3d_stream_read_int32_le(global->stream);
				size = g3d_stream_read_int32_le(global->stream);
				g3d_stream_seek(global->stream, size, G_SEEK_CUR);
				g3d_stream_seek(global->stream, 16, G_SEEK_CUR);
				tmp = skp_read_wchar(global->stream);
				g3d_stream_seek(global->stream, 8, G_SEEK_CUR);
#if DEBUG > 1
				g_debug(
					"\tmaterial: %-30s 0x%04x, 0x%08x\n"
					"\t\ttexture (%i bytes):\n\t\t%s",
					name, type, x1, size, tmp);
#endif
				break;

			default:
				g_debug("SKP: material: unknown type 0x%04X @ 0x%08x",
					type, (guint32)g3d_stream_tell(global->stream));
				return FALSE;
		}

		x1 = g3d_stream_read_int32_be(global->stream);
		switch(x1 & 0x00FFL) {
			case 0x00:
				g3d_stream_seek(global->stream, 21, G_SEEK_CUR);
				break;
			case 0xFF:
				g3d_stream_seek(global->stream, 22, G_SEEK_CUR);
				break;
			default:
				g_debug("x1: 0x%08x", x1);
				break;
		}

		/* clean up */
		if(tmp)
			g_free(tmp);
		g_free(name);

		/* next one */
		name = skp_read_wchar(global->stream);
	}
	return TRUE;
}

/*****************************************************************************/

gboolean skp_cb_vertex(SkpGlobalData *global, SkpLocalData *local)
{
	guint16 w1;
	gdouble d1, d2, d3;
	gboolean handled;
	GSList *item;
	SkpComponent *comp;

	return FALSE;

	w1 = g3d_stream_read_int16_le(global->stream);
	g_return_val_if_fail(w1 == 0x0000, FALSE);

	skp_read_dbl3(global->stream, &d1, &d2, &d3);
	g_debug("\tvertex: %.4f, %.4f, %.4f", d1, d2, d3);

	do {
		w1 = g3d_stream_read_int16_le(global->stream);
		handled = FALSE;

		g_debug("opcode: 0x%04x", w1);
		if(w1 == 0xFFFF) {
			g3d_stream_seek(global->stream, -2, G_SEEK_CUR);
			return TRUE;
		}
		if(w1 & 0x8000) {
			g3d_stream_read_int16_le(global->stream);
			w1 &= 0x7FFF;
		}
		if(w1 == 0x0000) {
			handled = TRUE;
		}

		for(item = global->components;
			(handled == FALSE) && (item != NULL);
			item = item->next) {

			comp = item->data;

			switch(w1 - comp->id0) {
				case 1:
					skp_read_10b(global->stream);
					handled = TRUE;
					break;
				case 3:
					skp_read_dbl3(global->stream, &d1, &d2, &d3);
					g_debug("\tvertex: %.4f, %.4f, %.4f", d1, d2, d3);
					handled = TRUE;
					break;
				case 4:
				case 5:
				case 7:
				case 9:
				case 11:
				case 13:
					g_debug("\tempty");
					handled = TRUE;
					break;
				default:
					break;
			}
		} /* loop through known layers */

		if(handled == FALSE) {
			g_debug("vertex: unknown opcode %#04x (layer0: 0x%04x, 0x%04x)",
				w1,
				global->components ?
					((SkpComponent *)(global->components->data))->id0 : -1,
				global->components ?
					((SkpComponent *)(global->components->data))->id1 : -1);
		}
	} while(TRUE);

	return FALSE;
}
