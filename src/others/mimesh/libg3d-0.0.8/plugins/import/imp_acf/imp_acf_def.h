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
#ifndef _IMP_ACF_DEF_H
#define _IMP_ACF_DEF_H

#include <g3d/stream.h>

#include "imp_acf_def_proto.h"

typedef struct {
	GHashTable *db;
} AcfFile;

AcfFile *acf_def_read(G3DStream *stream, const AcfDef *def,
	gboolean bigendian);
void acf_def_free(AcfFile *acf);
AcfValue *acf_def_lookup(AcfFile *acf, const gchar *name);
void acf_def_dump(AcfValue *value);

#endif /* _IMP_ACF_DEF_H */
