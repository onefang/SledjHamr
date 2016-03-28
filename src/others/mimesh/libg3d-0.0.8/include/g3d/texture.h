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

#ifndef __G3D_TEXTURE_H__
#define __G3D_TEXTURE_H__

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:texture
 * @short_description: Texture loading and manipulation
 * @include: g3d/texture.h
 *
 * A texture is an image used in materials. Here are some helper functions,
 * mostly for cached loading of a #G3DImage.
 */

/**
 * g3d_texture_load_from_stream:
 * @context: a valid context
 * @model: a valid model or NULL
 * @stream: an open stream
 *
 * Load a texture image from a stream. The file type is determined by the
 * extension of the stream URI, so it should be valid. If @model is not NULL
 * the texture image is cached (or retrieved from cache if available).
 *
 * Returns: the texture image or NULL in case of an error.
 */
EAPI
G3DImage *g3d_texture_load_from_stream(G3DContext *context, G3DModel *model,
	G3DStream *stream);

/**
 * g3d_texture_load_cached:
 * @context: a valid context
 * @model: a valid model
 * @filename: the file name of the texture to load
 *
 * Loads a texture image from file and attaches it to a hash table in the
 * model. On a second try to load this texture it is returned from cache.
 *
 * Returns: the texture image
 */
EAPI
G3DImage *g3d_texture_load_cached(G3DContext *context, G3DModel *model,
	const gchar *filename);

/**
 * g3d_texture_free:
 * @texture: a texture image
 *
 * Frees all memory used by this texture image.
 */
EAPI
void g3d_texture_free(G3DImage *texture);

/**
 * g3d_texture_prepare:
 * @texture: a texture image
 *
 * Resizes the image to dimensions which are a power of 2 to be
 * usable as an OpenGL texture.
 * (FIXME: unimplemented)
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_texture_prepare(G3DImage *texture);

/**
 * g3d_texture_flip_y:
 * @texture: a texture image
 *
 * Mirror the image along the x axis - all y coordinates are inverted.
 *
 * Returns: TRUE on success, FALSE on error.
 */
EAPI
gboolean g3d_texture_flip_y(G3DImage *texture);

/**
 * g3d_texture_merge_alpha:
 * @image: a texture image or NULL
 * @aimage: an image with alpha information
 *
 * Merges alpha information from @aimage into output image. If @image is NULL a
 * new image is created, else @image is returned with alpha from @aimage.
 *
 * Returns: a texture image or NULL in case of an error.
 */
EAPI
G3DImage *g3d_texture_merge_alpha(G3DImage *image, G3DImage *aimage);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_TEXTURE_H__ */

