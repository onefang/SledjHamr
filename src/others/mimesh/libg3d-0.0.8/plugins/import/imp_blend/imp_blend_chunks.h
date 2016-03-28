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
#ifndef _IMP_BLEND_CHUNKS_H
#define _IMP_BLEND_CHUNKS_H

#include "imp_blend_def.h"
#include "imp_blend_types.h"
#include "imp_blend_callbacks.h"

typedef gboolean (* BlendCallback)(BlendGlobal *, BlendLocal *);

typedef struct {
	guint32 code;
	guint32 oid;
	const gchar *description;
	BlendCallback callback;
} BlendChunkInfo;

#define ID_DATA MKID('D','A','T','A')

static const BlendChunkInfo blend_chunks[] = {
	{ ID_DATA, MKID2('M','E'),  "mesh data",                blend_cb_DATA_ME },
	{ ID_DATA,               0, "struct data",              NULL },

	{ MKID2('C','A'),        0, "camera",                   NULL },
	{ MKID2('I','M'),        0, "image",                    NULL },
	{ MKID2('L','A'),        0, "lamp",                     NULL },
	{ MKID2('M','A'),        0, "material",                 blend_cb_MA },
	{ MKID2('M','E'),        0, "mesh",                     blend_cb_ME },
	{ MKID2('O','B'),        0, "object",                   blend_cb_OB },
	{ MKID2('S','C'),        0, "scene",                    NULL },
	{ MKID2('S','R'),        0, "screen",                   NULL },
	{ MKID2('T','E'),        0, "texture",                  NULL },
	{ MKID2('T','X'),        0, "text",                     NULL },
	{ MKID2('W','O'),        0, "world",                    NULL },

	{ MKID('D','N','A','1'), 0, "SDNA data",                blend_cb_DNA1 },
	{ MKID('G','L','O','B'), 0, "global data",              NULL },
	{ MKID('R','E','N','D'), 0, "render data",              NULL },

	{ 0, 0, NULL, NULL }
};

#endif /* _IMP_BLEND_CHUNKS_H */
