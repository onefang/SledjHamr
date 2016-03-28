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
#ifndef _IMP_ACF_WING_H
#define _IMP_ACF_WING_H

#include <g3d/types.h>

#include "imp_acf_airfoil.h"

G3DObject *acf_wing(G3DMaterial *material, const gchar *name,
	G3DFloat *msweep, G3DFloat *mdihed, G3DFloat *mtrans,
	G3DFloat *vrt, G3DFloat *vtp,
	AcfAirfoil *aflrt, AcfAirfoil *afltp,
	G3DFloat wrt, G3DFloat wtp, G3DFloat lf);

#endif
