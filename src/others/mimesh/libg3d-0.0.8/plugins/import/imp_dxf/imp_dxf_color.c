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
#include <string.h>

#include <g3d/types.h>
#include <g3d/material.h>

static G3DMaterial *get_material(G3DModel *model, const gchar *cname)
{
	G3DMaterial *material;
	GSList *item;

	for(item = model->materials; item != NULL; item = item->next) {
		material = item->data;
		if(strcmp(material->name, cname) == 0)
			return material;
	}
	return NULL;
}

#define DXF_COL_SET(rx, gx, bx) \
	material->r = ((G3DFloat)(rx) / 255.0); \
	material->g = ((G3DFloat)(gx) / 255.0); \
	material->b = ((G3DFloat)(bx) / 255.0);

/*
 * based on GPL code from
 * http://wiki.inkscape.org/wiki/index.php/SOC_Accepted_Proposals
 */
static gboolean set_aci(G3DMaterial *material, gint32 aci)
{
	G3DFloat r, g, b, h, s, l, m;
	gint32 mod10;

	if((aci < 10) || (aci > 249)) {
		switch(aci) {
			case 0:   DXF_COL_SET(0x00, 0x00, 0x00); break;
			case 1:   DXF_COL_SET(0xFF, 0x00, 0x00); break;
			case 2:   DXF_COL_SET(0xFF, 0xFF, 0x00); break;
			case 3:   DXF_COL_SET(0x00, 0xFF, 0x00); break;
			case 4:   DXF_COL_SET(0x00, 0xFF, 0xFF); break;
			case 5:   DXF_COL_SET(0x00, 0x00, 0xFF); break;
			case 6:   DXF_COL_SET(0xFF, 0x00, 0xFF); break;
			case 7:   DXF_COL_SET(0xFF, 0xFF, 0xFF); break;
			case 8:   DXF_COL_SET(0x80, 0x80, 0x80); break;
			case 9:   DXF_COL_SET(0xC0, 0xC0, 0xC0); break;

			case 250: DXF_COL_SET(0x33, 0x33, 0x33); break;
			case 251: DXF_COL_SET(0x5B, 0x5B, 0x5B); break;
			case 252: DXF_COL_SET(0x84, 0x84, 0x84); break;
			case 253: DXF_COL_SET(0xAD, 0xAD, 0xAD); break;
			case 254: DXF_COL_SET(0xD6, 0xD6, 0xD6); break;
			case 255: DXF_COL_SET(0xFF, 0xFF, 0xFF); break;
			default: break;
		}
	} else {
		mod10 = aci % 10;
		h = 1.5 * (aci - mod10 - 10);
		s = ((aci % 2) ? 0.5 : 1.0);
		if(mod10 < 2) l = 1.0;
		else if(mod10 < 4) l = 0.8;
		else if(mod10 < 6) l = 0.6;
		else if(mod10 < 8) l = 0.5;
		else l = 0.3;

		if(h <= 120) {
			r = (120 - h) / 60;
			g = h / 60;
			b = 0;
		} else if(h <= 240) {
			r = 0;
			g = (240 - h) / 60;
			b = (h - 120) / 60;
		} else if(h <= 360) {
			r = (h - 240) / 60;
			g = 0;
			b = (360 - h) / 60;
		}
		r = MIN(r, 1.0);
		g = MIN(g, 1.0);
		b = MIN(b, 1.0);
		m = MAX(r, MAX(g, b));
		material->r = (m - s * (m - r)) * l;
		material->g = (m - s * (m - g)) * l;
		material->b = (m - s * (m - b)) * l;
	}
	return TRUE;
}

G3DMaterial *dxf_color_get_material(G3DModel *model, gint32 aci)
{
	gchar *cname;
	G3DMaterial *material;

	cname = g_strdup_printf("color #%d", ABS(aci));
	material = get_material(model, cname);
	if(material != NULL) {
		g_free(cname);
		return material;
	}
	material = g3d_material_new();
	material->name = cname;
	model->materials = g_slist_append(model->materials, material);
	set_aci(material, ABS(aci));
#if DEBUG > 0
	g_debug("| color: %.2f %.2f %.2f", material->r, material->g, material->b);
#endif

	return material;
}

