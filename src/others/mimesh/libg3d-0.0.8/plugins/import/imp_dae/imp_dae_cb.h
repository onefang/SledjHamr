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

#ifndef _IMP_DAE_CB_H
#define _IMP_DAE_CB_H

#include <g3d/types.h>
#include <libxml/tree.h>

#include "imp_dae_library.h"

typedef struct {
	G3DContext *context;
	G3DStream *stream;
	G3DModel *model;
	DaeLibrary *lib;
	xmlDocPtr xmldoc;
} DaeGlobalData;

typedef struct {
	xmlNodePtr parent;
	xmlNodePtr node;
	xmlNodePtr instance;
	guint32 level;
	gpointer user_data;
} DaeLocalData;

typedef gboolean (* DaeCallback)(DaeGlobalData *global, DaeLocalData *local);

typedef struct {
	const gchar *name;
	DaeCallback callback;
} DaeChunkDesc;

gboolean dae_cb_bind_material(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_effect(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_geometry(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_matrix(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_mesh(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_newparam(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_node(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_phong(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_polylist(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_profile_COMMON(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_rotate(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_scale(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_source(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_technique(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_technique_common(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_translate(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_triangles(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_vertices(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_vertices__input(DaeGlobalData *global, DaeLocalData *local);
gboolean dae_cb_visual_scene(DaeGlobalData *global, DaeLocalData *local);

#endif /* _IMP_DAE_CB_H */
