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
#ifndef _IMP_3DMF_CHUNKS_H
#define _IMP_3DMF_CHUNKS_H

#include <g3d/iff.h>
#include "imp_3dmf_callbacks.h"

typedef struct {
	guint32 id;
	const gchar *description;
	X3dmfCallback callback;
} X3dmfChunkDesc;

#define ID G3D_IFF_MKID

static X3dmfChunkDesc x3dmf_chunks[] = {
	{ ID('a', 'm', 'b', 'n'), "ambient light",          NULL },
	{ ID('a', 't', 'a', 'r'), "(registered unknown)",   NULL },
	{ ID('a', 't', 't', 'r'), "attribute set",          NULL },
	{ ID('b', 'g', 'n', 'g'), "begin group",            NULL },
	{ ID('c', 'a', 'm', 'b'), "ambient coefficient",    NULL },
	{ ID('c', 'm', 'p', 'l'), "camera placement",       NULL },
	{ ID('c', 'm', 'r', 'g'), "camera range",           NULL },
	{ ID('c', 'm', 'v', 'p'), "camera viewport",        NULL },
	{ ID('c', 'n', 't', 'r'), "container",              NULL },
	{ ID('c', 's', 'g', 'e'), "(registered unknown)",   NULL },
	{ ID('c', 's', 'p', 'c'), "specular control",       NULL },
	{ ID('c', 't', 'w', 'n'), "interactive renderer",   NULL },
	{ ID('d', 'b', 'b', 'p'), "(registered unknown)",   NULL },
	{ ID('d', 'r', 'c', 't'), "directional light",      NULL },
	{ ID('e', 'n', 'd', 'g'), "end group",              NULL },
	{ ID('h', 'l', 's', 't'), "highlight state",        NULL },
	{ ID('i', 'm', 'c', 'c'), "image clear color",      NULL },
	{ ID('i', 'm', 'd', 'm'), "image dimensions",       NULL },
	{ ID('k', 'd', 'i', 'f'), "diffuse color",          NULL },
	{ ID('k', 's', 'p', 'c'), "specular color",         NULL },
	{ ID('k', 'x', 'p', 'r'), "transparency color",     NULL },
	{ ID('l', 'g', 'h', 't'), "light data",             NULL },
	{ ID('m', 'e', 's', 'h'), "mesh",                   NULL },
	{ ID('m', 't', 'r', 'x'), "matrix",                 NULL },
	{ ID('n', 'r', 'm', 'l'), "normal",                 NULL },
	{ ID('p', 'n', 't', 'l'), "point light",            NULL },
	{ ID('r', 'f', 'r', 'n'), "reference",              NULL },
	{ ID('r', 'v', 'h', 'n'), "(registered unknown)",   NULL },
	{ ID('r', 'v', 't', 'x'), "(registered unknown)",   NULL },
	{ ID('s', 'e', 't', ' '), "set",                    NULL },
	{ ID('s', 't', 'r', 'c'), "C string",               NULL },
	{ ID('t', 'm', 's', 'h'), "triangle mesh",          NULL },
	{ ID('t', 'o', 'c', ' '), "tabe of contents",       NULL },
	{ ID('t', 'r', 'n', 's'), "translation",            NULL },
	{ ID('t', 'y', 'p', 'e'), "type definition",        NULL },
	{ ID('v', 'a', 'n', 'a'), "view angle aspect cam",  NULL },
	{ ID('v', 'a', 's', 'l'), "vertex attr set list",   NULL },
	{ ID('v', 'n', 'i', 'd'), "(registered unknown)",   NULL },
	{ ID('v', 'w', 'h', 'n'), "view hints",             NULL },
	{ ID('v', 'w', 'p', 'l'), "view plane camera",      NULL },

	{ ID(0xFF, 0xFF, 0xFF, 0xE5), "0xFFFFFFE5",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xE7), "0xFFFFFFE7",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xE9), "0xFFFFFFE9",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xEA), "0xFFFFFFEA",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xEB), "0xFFFFFFEB",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xEC), "0xFFFFFFEC",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xEF), "0xFFFFFFEF (EoC?)",  NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xF1), "0xFFFFFFF1",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xF4), "0xFFFFFFF4",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xF6), "0xFFFFFFF6",         NULL },
	{ ID(0xFF, 0xFF, 0xFF, 0xFD), "0xFFFFFFFD",         NULL },

	{ 0, NULL, NULL }
};

#undef ID

#endif /* _IMP_3DMF_CHUNKS_H */
