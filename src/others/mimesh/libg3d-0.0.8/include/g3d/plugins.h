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

#ifndef __G3D_PLUGINS_H__
#define __G3D_PLUGINS_H__

#include <gmodule.h>
#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:plugins
 * @short_description: G3DPlugin interface
 * @see_also: #G3DPlugin
 * @include: g3d/plugins.h
 *
 * Direct interaction with the plugin system is normally not needed when using
 * libg3d. It may be required when writing a plugin which should load a
 * #G3DImage or a #G3DModel with another plugin.
 */

/**
 * G3DPluginType:
 * @G3D_PLUGIN_UNKNOWN: unknown plugin type
 * @G3D_PLUGIN_IMPORT: model import plugin
 * @G3D_PLUGIN_IMAGE: image loading plugin
 *
 * Type of plugin.
 */
typedef enum {
	G3D_PLUGIN_UNKNOWN = 0x00,
	G3D_PLUGIN_IMPORT,
	G3D_PLUGIN_IMAGE
} G3DPluginType;

/**
 * G3DPluginInitFunc:
 * @context: the context
 *
 * Prototype for plugin_init().
 *
 * Returns: opaque plugin data.
 */
typedef gpointer (* G3DPluginInitFunc)(G3DContext *context);

/**
 * G3DPluginCleanupFunc:
 * @user_data: opaque plugin data
 *
 * Prototype for plugin_cleanup().
 */
typedef void (* G3DPluginCleanupFunc)(gpointer user_data);

/**
 * G3DPluginLoadModelFromStreamFunc:
 * @context: the context
 * @stream: the stream to load from
 * @model: the model structure to fill
 * @user_data: opaque plugin data
 *
 * Prototype for plugin_load_model_from_stream().
 *
 * Returns: TRUE on success, FALSE else.
 */
typedef gboolean (* G3DPluginLoadModelFromStreamFunc)(G3DContext *context,
	G3DStream *stream, G3DModel *model, gpointer user_data);

/**
 * G3DPluginLoadImageStreamFunc:
 * @context: the context
 * @stream: the stream to load from
 * @image: image structure to fill
 * @user_data: opaque plugin data
 *
 * Prototype for plugin_load_image_from_stream().
 *
 * Returns: TRUE on success, FALSE else.
 */
typedef gboolean (* G3DPluginLoadImageStreamFunc)(G3DContext *context,
	G3DStream *stream, G3DImage *image, gpointer user_data);

/**
 * G3DPluginGetDescFunc:
 * @context: the context
 *
 * Prototype for plugin_description().
 *
 * Returns: a newly-allocated string containing the description of the plugin.
 */
typedef gchar *(* G3DPluginGetDescFunc)(G3DContext *context);

/**
 * G3DPluginGetExtFunc:
 * @context: the context
 *
 * Prototype for plugin_extensions().
 *
 * Returns: NULL-terminated list of file extensions supported by this plugin.
 * Free with g_strfreev().
 */
typedef gchar **(* G3DPluginGetExtFunc)(G3DContext *context);

/**
 * G3DPlugin:
 *
 * A libg3d plugin.
 */
struct _G3DPlugin {
	/*< private >*/
	gchar *name;
	gchar *path;
	G3DPluginType type;
	gchar **extensions;

	G3DPluginInitFunc init_func;
	G3DPluginCleanupFunc cleanup_func;
	G3DPluginLoadModelFromStreamFunc loadmodelstream_func;
	G3DPluginLoadImageStreamFunc loadimagestream_func;
	G3DPluginGetDescFunc desc_func;
	G3DPluginGetExtFunc ext_func;

	gpointer user_data;

	GModule *module;
};

/**
 * g3d_plugins_init:
 * @context: a valid #G3DContext
 *
 * Initializes the plugin system. This is implicitly done when using
 * g3d_context_new().
 *
 * Returns: TRUE on success, FALSE else.
 */
EAPI
gboolean g3d_plugins_init(G3DContext *context);

/**
 * g3d_plugins_cleanup:
 * @context: a valid context
 *
 * Clean up the plugin system. Usually done by g3d_context_free().
 */
EAPI
void g3d_plugins_cleanup(G3DContext *context);

/**
 * g3d_plugins_load_model:
 * @context: a valid context
 * @filename: file name of model to load
 * @model: model structure to fill
 *
 * Try to load a model from file using import plugins.
 *
 * Returns: TRUE on success, FALSE else.
 */
EAPI
gboolean g3d_plugins_load_model(G3DContext *context, const gchar *filename,
	G3DModel *model);

/**
 * g3d_plugins_load_model_from_stream:
 * @context: a valid context
 * @stream: stream to load model from
 * @model: model structure to fill
 *
 * Try to load a model from stream using import plugins.
 *
 * Returns: TRUE on success, FALSE else.
 */
EAPI
gboolean g3d_plugins_load_model_from_stream(G3DContext *context,
	G3DStream *stream, G3DModel *model);

/**
 * g3d_plugins_load_image:
 * @context: a valid context
 * @filename: file name of image to load
 * @image: image structure to fill
 *
 * Try to load an image from file using import plugins.
 *
 * Returns: TRUE on success, FALSE else.
 */
EAPI
gboolean g3d_plugins_load_image(G3DContext *context, const gchar *filename,
	G3DImage *image);

/**
 * g3d_plugins_load_image_from_stream:
 * @context: a valid context
 * @stream: stream to load image from
 * @image: image structure to fill
 *
 * Try to load an image from stream using import plugins.
 *
 * Returns: TRUE on success, FALSE else.
 */
EAPI
gboolean g3d_plugins_load_image_from_stream(G3DContext *context,
	G3DStream *stream, G3DImage *image);

/**
 * g3d_plugins_get_image_extensions:
 * @context: a valid context
 *
 * Get the supported image type extensions.
 *
 * Returns: NULL-terminated list of image file extensions supported by this
 * plugin. Free with g_strfreev().
 */
EAPI
gchar **g3d_plugins_get_image_extensions(G3DContext *context);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_PLUGINS_H__ */

