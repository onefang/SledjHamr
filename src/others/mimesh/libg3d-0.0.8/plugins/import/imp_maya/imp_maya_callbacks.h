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
#ifndef _IMP_MAYA_CALLBACKS_H
#define _IMP_MAYA_CALLBACKS_H

#include <g3d/iff.h>

gboolean maya_cb_CMPD(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_CREA(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_CWFL(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_DBLn(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_DBL2(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_DBL3(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_DBLE(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_DMSH(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_FLT3(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_MATR(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_MESH(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_PCUB(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_PCYL(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_STR_(G3DIffGlobal *global, G3DIffLocal *local);
gboolean maya_cb_XFRM(G3DIffGlobal *global, G3DIffLocal *local);

#endif /* _IMP_MAYA_CALLBACKS_H */
