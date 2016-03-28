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
#ifndef _G3D_R4_CALLBACKS_H
#define _G3D_R4_CALLBACKS_H

#include <g3d/iff.h>

gboolean r4_cb_DRE2(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_GMAx(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_KSYS(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_KUG1(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_LGH3(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_PKTM(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_RGE1(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_RKAx(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_ROBJ(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_SURx(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_TXM1(G3DIffGlobal *global, G3DIffLocal *local);
gboolean r4_cb_TXO1(G3DIffGlobal *global, G3DIffLocal *local);

#endif /* _G3D_R4_CALLBACKS_H */
