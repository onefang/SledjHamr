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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>

#include <g3d/types.h>
#include <g3d/read.h>
#include <g3d/material.h>
#include <g3d/matrix.h>

#include "imp_ar.h"
#include "imp_ar_decompress.h"
#include "imp_ar_dof.h"
#include "imp_ar_carini.h"

static GSList *ar_read_directory(G3DStream *stream);
static G3DObject *ar_load_subfile(G3DContext *context, G3DModel *model,
	G3DStream *stream, const gchar *subfile);

/*****************************************************************************/
/* plugin interface                                                          */

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer plugin_data)
{
	GSList *dir, *item;
	GHashTable *carini;
	G3DMaterial *material;
	G3DObject *object;
	gchar *mname;
	G3DFloat x, y, z;

	setlocale(LC_NUMERIC, "C");

	/* default material */
	material = g3d_material_new();
	material->name = g_strdup("default material");
	model->materials = g_slist_append(model->materials, material);

	if(g_strcasecmp(stream->uri + (strlen(stream->uri) - 4), ".dof") == 0) {
		/* single DOF file */
		ar_dof_load(context, model, stream);
	} else {
		/* compressed AR archive */
		carini = ar_carini_load();

		/* load directory */
		dir = ar_read_directory(stream);

		/* decompress files */
		for(item = dir; item != NULL; item = item->next)
			ar_decompress_to_file(stream, (ArDirEntry *)item->data);

		/* load body */
		mname = g_hash_table_lookup(carini, "body.model.file");
		ar_load_subfile(context, model, stream, mname);
		/* steering wheel */
		mname = g_hash_table_lookup(carini, "steer.model.file");
		if(mname != NULL)
		{
			printf("D: steering wheel (%s)\n", mname);
			object = ar_load_subfile(context, model, stream, mname);
			ar_carini_get_position(carini, "steer", &x, &y, &z);
			object->transformation = g_new0(G3DTransformation, 1);
			g3d_matrix_identity(object->transformation->matrix);
			g3d_matrix_translate(x, y, z, object->transformation->matrix);
		}

		/* load wheel 0 */
		mname = g_hash_table_lookup(carini, "wheel0~wheel_front.model.file");
		if(mname != NULL)
		{
			printf("D: loading wheel 0 (%s)\n", mname);
			object = ar_load_subfile(context, model, stream, mname);
			x = ar_carini_get_float(carini, "susp0~susp_front.x");
			y = ar_carini_get_float(carini, "susp_front.y") -
				ar_carini_get_float(carini, "wheel_front.radius");
			z = ar_carini_get_float(carini, "susp_front.z");
			object->transformation = g_new0(G3DTransformation, 1);
			g3d_matrix_identity(object->transformation->matrix);
			g3d_matrix_translate(x, y, z, object->transformation->matrix);
		}

		ar_carini_free(carini);
	}

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("Racer models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("ar:dof", ":", 0);
}

/*****************************************************************************/

static GSList *ar_read_directory(G3DStream *stream)
{
	ArDirEntry *dirent;
	GSList *list = NULL;
	guint32 fsize, dpos;
	gint32 nbytes;
	gchar buffer[128];

	g3d_stream_seek(stream, -4, G_SEEK_END);
	fsize = g3d_stream_tell(stream);
	dpos = g3d_stream_read_int32_le(stream);

	/* start of directory */
	g3d_stream_seek(stream, dpos, G_SEEK_SET);
	nbytes = fsize - dpos;
#if DEBUG > 0
	printf("D: AR: directory @ 0x%08x, %d bytes\n", dpos, nbytes);
#endif

	while(nbytes > 0) {
		dirent = g_new0(ArDirEntry, 1);
		list = g_slist_append(list, dirent);

		nbytes -= g3d_stream_read_cstr(stream, buffer, 127);
		dirent->name = g_strdup(buffer);
		dirent->offset = g3d_stream_read_int32_le(stream);
		dirent->size = g3d_stream_read_int32_le(stream);
		nbytes -= 8;

#if DEBUG > 0
		printf("D: AR: * %s @ 0x%08x, %d bytes\n",
			dirent->name, dirent->offset, dirent->size);
#endif
	}

	return list;
}

static G3DObject *ar_load_subfile(G3DContext *context, G3DModel *model,
	G3DStream *stream, const gchar *subfile)
{
	G3DStream *substream;
	G3DObject *o;

	if(subfile == NULL)
		return NULL;

	substream = g3d_stream_open_file(subfile, "rb");
	if(substream == NULL)
		return NULL;

	o = ar_dof_load(context, model, substream);
	g3d_stream_close(substream);

	return o;
}

