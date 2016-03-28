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

#include <stdlib.h>
#include <string.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/plugins.h>
#include <g3d/material.h>
#include <g3d/primitive.h>

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	G3DImage *image = g_new0(G3DImage, 1);
	G3DObject *object;
	G3DMaterial *material;
	guint32 x, y, index;
	G3DFloat pcnt, prev_pcnt = 0.0;

	if(!g3d_plugins_load_image_from_stream(context, stream, image)) {
		g_free(image);
		return FALSE;
	}

	material = g3d_material_new();
	material->name = g_strdup("default material");
	material->r = 0.4;
	material->g = 0.4;
	material->b = 0.4;
	material->a = 1.0;
	model->materials = g_slist_append(model->materials, material);

	object = g3d_primitive_mesh(image->width, image->height, FALSE, FALSE,
		material);
	object->name = g_strdup("height field");
	model->objects = g_slist_append(model->objects, object);

#if DEBUG > 0
	g_debug("height field loader: image: %dx%dx%d",
		image->width, image->height, image->depth);
#endif

	for(y = 0; y < image->height; y ++) {
		for(x = 0; x < image->width; x ++) {
			index = y * image->width + x;

			object->vertex_data[index * 3 + 0] = x;
			object->vertex_data[index * 3 + 1] = y;
			switch(image->depth) {
				case 8:
					object->vertex_data[index * 3 + 2] = 0.0 +
						(G3DFloat)image->pixeldata[index] / 32.0;
					break;
				case 15:
				case 16:
					object->vertex_data[index*3+2] = 0.0 +
						*((guint16*)&image->pixeldata[index]);
					break;
				case 24:
				case 32:
					object->vertex_data[index * 3 + 2] = 0.0 +
						image->pixeldata[index * 4] / 32.0;
					break;
				default:
					break;
			}

			pcnt = (G3DFloat)(y * image->width + x) /
				(G3DFloat)(image->width * image->height);
			if((pcnt - prev_pcnt) > 0.01) {
				prev_pcnt = pcnt;
				g3d_context_update_progress_bar(context, pcnt, TRUE);
			}
			g3d_context_update_interface(context);
		} /* for(x) */
	} /* for(y) */

	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("Generate height fields from images.\n"
		"Author: Markus Dahms");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g3d_plugins_get_image_extensions(context);
}

