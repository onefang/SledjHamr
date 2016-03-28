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
#ifndef _IMP_3DM_OBJECT_TYPES_H
#define _IMP_3DM_OBJECT_TYPES_H

#include "imp_3dm_callbacks.h"

static TdmObjectTypeInfo tdm_object_types[] = {
	{ 0x00000000, "unknown object",              NULL },
	{ 0x00000001, "point",                       NULL },
	{ 0x00000002, "point set",                   NULL },
	{ 0x00000004, "curve",                       NULL },
	{ 0x00000008, "surface",                     NULL },
	{ 0x00000010, "brep",                        NULL },
	{ 0x00000020, "mesh",                        tdm_cb_o_0x00000020 },
	{ 0x00000040, "layer",                       NULL },
	{ 0x00000080, "material",                    NULL },
	{ 0x00000100, "light",                       NULL },
	{ 0x00000200, "annotation",                  NULL },
	{ 0x00000400, "user data",                   NULL },
	{ 0x00000800, "instance definition",         NULL },
	{ 0x00001000, "instance reference",          NULL },
	{ 0x00002000, "text dot",                    NULL },
	{ 0x00004000, "grip",                        NULL },
	{ 0x00008000, "detail",                      NULL },
	{ 0x00010000, "hatch",                       NULL },
	{ 0x00020000, "morph control",               NULL },
	{ 0x00080000, "loop",                        NULL },
	{ 0x00200000, "poly surface",                NULL },
	{ 0x00400000, "edge filter",                 NULL },
	{ 0x00800000, "poly edge filter",            NULL },
	{ 0x01000000, "mesh vertex",                 NULL },
	{ 0x02000000, "mesh edge",                   NULL },
	{ 0x04000000, "mesh face",                   NULL },
	{ 0x08000000, "cage",                        NULL },
	{ 0x10000000, "phantom",                     NULL },
	{ 0x20000000, "clip plane",                  NULL },

	{ 0xFFFFFFFF, "any object",                  NULL }
};

#endif /* _IMP_3DM_OBJECT_TYPES_H */
