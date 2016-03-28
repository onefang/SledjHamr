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
#ifndef _IMP_MAX_CHUNKS_H
#define _IMP_MAX_CHUNKS_H

#include <glib.h>

#include "imp_max_callbacks.h"

#define IDNONE -1
#define IDSOME -2
#define IDROOT -3
#define IDGEOM -4
#define IDMATG -5
#define IDFILE -6

#define ID_IS_ROOT(pid) \
	(((pid) == 0x2001) || ((pid) == 0x2003) || ((pid) == 0x2004) || \
	 ((pid) == 0x2006) || ((pid) == 0x2008) || ((pid) == 0x2009) || \
	 ((pid) == 0x200A) || ((pid) == 0x200B))
#define ID_IS_GEOM(pid) \
	((((pid) >= 0x0010) && ((pid) <= 0x0011)) || \
	 (((pid) >= 0x0013) && ((pid) <= 0x0014)) || \
	 (((pid) >= 0x0016) && ((pid) <= 0x001C)) || \
	 ((pid) == 0x0023) || ((pid) == 0x0026))
#define ID_IS_MATG(pid) \
	(((pid) == 0x0002) || \
	(((pid) >= 0x0004) && ((pid) <= 0x000F)))
#define ID_IS_FILE(pid) \
	(((pid) == 0x0003) || ((pid) == 0x0400))

typedef struct {
	gint32 id;
	gint32 parentid;
	char *desc;
	MaxCallback callback;
} MaxChunk;

/* container chunks */
static MaxChunk max_cnt_chunks[] = {
	{ 0x0118, 0x08FE, "face",                      NULL },

	{ 0x08FE, IDGEOM, "mesh",                      max_cb_IDGEOM_0x08FE },
	{ 0x0901, IDGEOM, "matrix or not",             NULL },
	{ 0x0906, IDGEOM, "matrix or not",             NULL },

	{ 0x092C, 0x08FE, "mesh bit map group",        NULL },
	{ 0x092D, 0x08FE, "mesh bit map group",        NULL },
	{ 0x092E, 0x08FE, "mesh bit map group",        NULL },

	{ 0x2001, IDNONE, "3ds MAX x.x root node",     NULL },
	{ 0x2003, IDNONE, "3ds MAX x.x root node",     NULL },
	{ 0x2004, IDNONE, "3ds MAX 3.x root node",     NULL },
	{ 0x2006, IDNONE, "3ds MAX 4.x root node",     NULL },
	{ 0x2008, IDNONE, "3ds MAX 5.x root node",     NULL },
	{ 0x2009, IDNONE, "3ds MAX 6.x root node",     NULL },
	{ 0x200A, IDNONE, "3ds MAX 7.x root node",     NULL },
	{ 0x200B, IDNONE, "3ds MAX 8.x root node",     NULL },
	{ 0x2038, IDNONE, "DLL directory entry",       NULL },
	{ 0x2040, IDNONE, "class directory entry",     NULL },
	{ 0x2100, IDNONE, "class data entry",          NULL },

	{ 0x4000, IDMATG, "material group",            max_cb_IDMATG_0x4000 },

	{ IDNONE, IDSOME, NULL, NULL }
};

