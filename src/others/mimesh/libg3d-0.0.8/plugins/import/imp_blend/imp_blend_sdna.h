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
#ifndef _IMP_BLEND_SDNA_H
#define _IMP_BLEND_SDNA_H

#include <g3d/stream.h>

#include "imp_blend_types.h"

/* as in #if DEBUG > BLEND_DEBUG_STRUCT */
#define BLEND_DEBUG_STRUCT 1

BlendSdna *blend_sdna_read_dna1(G3DStream *stream, guint32 flags, gint32 len);
void blend_sdna_cleanup(BlendSdna *sdna);

const BlendSdnaStruct *blend_sdna_get_struct_by_id(BlendSdna *sdna,
	guint32 sdnanr);
const BlendSdnaStruct *blend_sdna_get_struct_by_name(BlendSdna *sdna,
	const gchar *name);

BlendSdnaData *blend_sdna_data_read(BlendSdna *sdna,
	const BlendSdnaStruct *sstruct, BlendGlobal *global, gsize *r,
	guint32 level);
void blend_sdna_data_free(BlendSdnaData *sdata);

BlendSdnaPropData *blend_sdna_get_property(BlendSdnaData *sdata,
	const gchar *name, BlendSdnaPropType type);

#if DEBUG > 0
gboolean blend_sdna_dump_data(BlendSdnaData *sdata, guint32 level);
gboolean blend_sdna_dump_struct(BlendSdna *sdna, guint32 sdnanr);
#endif

#endif /* _IMP_BLEND_SDNA_H */
