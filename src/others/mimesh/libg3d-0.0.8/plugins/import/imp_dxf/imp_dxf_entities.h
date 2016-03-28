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
#ifndef _IMP_DXF_ENTITIES_H
#define _IMP_DXF_ENTITIES_H

#include "imp_dxf_types.h"
#include "imp_dxf_def.h"
#include "imp_dxf_callbacks.h"

static DxfEntityInfo dxf_entities[] = {
	{ "3DFACE",    "3D face",                   DXF_E_3DFACE, dxf_e_3DFACE },
	{ "POLYLINE",  "Polyline",                DXF_E_POLYLINE, dxf_e_POLYLINE },
	{ "VERTEX",    "Vertex",                    DXF_E_VERTEX, dxf_e_VERTEX },

	{ "BLOCK",     "block",                     DXF_E_BLOCK,  dxf_e_BLOCK },
	{ "ENDBLK",    "end of block",              DXF_E_OTHER,  dxf_e_ENDBLK },
	{ "INSERT",    "insert block reference",    DXF_E_INSERT, dxf_e_INSERT },

	{ "3DLINE",    "3D line",                   DXF_E_OTHER,    NULL },
	{ "ACDBDICTIONARYWDFLT", "unknown",         DXF_E_OTHER,    NULL },
	{ "ACDBPLACEHOLDER", "unknown",             DXF_E_OTHER,    NULL },
	{ "APPID",     "application ID",            DXF_E_OTHER,    NULL },
	{ "ARC",       "arc",                       DXF_E_OTHER,    NULL },
	{ "ATTDEF",    "attribute definition",      DXF_E_OTHER,    NULL },
	{ "ATTRIB",    "attribute",                 DXF_E_OTHER,    NULL },
	{ "BLOCK_RECORD", "block record",           DXF_E_OTHER,    NULL },
	{ "CIRCLE",    "circle",                    DXF_E_OTHER,    NULL },
	{ "CLASS",     "class",                     DXF_E_OTHER,    NULL },
	{ "DICTIONARY", "dictionary",               DXF_E_OTHER,    NULL },
	{ "DIMSTYLE",  "unknown",                   DXF_E_OTHER,    NULL },
	{ "ENDTAB",    "end of table",              DXF_E_OTHER,    NULL },
	{ "LAYER",     "layer",                     DXF_E_OTHER,    NULL },
	{ "LAYOUT",    "layout",                    DXF_E_OTHER,    NULL },
	{ "LINE",      "line",                      DXF_E_OTHER,    NULL },
	{ "LTYPE",     "line type",                 DXF_E_OTHER,    NULL },
	{ "LWPOLYLINE", "unknown",                  DXF_E_OTHER,    NULL },
	{ "MLINESTYLE", "unknown",                  DXF_E_OTHER,    NULL },
	{ "POINT",     "point",                     DXF_E_OTHER,    NULL },
	{ "SEQEND",    "end of sequence",           DXF_E_OTHER,    NULL },
	{ "SPLINE",    "spline curve",              DXF_E_OTHER,    NULL },
	{ "STYLE",     "style",                     DXF_E_OTHER,    NULL },
	{ "TABLE",     "table",                     DXF_E_OTHER,    NULL },
	{ "UCS",       "UCS",                       DXF_E_OTHER,    NULL },
	{ "VIEW",      "view",                      DXF_E_OTHER,    NULL },
	{ "VIEWPORT",  "viewport",                  DXF_E_OTHER,    NULL },
	{ "VPORT",     "viewport",                  DXF_E_OTHER,    NULL },

	{ NULL, NULL, 0, NULL }
};

#endif
