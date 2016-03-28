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
#ifndef _IMP_LDRAW_LIBRARY_H
#define _IMP_LDRAW_LIBRARY_H

#include "imp_ldraw_types.h"

LDrawLibrary *ldraw_library_init(void);
void ldraw_library_cleanup(LDrawLibrary *lib);
void ldraw_library_insert(LDrawLibrary *lib, gchar *name, gpointer data);
G3DObject *ldraw_library_lookup(LDrawLibrary *lib, const gchar *name);

#endif
