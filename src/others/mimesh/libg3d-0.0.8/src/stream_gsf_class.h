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
#ifndef _STREAM_GSF_CLASS_H
#define _STREAM_GSF_CLASS_H

#include <gsf/gsf-input.h>
#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */


#define G3D_GSF_INPUT_STREAM_TYPE      (g3d_gsf_input_stream_get_type())
#define G3D_GSF_INPUT_STREAM(o)        (G_TYPE_CHECK_INSTANCE_CAST((o), \
	G3D_GSF_INPUT_STREAM_TYPE, G3DGsfInputStream))
#define G3D_IS_GSF_INPUT_STREAM(o)     (G_TYPE_CHECK_INSTANCE_TYPE((o), \
	G3D_GSF_INPUT_STREAM_TYPE))

typedef struct _G3DGsfInputStream G3DGsfInputStream;

EAPI GType g3d_gsf_input_stream_get_type(void) G_GNUC_CONST;

EAPI GsfInput *g3d_gsf_input_stream_new(G3DStream *stream);

/* Classes be damned, I don't have time to learn this opaque class system.  Maybe fix it later, or just replace it all.  Pffft */
EAPI G3DStream *g3d_gsf_input_stream_get_stream(GsfInput *input);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* _STREAM_GSF_CLASS_H */

