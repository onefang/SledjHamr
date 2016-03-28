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

#ifndef _IMP_SKP_CHUNKS_H
#define _IMP_SKP_CHUNKS_H

#include "imp_skp_callbacks.h"

typedef struct {
	gchar *id;
	guint32 min_ver;
	guint32 max_ver;
	SkpCallback callback;
} SkpChunkDesc;

static SkpChunkDesc skp_chunks[] = {
	{ "CArcCurve",              1,  1,  skp_cb_arc_curve },
	{ "CAttributeContainer",    0,  0,  skp_cb_attribute_container },
	{ "CAttributeNamed",        0,  0,  skp_cb_attribute_named },
	{ "CCamera",                4,  5,  NULL },
	{ "CComponentDefinition",   10, 10, skp_cb_component_definition },
	{ "CComponentInstance",     4,  4,  NULL },
	{ "CCurve",                 4,  4,  NULL },
	{ "CDib",                   2,  3,  NULL },
	{ "CEdge",                  2,  2,  NULL },
	{ "CEdgeUse",               1,  1,  skp_cb_edge_use },
	{ "CFace",                  3,  3,  NULL },
	{ "CFaceTextureCoords",     4,  4,  skp_cb_face_texture_coords },
	{ "CGroup",                 1,  1,  NULL },
	{ "CLayer",                 2,  2,  skp_cb_layer },
	{ "CLoop",                  1,  1,  NULL },
	{ "CMaterial",              12, 12, skp_cb_material },
	{ "CRelationship",          0,  0,  NULL },
	{ "CSkFont",                0,  1,  NULL },
	{ "CSkpStyle",              1,  1,  NULL },
	{ "CThumbnail",             1,  1,  NULL },
	{ "CVertex",                0,  0,  skp_cb_vertex },
	{ "CViewPage",              9,  11, NULL },

	{ NULL, 0, 0, NULL }
};

#endif /* _IMP_SKP_CHUNKS_H */
