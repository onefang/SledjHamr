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
#ifndef _IMP_VRML2_TYPES_H
#define _IMP_VRML2_TYPES_H

#include "imp_vrml2.h"
#include "imp_vrml_types.h"

/* list or not list */
VrmlType vrml2_type_cb__float3(VrmlReader *reader);
VrmlType vrml2_type_cb__int(VrmlReader *reader);

VrmlType vrml2_type_cb_field(VrmlReader *reader);
VrmlType vrml2_type_cb_children(VrmlReader *reader);
VrmlType vrml2_type_cb_color(VrmlReader *reader);

static VrmlTypeDef vrml2_types[] = {
	{ "ambientIntensity",         T_FLOAT,            NULL },
	{ "appearance",               T_OBJECT,           NULL },
	{ "autoOffset",               T_BOOLEAN,          NULL },
	{ "ccw",                      T_BOOLEAN,          NULL },
	{ "center",                   T_FLOAT_X,          NULL },
	{ "children",                 T_UNKNOWN,          vrml2_type_cb_children },
	{ "color",                    T_UNKNOWN,          vrml2_type_cb_color },
	{ "colorIndex",               T_LIST_INT,         vrml2_type_cb__int },
	{ "colorPerVertex",           T_BOOLEAN,          NULL },
	{ "convex",                   T_BOOLEAN,          NULL },
	{ "coord",                    T_OBJECT,           NULL },
	{ "coordIndex",               T_LIST_INT,         NULL },
	{ "creaseAngle",              T_FLOAT,            NULL },
	{ "cycleInterval",            T_INT,              NULL },
	{ "description",              T_STRING,           NULL },
	{ "diffuseColor",             T_FLOAT3,           NULL },
	{ "emissiveColor",            T_FLOAT3,           NULL },
	{ "eventIn",                  T_TOKEN2,           NULL },
	{ "eventOut",                 T_TOKEN2,           NULL },
	{ "field",                    T_UNKNOWN,          vrml2_type_cb_field },
	{ "fieldOfView",              T_FLOAT,            NULL },
	{ "geometry",                 T_OBJECT,           NULL },
	{ "groundAngle",              T_LIST_FLOAT2,      NULL },
	{ "groundColor",              T_LIST_FLOAT3,      NULL },
	{ "height",                   T_FLOAT,            NULL },
	{ "info",                     T_LIST_STRING,      NULL },
	{ "key",                      T_LIST_FLOAT,       NULL },
	{ "keyValue",                 T_LIST_FLOAT4,      NULL },
	{ "material",                 T_OBJECT,           NULL },
	{ "maxPosition",              T_FLOAT_X,          NULL },
	{ "minPosition",              T_FLOAT_X,          NULL },
	{ "normal",                   T_OBJECT,           NULL },
	{ "normalIndex",              T_LIST_INT,         NULL },
	{ "normalPerVertex",          T_BOOLEAN,          NULL },
	{ "offset",                   T_FLOAT_X,          NULL },
	{ "orientation",              T_FLOAT4,           NULL },
	{ "parameter",                T_STRING,           NULL },
	{ "point",                    T_LIST_FLOAT_X,     NULL },
	{ "position",                 T_FLOAT3,           NULL },
	{ "radius",                   T_FLOAT,            NULL },
	{ "repeatS",                  T_BOOLEAN,          NULL },
	{ "repeatT",                  T_BOOLEAN,          NULL },
	{ "rotation",                 T_FLOAT_X,          NULL },
	{ "scale",                    T_FLOAT_X,          NULL },
	{ "scaleOrientation",         T_FLOAT_X,          NULL },
	{ "shininess",                T_FLOAT,            NULL },
	{ "size",                     T_FLOAT3,           NULL },
	{ "skyAngle",                 T_LIST_FLOAT2,      NULL },
	{ "skyColor",                 T_LIST_FLOAT3,      vrml2_type_cb__float3 },
	{ "solid",                    T_BOOLEAN,          NULL },
	{ "specularColor",            T_FLOAT3,           NULL },
	{ "startTime",                T_INT,              NULL },
	{ "stopTime",                 T_INT,              NULL },
	{ "texCoord",                 T_OBJECT,           NULL },
	{ "texCoordIndex",            T_LIST_INT,         NULL },
	{ "texture",                  T_OBJECT,           NULL },
	{ "textureTransform",         T_OBJECT,           NULL },
	{ "translation",              T_FLOAT3,           NULL },
	{ "transparency",             T_FLOAT,            NULL },
	{ "type",                     T_STRING,           NULL },
	{ "url",                      T_STRING,           NULL },
	{ "vector",                   T_LIST_FLOAT3,      NULL },

	{ NULL, T_UNKNOWN }
};

#endif /* _IMP_VRML2_TYPES_H */
