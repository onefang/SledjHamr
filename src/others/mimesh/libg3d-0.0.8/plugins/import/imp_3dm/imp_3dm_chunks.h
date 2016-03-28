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
#ifndef _IMP_3DM_CHUNKS_H
#define _IMP_3DM_CHUNKS_H

#include "imp_3dm_callbacks.h"

static TdmChunkInfo tdm_chunks[] = {
	{ 0x00000001, FALSE, FALSE, "comment block",             NULL },
	{ 0x00007FFE, FALSE, TRUE,  "end of file",               NULL },
	{ 0x00007FFF, FALSE, TRUE,  "end of file",               NULL },

	{ 0x00027FF9, FALSE, FALSE, "ON class userdata header",  NULL },
	{ 0x00027FFA, TRUE,  FALSE, "OpenNURBS class",           NULL },
	{ 0x00027FFB, FALSE, FALSE, "OpenNURBS class UUID",      NULL },
	{ 0x00027FFC, FALSE, FALSE, "OpenNURBS class data",    tdm_cb_0x00027ffc },
	{ 0x00027FFD, FALSE, FALSE, "OpenNURBS class userdata",  NULL },
	{ 0x00027FFF, FALSE, TRUE,  "OpenNURBS class end",       NULL },

	{ 0x02000061, FALSE, FALSE, "light record attributes",   NULL },
	{ 0x0200006F, FALSE, TRUE,  "light record end",          NULL },

	{ 0x02000071, FALSE, FALSE, "object record type",      tdm_cb_0x02000071 },
	{ 0x02000072, FALSE, FALSE, "object record attributes",  NULL },
	{ 0x0200007F, FALSE, TRUE,  "object record end",       tdm_cb_0x0200007f },

	{ 0x10000010, TRUE,  FALSE, "material table",            NULL },
	{ 0x10000011, TRUE,  FALSE, "layer table",               NULL },
	{ 0x10000012, TRUE,  FALSE, "light table",               NULL },
	{ 0x10000013, TRUE,  FALSE, "object table",              NULL },
	{ 0x10000014, TRUE,  FALSE, "properties table",          NULL },
	{ 0x10000015, TRUE,  FALSE, "settings table",            NULL },
	{ 0x10000016, TRUE,  FALSE, "bitmap table",              NULL },
	{ 0x10000017, TRUE,  FALSE, "user table",                NULL },
	{ 0x10000018, TRUE,  FALSE, "group table",               NULL },
	{ 0x10000019, TRUE,  FALSE, "font table",                NULL },
	{ 0x10000020, TRUE,  FALSE, "dimension style table",     NULL },
	{ 0x10000021, TRUE,  FALSE, "instance definition table", NULL },
	{ 0x10000022, TRUE,  FALSE, "hatch pattern table",       NULL },
	{ 0x10000023, TRUE,  FALSE, "linetype table",            NULL },
	{ 0x10000024, TRUE,  FALSE, "obsolete layer set table",  NULL },
	{ 0x10000025, TRUE,  FALSE, "texture mapping table",     NULL },
	{ 0x10000026, TRUE,  FALSE, "history record table",      NULL },

	{ 0x20000021, FALSE, FALSE, "revision history",          NULL },
	{ 0x20000022, FALSE, FALSE, "notes",                     NULL },
	{ 0x20000023, FALSE, FALSE, "preview image",             NULL },
	{ 0x20000024, FALSE, FALSE, "application",               NULL },
	{ 0x20000025, FALSE, FALSE, "compressed preview image",  NULL },
	{ 0x20000026, FALSE, FALSE, "OpenNURBS version",         NULL },

	{ 0x20000031, FALSE, FALSE, "units and tols",            NULL },
	{ 0x20000032, FALSE, FALSE, "render mesh",               NULL },
	{ 0x20000033, FALSE, FALSE, "analysis mesh",             NULL },
	{ 0x20000034, FALSE, FALSE, "annotation",                NULL },
	{ 0x20000035, FALSE, FALSE, "named cplane list",         NULL },
	{ 0x20000036, FALSE, FALSE, "named view list",           NULL },
	{ 0x20000037, FALSE, FALSE, "view list",                 NULL },
	{ 0x20000038, FALSE, FALSE, "current layer index",       NULL },
	{ 0x20000039, FALSE, FALSE, "current material index",    NULL },
	{ 0x2000003A, FALSE, FALSE, "current color",             NULL },
	{ 0x2000003B, FALSE, FALSE, "view record",               NULL },
	{ 0x2000003C, FALSE, FALSE, "current wire density",      NULL },
	{ 0x2000003D, FALSE, FALSE, "render",                    NULL },
	{ 0x2000003E, FALSE, FALSE, "NEVER USE THIS",            NULL },
	{ 0x2000003F, FALSE, FALSE, "grid defaults",             NULL },
	{ 0x20000040, FALSE, FALSE, "material record",           NULL },

	{ 0x20000050, FALSE, FALSE, "layer record",              NULL },

	{ 0x20000060, TRUE,  FALSE, "light record",              NULL },

	{ 0x20000070, TRUE,  FALSE, "object record",           tdm_cb_0x20000070 },

	{ 0x20000073, FALSE, FALSE, "group record",              NULL },
	{ 0x20000074, FALSE, FALSE, "font record",               NULL },
	{ 0x20000075, FALSE, FALSE, "dimstyle record",           NULL },
	{ 0x20000076, FALSE, FALSE, "instance definition record",NULL },
	{ 0x20000077, FALSE, FALSE, "hatch pattern record",      NULL },
	{ 0x20000078, FALSE, FALSE, "linetype record",           NULL },

	{ 0x20000080, FALSE, FALSE, "table id",                  NULL },
	{ 0x20000081, FALSE, FALSE, "user record",               NULL },

	{ 0x20000131, FALSE, FALSE, "model url",                 NULL },
	{ 0x20000132, FALSE, FALSE, "current font index",        NULL },
	{ 0x20000133, FALSE, FALSE, "current dimstyle index",    NULL },
	{ 0x20000134, FALSE, FALSE, "attributes",                NULL },
	{ 0x20000135, FALSE, FALSE, "plugin list",               NULL },
	{ 0x2000013B, FALSE, FALSE, "cplane",                    NULL },

	{ 0x7FFF7FFF, FALSE, TRUE,  "end of table",              NULL },
};

#endif /* _IMP_3DM_CHUNKS_H */
