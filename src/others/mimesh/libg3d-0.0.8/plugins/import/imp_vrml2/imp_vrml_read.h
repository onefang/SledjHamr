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
#ifndef _IMP_VRML_READ_H
#define _IMP_VRML_READ_H

#include "imp_vrml2.h"
#include "imp_vrml_types.h"

typedef gboolean (* vrml_read_list_item_callback)(VrmlReader *reader,
	gpointer user_data);

gboolean vrml_read_global(VrmlReader *reader);
gboolean vrml_read_check_buffer(VrmlReader *reader);
gboolean vrml_read_skip_ws(VrmlReader *reader);
gboolean vrml_read_skip_unknown(VrmlReader *reader);
gboolean vrml_read_expect(VrmlReader *reader, const gchar c);
gboolean vrml_read_list(VrmlReader *reader, vrml_read_list_item_callback cb,
	gpointer user_data);

gchar *vrml_read_token(VrmlReader *reader);
gchar *vrml_read_numeric(VrmlReader *reader);
gchar *vrml_read_id(VrmlReader *reader);
gchar *vrml_read_nodeid(VrmlReader *reader);
gchar *vrml_read_string(VrmlReader *reader);

VrmlProperty *vrml_read_property(VrmlReader *reader, guint32 level);
VrmlObject *vrml_read_object(VrmlReader *reader, guint32 level);

static inline void vrml_read_dec(VrmlReader *reader)
{
	reader->bufp ++;
	reader->bufsize --;
}

static gchar vrml_read_padding[] = "                                         ";

#endif /* _IMP_VRML_READ_H */
