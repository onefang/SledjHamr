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
#ifndef _IMP_VRML_TYPES_H
#define _IMP_VRML_TYPES_H

#include "imp_vrml2.h"

typedef enum {
	T_UNKNOWN = 0,
	T_SPECIAL,
	T_OBJECT,
	T_TOKEN2,
	T_TOKEN3,
	T_STRING,
	T_BOOLEAN,
	T_INT,
	T_FLOAT,
	T_FLOAT2,
	T_FLOAT3,
	T_FLOAT4,
	T_FLOAT_X,
	T_LIST_OBJECT,
	T_LIST_STRING,
	T_LIST_INT,
	T_LIST_FLOAT,
	T_LIST_FLOAT2,
	T_LIST_FLOAT3,
	T_LIST_FLOAT4,
	T_LIST_FLOAT_X,
	N_TYPES
} VrmlType;

typedef struct {
	gchar *name;
	gchar *defid;
	gboolean define;
	guint32 level;
	GSList *properties;
} VrmlObject;

typedef struct {
	gchar *name;
	VrmlType id;

	gsize n_items;
	VrmlObject **v_object;
	gchar **v_str;
	G3DFloat *v_float;
	gint32 *v_int;
	gboolean v_boolean;
	guint32 level;
} VrmlProperty;

typedef VrmlType (* VrmlTypeDetect)(VrmlReader *reader);

typedef struct {
	const gchar *name;
	VrmlType id;
	VrmlTypeDetect detect;
} VrmlTypeDef;

typedef gboolean (* VrmlNodeCallback)(VrmlReader *reader, VrmlObject *object);

typedef struct {
	const gchar *name;
	VrmlNodeCallback callback;
} VrmlNodeDef;

#endif /* _IMP_VRML_TYPES_H */
