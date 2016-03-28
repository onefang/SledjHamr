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

#ifndef __G3D_MATERIAL_H__
#define __G3D_MATERIAL_H__

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:material
 * @short_description: Material generation and manipulation
 * @include: g3d/material.h
 *
 * A material contains all color, shading and texture information for a
 * #G3DFace.
 */
/**
 * g3d_material_new
 *
 * Generates a new material with a default color.
 *
 * Returns: the new material or NULL on error
 */
EAPI
G3DMaterial *g3d_material_new(void);

/**
 * g3d_material_free:
 * @material: the material to free
 *
 * Frees all memory allocated for that material.
 */
EAPI
void g3d_material_free(G3DMaterial *material);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_MATERIAL_H__ */

