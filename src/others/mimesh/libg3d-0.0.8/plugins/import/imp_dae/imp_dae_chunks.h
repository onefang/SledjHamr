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

#ifndef _IMP_DAE_CHUNKS_H
#define _IMP_DAE_CHUNKS_H

#include "imp_dae_cb.h"

static DaeChunkDesc dae_chunks_bind_material[] = {
	{ "technique_common", dae_cb_technique_common },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_effect[] = {
	{ "profile_COMMON",   dae_cb_profile_COMMON },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_geometry[] = {
	{ "bind_material",    dae_cb_bind_material },
	{ "mesh",             dae_cb_mesh },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_material[] = {
	{ "effect",           dae_cb_effect },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_mesh[] = {
	{ "lines",            NULL },
	{ "polylist",         dae_cb_polylist },
	{ "source",           dae_cb_source },
	{ "triangles",        dae_cb_triangles },
	{ "vertices",         dae_cb_vertices },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_node[] = {
	{ "camera",           NULL },
	{ "controller",       NULL },
	{ "geometry",         dae_cb_geometry },
	{ "light",            NULL },
	{ "matrix",           dae_cb_matrix },
	{ "node",             dae_cb_node },
	{ "scale",            dae_cb_scale },
	{ "rotate",           dae_cb_rotate },
	{ "translate",        dae_cb_translate },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_profile_COMMON[] = {
	{ "extra",            NULL },
	{ "newparam",         dae_cb_newparam },
	{ "technique",        dae_cb_technique },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_technique[] = {
	{ "blinn",            dae_cb_phong },
	{ "lambert",          dae_cb_phong },
	{ "phong",            dae_cb_phong },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_vertices[] = {
	{ "input",            dae_cb_vertices__input },
	{ NULL, NULL }
};

static DaeChunkDesc dae_chunks_visual_scene[] = {
	{ "node",             dae_cb_node },
	{ NULL, NULL }
};

#endif /* _IMP_DAE_CHUNKS_H */
