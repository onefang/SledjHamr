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

#ifndef _IMP_SKP_CALLBACKS_H
#define _IMP_SKP_CALLBACKS_H

#include <g3d/types.h>

typedef struct {
	G3DContext *context;
	G3DModel *model;
	G3DStream *stream;
	GSList *layers;
	GSList *components;
} SkpGlobalData;

typedef struct {
	gchar *id;
	guint32 version;
	gpointer object;
} SkpLocalData;

typedef gboolean (* SkpCallback)(SkpGlobalData *global, SkpLocalData *local);

/* callbacks */

gboolean skp_cb_arc_curve(SkpGlobalData *global, SkpLocalData *local);
gboolean skp_cb_attribute_container(SkpGlobalData *global,
	SkpLocalData *local);
gboolean skp_cb_attribute_named(SkpGlobalData *global, SkpLocalData *local);
gboolean skp_cb_component_definition(SkpGlobalData *global,
	SkpLocalData *local);
gboolean skp_cb_edge_use(SkpGlobalData *global, SkpLocalData *local);
gboolean skp_cb_face_texture_coords(SkpGlobalData *global,
	SkpLocalData *local);
gboolean skp_cb_layer(SkpGlobalData *global, SkpLocalData *local);
gboolean skp_cb_material(SkpGlobalData *global, SkpLocalData *local);
gboolean skp_cb_vertex(SkpGlobalData *global, SkpLocalData *local);

#endif /* _IMP_SKP_CALLBACKS_H */
