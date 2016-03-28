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
#ifndef _IMP_BLEND_DEF_H
#define _IMP_BLEND_DEF_H

#include <glib.h>

#define BLEND_HEADER_SIZE          12

#define FLAG_LITTLE_ENDIAN   1
#define FLAG_POINTERSIZE_4   4
#define FLAG_POINTERSIZE_8   8
#define FLAG_POINTER_MASK  0xC

#define MKID(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define MKID2(a,b) ((a) | ((b) << 8))

static inline gchar blend_from_id(guint32 id, guint8 pos) {
	guint8 c = ((id >> (pos * 8)) & 0xFF);
	return ((c < 0x20) || (c > 0x7F)) ? '_' : c;
}

#endif /* _IMP_BLEND_DEF_H */
