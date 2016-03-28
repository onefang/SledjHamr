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
#include <string.h>

#include "imp_vrml_types.h"
#include "imp_vrml_read.h"

VrmlType vrml2_type_cb__float3(VrmlReader *reader)
{
	if(!vrml_read_skip_ws(reader))
		return T_UNKNOWN;
	if(*(reader->bufp) == '[')
		return T_LIST_FLOAT3;
	return T_FLOAT3;
}

VrmlType vrml2_type_cb__int(VrmlReader *reader)
{
	if(!vrml_read_skip_ws(reader))
		return T_UNKNOWN;
	if(*(reader->bufp) == '[')
		return T_LIST_INT;
	return T_INT;
}

VrmlType vrml2_type_cb_field(VrmlReader *reader)
{
	gchar *id;
	VrmlType t = T_UNKNOWN;

	id = vrml_read_id(reader);
	if(!id) {
		g_warning("VRML: vrml2_type_cb_field: failed to read id");
		return T_UNKNOWN;
	}
	if(strcmp(id, "SFFloat") == 0)
		t = T_TOKEN2; /* data 0 */

	g_free(id);
	return t;
}

VrmlType vrml2_type_cb_children(VrmlReader *reader)
{
	if(!vrml_read_skip_ws(reader))
		return T_UNKNOWN;
	if(*(reader->bufp) == '[')
		return T_LIST_OBJECT;
	return T_OBJECT;
}

VrmlType vrml2_type_cb_color(VrmlReader *reader)
{
	if(!vrml_read_skip_ws(reader))
		return T_UNKNOWN;
	if(*(reader->bufp) == '[')
		return T_LIST_FLOAT3;
	return T_OBJECT;
}
