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
#ifndef _IMP_DXF_PROP_H
#define _IMP_DXF_PROP_H

#include "imp_dxf_types.h"

DxfEntityProps *dxf_prop_create(void);
void dxf_prop_cleanup(DxfEntityProps *eprop);

gboolean dxf_prop_set_int(DxfEntityProps *eprop, gint32 key, gint32 i);
gboolean dxf_prop_set_dbl(DxfEntityProps *eprop, gint32 key, gdouble dbl);
gboolean dxf_prop_set_str(DxfEntityProps *eprop, gint32 key,
	const gchar *str);

gint32 dxf_prop_get_int(DxfEntityProps *eprop, gint32 key, gint32 dfl);
gdouble dxf_prop_get_dbl(DxfEntityProps *eprop, gint32 key, gdouble dfl);
const gchar *dxf_prop_get_str(DxfEntityProps *eprop, gint32 key,
	const gchar *dfl);

#endif /* _IMP_DXF_PROP_H */
