#ifndef _IMP_SKP_READ_H
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
#define _IMP_SKP_READ_H

#include <g3d/stream.h>

guint32 skp_read_xint16(G3DStream *stream);
gchar *skp_read_char(G3DStream *stream);
gchar *skp_read_wchar(G3DStream *stream);

gboolean skp_read_dbl3(G3DStream *stream,
	gdouble *d1, gdouble *d2, gdouble *d3);
gboolean skp_read_10b(G3DStream *stream);

#endif /* _IMP_SKP_READ_H */