/* data chunks */
static MaxChunk max_chunks[] = {
	/* IDGEOM */
	{ 0x08FF, IDGEOM, "mesh something",            max_cb_debug_int32 },
	{ 0x0900, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0902, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0903, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0904, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0960, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0962, IDGEOM, "object name",               max_cb_IDGEOM_0x0962 },
	{ 0x0963, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x096A, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x096B, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x096C, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0974, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0975, IDGEOM, "object flag",               NULL },
	{ 0x0978, IDGEOM, "object flag",               NULL },
	{ 0x099C, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x09BA, IDGEOM, "object flag",               NULL },
	{ 0x09CE, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0A28, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0A32, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x0ABE, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x2500, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x2501, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x2505, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x2532, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x3002, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x3003, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x3005, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x4020, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x4024, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x4025, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x4026, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x402C, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x402D, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x4030, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x4034, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x4038, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x403B, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x7230, IDGEOM, "object something",          max_cb_debug_int32 },
	{ 0x7231, IDGEOM, "object something",          max_cb_debug_int32 },
	/* IDMATG */
	{ 0x5300, IDMATG, "material something",        max_cb_debug_int32 },
	/* IDFILE */
	{ 0x1201, IDFILE, "file dimensions + unknown", max_cb_IDFILE_0x1201 },
	{ 0x1230, IDFILE, "file name",                 max_cb_debug_wchars },
	{ 0x1240, IDFILE, "file type",                 max_cb_debug_wchars },
	/* 0x0000 */
	{ 0x0005, 0x0000, "[DEBUG] big chunk",         NULL },
	{ 0x0009, 0x0000, "[DEBUG] 16",                max_cb_debug_int32 },
	{ 0x000A, 0x0000, "[DEBUG] 3n, 11",            NULL },
	{ 0x000B, 0x0000, "[DEBUG] 24",                max_cb_debug_int32 },
	{ 0x000C, 0x0000, "[DEBUG] 10",                NULL },
	{ 0x2035, 0x0000, "[DEBUG] n * 4",             max_cb_debug_int32 },
	/* 0x0001 */
	{ 0x0005, 0x0001, "properties",                max_cb_0x0001_0x0005 },
	{ 0x1000, 0x0003, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x2045, 0x0001, "[DEBUG] 4",                 max_cb_debug_int32 },
	/* 0x0002 */
	{ 0x2045, 0x0002, "[DEBUG] 4",                 max_cb_debug_int32 },
	/* 0x0003 */
	{ 0x2045, 0x0003, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x1030, 0x0003, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x1040, 0x0003, "[DEBUG] 4",                 max_cb_debug_int32 },
	/* 0x0004 */
	{ 0x2035, 0x0004, "[DEBUG] n * 4",             max_cb_debug_int32 },
	{ 0x2500, 0x0004, "[DEBUG] 8",                 max_cb_debug_int32 },
	{ 0x2501, 0x0004, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x2532, 0x0004, "[DEBUG] flag",              NULL },
	{ 0x2533, 0x0004, "[DEBUG] flag",              NULL },
	{ 0x2534, 0x0004, "[DEBUG] flag",              NULL },
	{ 0x3002, 0x0004, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x3003, 0x0004, "[DEBUG] 8",                 max_cb_debug_int32 },
	{ 0x3005, 0x0004, "[DEBUG] 4",                 max_cb_debug_int32 },
	/* 0x0010 */
	{ 0x0962, 0x0010, "name",                      max_cb_debug_wchars },
	/* 0x0013 */
	{ 0x1000, 0x0013, "text",                      max_cb_debug_string },
	/* 0x0019 */
	{ 0x0110, 0x0019, "name",                      max_cb_debug_wchars },
	{ 0x0120, 0x0019, "name",                      max_cb_debug_wchars },
	/* 0x0022 */
	{ 0x0110, 0x0022, "text",                      max_cb_debug_wchars },
	{ 0x0120, 0x0022, "type data",                 max_cb_debug_int32 },
	{ 0x0130, 0x0022, "type data",                 max_cb_debug_int32 },
	/* 0x0027 */
	{ 0x0110, 0x0027, "text",                      max_cb_debug_wchars },
	/* 0x002C */
	{ 0x0110, 0x002C, "text",                      max_cb_debug_wchars },
	/* 0x002E */
	{ 0x0110, 0x002E, "text",                      max_cb_debug_wchars },
	{ 0x0120, 0x002E, "type data",                 max_cb_debug_int32 },
	{ 0x0130, 0x002E, "type data",                 max_cb_debug_int32 },
	{ 0x0140, 0x002E, "type data",                 max_cb_debug_int32 },
	/* 0x0110 */
	{ 0x0120, 0x0110, "something",                 max_cb_debug_int32 },
	{ 0x0130, 0x0110, "something",                 max_cb_debug_int32 },
	/* 0x0118 */
	{ 0x0100, 0x0118, "face data",                 max_cb_debug_int32 },
	{ 0x0110, 0x0118, "face indices",              max_cb_0x0118_0x0110 },

	/* 0x08FE */
	{ 0x0100, 0x08FE, "vertices",                  max_cb_0x08FE_0x0100 },
	{ 0x0108, 0x08FE, "mesh data",                 max_cb_debug_int32 },
	{ 0x010A, 0x08FE, "lines",                     max_cb_0x08FE_0x010A },
	{ 0x0110, 0x08FE, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x0114, 0x08FE, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x011A, 0x08FE, "polygons",                  max_cb_0x08FE_0x011A },
	{ 0x0120, 0x08FE, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x0124, 0x08FE, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x0128, 0x08FE, "texture vertices",          max_cb_0x08FE_0x0128 },
	{ 0x012B, 0x08FE, "texture indices",           max_cb_0x08FE_0x012B },
	{ 0x0130, 0x08FE, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x0140, 0x08FE, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x0150, 0x08FE, "[DEBUG] 4",                 max_cb_debug_int32 },
	{ 0x0906, 0x08FE, "mesh something",            max_cb_debug_int32 },
	{ 0x0908, 0x08FE, "mesh something",            max_cb_debug_int32 },
	{ 0x0912, 0x08FE, "triangles",                 max_cb_0x08FE_0x0912 },
	{ 0x0914, 0x08FE, "vertices",                  max_cb_0x08FE_0x0914 },
	{ 0x0916, 0x08FE, "texture vertices",          max_cb_0x08FE_0x0916 },
	{ 0x0918, 0x08FE, "texture indices (tris)",    max_cb_0x08FE_0x0918 },
	{ 0x0924, 0x08FE, "mesh something",            max_cb_debug_int32 },
	{ 0x0928, 0x08FE, "mesh something",            max_cb_debug_int32 },
	{ 0x092A, 0x08FE, "mesh something",            max_cb_debug_int32 },
	{ 0x0959, 0x08FE, "mesh something",            max_cb_debug_int32 },
	{ 0x2394, 0x08FE, "vertices",                  max_cb_0x08FE_0x2394 },
	{ 0x2396, 0x08FE, "triangles",                 max_cb_0x08FE_0x2396 },
	{ 0x2398, 0x08FE, "mesh something",            max_cb_debug_int32 },
	/* 0x0901 */
	{ 0x300D, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x300E, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x300F, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x3010, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x3012, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x3013, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x3014, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x3015, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x3016, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x3017, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x3019, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x301A, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x301B, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x301C, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x301D, 0x0901, "x32",                       max_cb_debug_int32 },
	{ 0x301E, 0x0901, "x32",                       max_cb_debug_int32 },
	/* 0x0906 */
	{ 0x300D, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x300E, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x300F, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x3010, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x3012, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x3013, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x3014, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x3015, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x3016, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x3017, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x3019, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x301A, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x301B, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x301C, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x301D, 0x0906, "x32",                       max_cb_debug_int32 },
	{ 0x301E, 0x0906, "x32",                       max_cb_debug_int32 },
	/* 0x092C */
	{ 0x2700, 0x092C, "bit map",                   NULL },
	/* 0x092D */
	{ 0x2700, 0x092D, "bit map",                   NULL },
	/* 0x092E */
	{ 0x2700, 0x092E, "bit map",                   NULL },
	/* 0x0AF0 */
	{ 0x0001, 0x0AF0, "something",                 max_cb_debug_int32 },
	{ 0x0002, 0x0AF0, "something",                 max_cb_debug_int32 },
	{ 0x0003, 0x0AF0, "something",                 max_cb_debug_int32 },
	{ 0x0004, 0x0AF0, "something",                 max_cb_debug_int32 },
	{ 0x0005, 0x0AF0, "something",                 max_cb_debug_int32 },
	{ 0x0006, 0x0AF0, "something",                 max_cb_debug_int32 },
	{ 0x0007, 0x0AF0, "something",                 max_cb_debug_int32 },
	{ 0x0008, 0x0AF0, "something",                 max_cb_debug_int32 },
	/* 2500 */
	{ 0x2510, 0x2500, "something",                 max_cb_debug_int32 },
	{ 0x2511, 0x2500, "something",                 max_cb_debug_int32 },
	{ 0x2513, 0x2500, "something",                 max_cb_debug_int32 },
	/* 2512 */
	{ 0x0530, 0x2512, "something",                 max_cb_debug_int32 },
	{ 0x0540, 0x2512, "something",                 max_cb_debug_int32 },
	{ 0x0550, 0x2512, "something",                 max_cb_debug_int32 },
	{ 0x0560, 0x2512, "something",                 max_cb_debug_int32 },
	/* 0x39BF */
	{ 0x0100, 0x39BF, "text",                      max_cb_debug_wchars },
	/* 0x4000 */
	{ 0x4001, 0x4000, "material name",             max_cb_0x4000_0x4001 },
	{ 0x4003, 0x4000, "material something (4)",    max_cb_debug_int32 },
	{ 0x4020, 0x4000, "material something (4)",    max_cb_debug_int32 },
	{ 0x4030, 0x4000, "material color",            max_cb_0x4000_0x4030 },

	/* IDSOME */
	{ 0x2034, IDSOME, "init something",            max_cb_debug_int32 },
	{ 0x2035, IDSOME, "init something",            max_cb_debug_int32 },

	{ IDNONE, IDSOME, NULL, NULL }
};

#endif /* _IMP_MAX_CHUNKS_H */
