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
#ifndef _G3D_DEBUG_H
#define _G3D_DEBUG_H


#include <glib.h>
/* g_debug() is defined since glib 2.6 */

/*< private >*/
#ifndef g_debug
#define g_debug(...) g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#endif
/*< public >*/

#if DEBUG > 0
#include <string.h>
static const gchar *debug_padding_str = "                                    ";
static inline const gchar *debug_pad(guint32 level) {
	return debug_padding_str + (strlen(debug_padding_str) - level);
}
#endif

#endif /* _G3D_DEBUG_H */

