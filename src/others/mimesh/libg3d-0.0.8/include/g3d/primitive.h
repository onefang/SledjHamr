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

#ifndef __G3D_PRIMITIVE_H__
#define __G3D_PRIMITIVE_H__

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:primitive
 * @short_description: 3D primitive generation functions
 * @include: g3d/primitive.h
 *
 * Primitives are objects containing basic 3D geometrical structures. A
 * variety of them can be created using these functions.
 */

/**
 * g3d_primitive_box:
 * @width: the width of the box
 * @height: the height of the box
 * @depth: the depth of the box
 * @material: the material to use for all faces
 *
 * Generates an object containing a box.
 *
 * Returns: the box object
 */
EAPI
G3DObject *g3d_primitive_box(G3DFloat width, G3DFloat height, G3DFloat depth,
	G3DMaterial *material);

#ifndef G3D_DISABLE_DEPRECATED
/**
 * g3d_primitive_cube:
 * @width: the width of the box
 * @height: the height of the box
 * @depth: the depth of the box
 * @material: the material to use for all faces
 *
 * Generates an object containing a box. It is deprecated and now a wrapper for
 * g3d_primitive_box().
 *
 * Returns: the box object
 */
EAPI
G3DObject *g3d_primitive_cube(G3DFloat width, G3DFloat height, G3DFloat depth,
	G3DMaterial *material);
#endif

/**
 * g3d_primitive_cylinder:
 * @radius: the radius of the cylinder
 * @height: the height of the side faces
 * @sides: number of side faces (number of circle segments)
 * @top: add top faces
 * @bottom: add bottom faces
 * @material: material to use for faces
 *
 * Generates an object containing a cylinder.
 *
 * Returns: cylinder object
 */
EAPI
G3DObject *g3d_primitive_cylinder(G3DFloat radius, G3DFloat height,
	guint32 sides, gboolean top, gboolean bottom, G3DMaterial *material);

/**
 * g3d_primitive_tube:
 * @r_in: inner radius
 * @r_out: outer radius
 * @height: the height of the side faces
 * @sides: number of side faces (number of circle segments)
 * @top: add top faces
 * @bottom: add bottom faces
 * @material: material to use for faces
 *
 * Generates an object containing a tube (a cylinder with a hole).
 *
 * Returns: tube object
 */
EAPI
G3DObject *g3d_primitive_tube(G3DFloat r_in, G3DFloat r_out, G3DFloat height,
	guint32 sides, gboolean top, gboolean bottom, G3DMaterial *material);

/**
 * g3d_primitive_sphere:
 * @radius: radius
 * @vseg: number of vertical segments
 * @hseg: number of horizontal segments
 * @material: material to use for faces
 *
 * Generates an object containing a sphere.
 *
 * Returns: sphere object
 */
EAPI
G3DObject *g3d_primitive_sphere(G3DFloat radius, guint32 vseg, guint32 hseg,
	G3DMaterial *material);

/**
 * g3d_primitive_box_strip_2d:
 * @vcnt: number of control points
 * @vdata: 2-dimensional control point data (2 * vcnt * gdouble)
 * @height: height of resulting strip (y component)
 * @width: width of strip (corner diameter)
 * @material: material to use for faces
 *
 * Generates a strip of box segments defined by corner center points
 * using two-dimensional data (x/z plane).
 *
 * Returns: strip object
 */
EAPI
G3DObject *g3d_primitive_box_strip_2d(guint32 vcnt, gdouble *vdata,
	gdouble height, gdouble width, G3DMaterial *material);

/**
 * g3d_primitive_mesh:
 * @m: number of vertices in m direction
 * @n: number of vertices in n direction
 * @wrap_m: wrap around in m direction
 * @wrap_n: wrap around in n direction
 * @material: material to use for faces
 *
 * Generate a mesh consisting of m * n vertices. The vertex data is
 * initialized with (0.0, 0.0, 0.0) and has to be set to something
 * useful.
 *
 * Returns: mesh object
 */
EAPI
G3DObject *g3d_primitive_mesh(guint32 m, guint32 n, gboolean wrap_m,
	gboolean wrap_n, G3DMaterial *material);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_PRIMITIVE_H__ */

