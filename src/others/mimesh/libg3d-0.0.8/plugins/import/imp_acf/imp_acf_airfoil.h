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
#ifndef _IMP_ACF_AIRFOIL_H
#define _IMP_ACF_AIRFOIL_H

#include <glib.h>

typedef struct {
	gchar *filename;
	gchar *description;
	/* 2D vertexes */
	guint32 vertex_count;
	G3DFloat *vertex_data;
} AcfAirfoil;

typedef struct {
	GHashTable *db;
	GSList *airfoils;
} AcfAirfoilDb;

AcfAirfoilDb *acf_airfoil_init(void);
void acf_airfoil_cleanup(AcfAirfoilDb *db);
AcfAirfoil *acf_airfoil_lookup(AcfAirfoilDb *db, const gchar *aflname);

#endif /* _IMP_ACF_AIRFOIL_H */
