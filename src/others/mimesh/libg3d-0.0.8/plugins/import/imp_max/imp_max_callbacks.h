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
#ifndef _IMP_MAX_CALLBACKS_H
#define _IMP_MAX_CALLBACKS_H

#include <stdio.h>
#include <glib.h>
#include <g3d/types.h>

typedef struct {
	G3DContext *context;
	G3DModel *model;
	G3DStream *stream;
	const gchar *subfile;

	G3DObject *object;
	guint32 vertex_offset;
} MaxGlobalData;

typedef struct {
	guint16 id;
	gint32 parentid;
	guint16 level;
	gint32 nb;
	gpointer object;
} MaxLocalData;

typedef gboolean (* MaxCallback)(MaxGlobalData *global, MaxLocalData *local);

/* callbacks */

gboolean max_cb_debug_int32(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_debug_string(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_debug_wchars(MaxGlobalData *global, MaxLocalData *local);

gboolean max_cb_IDROOT_IDGEOM(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_IDMATG_0x4000(MaxGlobalData *global, MaxLocalData *local);

gboolean max_cb_IDGEOM_0x08FE(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_IDGEOM_0x0962(MaxGlobalData *global, MaxLocalData *local);

gboolean max_cb_IDFILE_0x1201(MaxGlobalData *global, MaxLocalData *local);

gboolean max_cb_0x0001_0x0005(MaxGlobalData *global, MaxLocalData *local);

gboolean max_cb_0x0118_0x0110(MaxGlobalData *global, MaxLocalData *local);

gboolean max_cb_0x08FE_0x0100(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x010A(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x011A(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x0128(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x012B(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x0912(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x0914(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x0916(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x0918(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x2394(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x08FE_0x2396(MaxGlobalData *global, MaxLocalData *local);

gboolean max_cb_0x4000_0x4001(MaxGlobalData *global, MaxLocalData *local);
gboolean max_cb_0x4000_0x4030(MaxGlobalData *global, MaxLocalData *local);

#endif /* _IMP_MAX_CALLBACKS_H */
