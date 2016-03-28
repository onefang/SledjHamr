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

#ifndef __G3D_MODEL_H__
#define __G3D_MODEL_H__

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:model
 * @short_description: Model manipulation functions
 * @include: g3d/g3d.h
 *
 * A model is a group of objects. All information loaded from a file by libg3d
 * is found in this model.
 */

/**
 * g3d_model_new:
 *
 * This functions allocates and initializes a new G3DModel.
 *
 * Returns: a newly allocated G3DModel
 */
EAPI
G3DModel *g3d_model_new(void);

/**
 * G3D_MODEL_SCALE:
 *
 * The model should be scaled to a maximum extension of +/- 10.0.
 */
#define G3D_MODEL_SCALE           (1 << 0)
/**
 * G3D_MODEL_CENTER:
 *
 * The model should be centered around the (0,0,0).
 */
#define G3D_MODEL_CENTER          (1 << 1)
/**
 * G3D_MODEL_OPTIMIZE:
 *
 * The model material/object/face lists should be serialized to some private
 * arrays (deprecated).
 */
#define G3D_MODEL_OPTIMIZE        (1 << 2)
/**
 * G3D_MODEL_NOCHECK:
 *
 * The common checks should be disabled. The checks include:
 * <itemizedlist>
 * <listitem>faces have at least 3 indices</listitem>
 * <listitem>face indices are <= number of vertices</listitem>
 * <listitem>material of faces is not NULL</listitem>
 * </itemizedlist>
 */
#define G3D_MODEL_NOCHECK         (1 << 3)

/**
 * g3d_model_load_full:
 * @context: a valid context
 * @filename: the file name of the model to load
 * @flags: object manipulation flags
 *
 * Loads a model from a file. Depending on @flags the model is checked,
 * centered, resized, optimized.
 *
 * Returns: the loaded model or NULL in case of an error.
 */
EAPI
G3DModel *g3d_model_load_full(G3DContext *context, const gchar *filename,
	guint32 flags);

/**
 * g3d_model_load:
 * @context: a valid context
 * @filename: the file name of the model to load
 *
 * Loads a model from a file. The model is checked, centered, resized,
 * optimized.
 *
 * Returns: the loaded model or NULL in case of an error
 */
EAPI
G3DModel *g3d_model_load(G3DContext *context, const gchar *filename);

/**
 * g3d_model_check:
 * @model: the model to check
 *
 * Checks whether a model returned by plugin is valid.
 *
 * Returns: TRUE on success, FALSE on error
 */
EAPI
gboolean g3d_model_check(G3DModel *model);

/**
 * g3d_model_center:
 * @model: the model to center
 *
 * Translates all object coordinates that the object center is at (0, 0, 0)
 *
 * Returns: TRUE on success, FALSE on error
 */
EAPI
gboolean g3d_model_center(G3DModel *model);

/**
 * g3d_model_clear:
 * @model: the model to clear
 *
 * Removes all objects from a model.
 */
EAPI
void g3d_model_clear(G3DModel *model);

/**
 * g3d_model_free:
 * @model: the model to free
 *
 * Frees all memory allocated for the model including all objects, materials
 * and textures.
 */
EAPI
void g3d_model_free(G3DModel *model);

/**
 * g3d_model_get_object_by_name:
 * @model: the model containing all objects
 * @name: the name of the requested object
 *
 * Searches the object tree for an object with the given name.
 *
 * Returns: the requested object or NULL if non was found
 */
EAPI
G3DObject *g3d_model_get_object_by_name(G3DModel *model, const gchar *name);

/**
 * g3d_model_transform:
 * @model: the model
 * @matrix: transformation matrix
 *
 * Transform all toplevel objects in model with matrix.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_model_transform(G3DModel *model, G3DMatrix *matrix);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_MODEL_H__ */

