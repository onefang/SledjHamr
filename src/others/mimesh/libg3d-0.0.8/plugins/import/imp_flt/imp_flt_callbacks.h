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
#ifndef _IMP_FLT_CALLBACKS_H
#define _IMP_FLT_CALLBACKS_H

#include <stdio.h>
#include <glib.h>
#include <g3d/types.h>
#include <g3d/stream.h>

typedef struct {
	guint32 n_entries;
	goffset offset;

	goffset *offsets;                /* n * goffset */
	guint32 *flags;                  /* n * guint32 */
	G3DMaterial **vertex_materials;  /* n * G3DMaterial* */
	G3DFloat *vertex_data;             /* 3 x n * G3DFloat */
	G3DFloat *normal_data;             /* 3 x n * G3DFloat */
	G3DFloat *tex_vertex_data;         /* 2 x n * G3DFloat */
} FltVertexPalette;

typedef struct {
	guint32 size;
	G3DImage **textures;
	gint32 *offsets;
} FltTexturePalette;

#define FLT_FLAG_BROKEN_VERTEX_LIST (1 << 0)

#define FLT_FLAG_NO_COLOR           (1 << 2)
#define FLT_FLAG_PACKED_COLOR       (1 << 3)

typedef struct {
	G3DContext *context;
	G3DModel *model;
	G3DStream *stream;
	guint32 level;
	GQueue *oqueue;
	FltVertexPalette *vertex_palette;
	FltTexturePalette *texture_palette;
	guint32 fversion;                /* format version */
	guint32 flags;
} FltGlobalData;

typedef struct {
	guint32 opcode;
	G3DObject *g3dobj;
	gpointer level_object;
	gint32 nb;
} FltLocalData;

typedef gboolean (*FltCallbackFunc)(FltGlobalData *gd, FltLocalData *ld);

/* callback functions */
gboolean flt_cb_0001(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0002(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0004(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0005(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0010(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0011(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0032(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0033(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0064(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0067(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0068(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0069(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0070(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0071(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0072(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0084(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0085(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0086(FltGlobalData *gd, FltLocalData *ld);
gboolean flt_cb_0113(FltGlobalData *gd, FltLocalData *ld);

#endif /* _IMP_FLT_CALLBACKS_H */
