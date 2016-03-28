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

#ifndef __G3D_CONTEXT_H__
#define __G3D_CONTEXT_H__

#include <g3d/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */


/**
 * SECTION:context
 * @short_description: Libg3d initialization and configuration
 * @see_also: #G3DContext
 * @include: g3d/g3d.h
 *
 * All state information is saved in the context. It also serves as an
 * interface to the application.
 */


/**
 * g3d_context_new:
 *
 * Create a new context. This initializes the library (and also the
 * plugin system so this has not to be done seperately).
 *
 * Returns: a valid context, or NULL on failure.
 */
EAPI
G3DContext *g3d_context_new(void);

/**
 * g3d_context_free:
 * @context: the context to free
 *
 * Cleans up the context and the plugin system and frees all reserved
 * memory.
 */
EAPI
void g3d_context_free(G3DContext *context);

/**
 * g3d_context_set_bgcolor:
 * @context: a valid context
 * @r: red component (range: 0.0 .. 1.0)
 * @g: green component (range: 0.0 .. 1.0)
 * @b: green component (range: 0.0 .. 1.0)
 * @a: alpha component
 *
 * Try to set the background color. This will call a function registered
 * with <link>@g3d_context_set_set_bgcolor_func</link>.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_context_set_bgcolor(G3DContext *context,
	G3DFloat r, G3DFloat g, G3DFloat b, G3DFloat a);

/**
 * g3d_context_update_interface:
 * @context: a valid context
 *
 * Try to update the interface. This will call a function registered with
 * @g3d_context_set_update_interface_func.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_context_update_interface(G3DContext *context);

/**
 * g3d_context_update_progress_bar:
 * @context: a valid context
 * @percentage: the percentage to set on the progress bar
 * @visibility: show or hide the progress bar
 *
 * Try to update the progress bar.
 *
 * Returns: TRUE on success, FALSE else
 */
EAPI
gboolean g3d_context_update_progress_bar(G3DContext *context,
	G3DFloat percentage, gboolean visibility);

/**
 * g3d_context_set_set_bgcolor_func:
 * @context: a valid context
 * @func: the callback function
 * @user_data: user-defined opaque pointer
 *
 * Registers a callback function for setting the background color.
 */
EAPI
void g3d_context_set_set_bgcolor_func(G3DContext *context,
	G3DSetBgColorFunc func, gpointer user_data);

/**
 * g3d_context_set_update_interface_func:
 * @context: a valid context
 * @func: the callback function
 * @user_data: user-defined opaque pointer
 *
 * Registers a callback function for updating the interface.
 */
EAPI
void g3d_context_set_update_interface_func(G3DContext *context,
	G3DUpdateInterfaceFunc func, gpointer user_data);

/**
 * g3d_context_set_update_progress_bar_func:
 * @context: a valid context
 * @func: the callback function
 * @user_data: user-defined opaque pointer
 *
 * Registers a callback function for updating the progress bar.
 */
EAPI
void g3d_context_set_update_progress_bar_func(G3DContext *context,
	G3DUpdateProgressBarFunc func, gpointer user_data);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_CONTEXT_H__ */

