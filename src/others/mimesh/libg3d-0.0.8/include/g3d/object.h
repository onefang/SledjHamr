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

#ifndef __G3D_OBJECT_H__
#define __G3D_OBJECT_H__

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:object
 * @short_description: Object manipulation
 * @include: g3d/object.h
 *
 * Objects are parts of a model. In most file formats vertices and faces are
 * grouped in some way into objects. Objects can be hierarchical, so what a
 * model contains is basically an object tree.
 */

/**
 * g3d_object_free:
 * @object: the object to free
 *
 * Frees all memory allocated for that object.
 */
EAPI
void g3d_object_free(G3DObject *object);

/**
 * g3d_object_radius:
 * @object: the object to measure
 *
 * Calculates the radius of the object. This is the maximum from the
 * center to a vertex.
 *
 * Returns: the radius of the given object
 */
EAPI
gdouble g3d_object_radius(G3DObject *object);

/**
 * g3d_object_scale:
 * @object: the object to scale
 * @scale: scale factor
 *
 * Resizes the object by the factor @scale.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_object_scale(G3DObject *object, G3DFloat scale);

/**
 * g3d_object_transform:
 * @object: the object to transform
 * @matrix: the transformation matrix
 *
 * Multiplies all vertices of the object with the transformation matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_object_transform(G3DObject *object, G3DMatrix *matrix);

/**
 * g3d_object_transform_normals:
 * @object: the object to transform
 * @matrix: the transformation matrix
 *
 * Multiplies all normals of the object with the transformation matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_object_transform_normals(G3DObject *object, G3DMatrix *matrix);

/**
 * g3d_object_duplicate:
 * @object: the object to duplicate
 *
 * Duplicates an object with all vertices, faces and materials.
 *
 * Returns: the new clone object
 */
EAPI
G3DObject *g3d_object_duplicate(G3DObject *object);

/**
 * g3d_object_optimize:
 * @object: the object to optimize
 *
 * Puts all vertex and face information into special arrays for faster
 * rendering. It is deprecated and should not be used.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_object_optimize(G3DObject *object);

/**
 * g3d_object_smooth:
 * @object: the object to smooth
 *
 * FIXME: unimplemented.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_object_smooth(G3DObject *object);

/**
 * g3d_object_merge:
 * @o1: first and target object
 * @o2: second object
 *
 * Merges both objects into @o1.
 * FIXME: needs cleanup
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_object_merge(G3DObject *o1, G3DObject *o2);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_OBJECT_H__ */

