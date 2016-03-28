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
#ifndef _IMP_LWO_CALLBACKS_H
#define _IMP_LWO_CALLBACKS_H

#include <g3d/iff.h>

gboolean lwo_cb_CLIP(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_COLR(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_IMAG(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_PNTS(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_POLS(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_PTAG(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_SPEC(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_SRFS(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_STIL(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_SURF(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_TAGS(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_TRAN(G3DIffGlobal *global, G3DIffLocal *local);
gboolean lwo_cb_VMAP(G3DIffGlobal *global, G3DIffLocal *local);

#endif
