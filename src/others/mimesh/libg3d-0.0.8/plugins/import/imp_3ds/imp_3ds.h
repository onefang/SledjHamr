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
#ifndef _IMP_3DS_H
#define _IMP_3DS_H

#include <stdio.h>
#include <glib.h>
#include <g3d/g3d.h>
#include <g3d/stream.h>

typedef struct {
    G3DContext *context;
    G3DModel *model;
    G3DStream *stream;
    G3DFloat scale;
	gint32 max_tex_id;
} x3ds_global_data;

typedef struct {
    gint32 id;
    gpointer object;
	gpointer misc_object;
    gint32 level;
	gpointer level_object;
    guint32 nb;
} x3ds_parent_data;

typedef gboolean (* x3ds_callback)(x3ds_global_data *global,
    x3ds_parent_data *parent);

gboolean x3ds_read_ctnr(x3ds_global_data *global, x3ds_parent_data *parent);
void x3ds_update_progress(x3ds_global_data *global, guint32 level);
gint32 x3ds_read_cstr(G3DStream *stream, gchar *string);
G3DObject *x3ds_newobject(G3DModel *model, const gchar *name);

#endif /* _IMP_3DS_H */
