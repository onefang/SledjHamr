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

/*
 *  imp_lwo.c - LightWave import plugin
 *
 *  (C) 2005,2006 Markus Dahms
 *
 *  based on gtkglarea example viewlw:
 *  Copyright (C) 1998 Janne Löf <jlof@mail.student.oulu.fi>
 */

#include <string.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/iff.h>

#define LW_MAX_POINTS   200
#define LW_MAX_NAME_LEN 500
#define LW_F_LWO2 1

#include "imp_lwo.h"
#include "imp_lwo_chunks.h"

/*****************************************************************************/
/* plugin interface                                                          */
/*****************************************************************************/

static void lwo_fix_texfaces(G3DModel *model);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	LwoObject *obj;
	G3DMaterial *material;
	guint32 id;
	gsize len;
	G3DIffGlobal *global;
	G3DIffLocal *local;

	if(!g3d_iff_check(stream, &id, &len))
		return FALSE;

	if((id != G3D_IFF_MKID('L','W','O','B')) &&
		(id != G3D_IFF_MKID('L','W','O','2'))) {
		g_warning("'%s' is not a LightWave object", stream->uri);
		return FALSE;
	}

	obj = g_new0(LwoObject, 1);

	global = g_new0(G3DIffGlobal, 1);
	global->stream = stream;
	global->context = context;
	global->model = model;
	if(id == G3D_IFF_MKID('L','W','O','2'))
		global->flags |= LWO_FLAG_LWO2;
	global->user_data = obj;

	local = g_new0(G3DIffLocal, 1);
	local->id = id;
	local->nb = len;

	material = g3d_material_new();
	material->name = g_strdup("fallback material");
	model->materials = g_slist_append(model->materials, material);

	g3d_iff_read_ctnr(global, local, lwo_chunks,
		G3D_IFF_PAD2 | G3D_IFF_SUBCHUNK_LEN16);

	lwo_fix_texfaces(model);

	/* cleanup */
	if(obj->ntags)
		g_strfreev(obj->tags);

	if(obj->nclips)
	{
		g_free(obj->clips);
		g_strfreev(obj->clipfiles);
	}

	if(obj->tex_vertices)
		g_free(obj->tex_vertices);

	g_free(obj);

	g_free(local);
	g_free(global);

	g3d_context_update_progress_bar(context, 0.0, FALSE);

	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("LightWave models.\n"
		"Author: Markus Dahms.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("lwo:lwb:lw", ":", 0);
}

/*****************************************************************************/
/* private                                                                   */
/*****************************************************************************/

G3DObject *lwo_create_object(G3DStream *stream, G3DModel *model, guint32 flags)
{
	G3DObject *object = g_new0(G3DObject, 1);
	object->name = g_strdup_printf("LWO%c object @ 0x%08x",
		(flags & LW_F_LWO2) ? '2' : 'B', (guint32)g3d_stream_tell(stream) - 8);
	model->objects = g_slist_append(model->objects, object);

#if 0
	/* LWO files should have correct faces */
	model->glflags &= ~G3D_FLAG_GL_ALLTWOSIDE;
#endif

	return object;
}

/*****************************************************************************/
/* LWO specific                                                              */
/*****************************************************************************/

gint lwo_read_string(G3DStream *stream, char *s)
{
	gint c;
	gint cnt = 0;
	do {
		c = g3d_stream_read_int8(stream);
		if (cnt < LW_MAX_NAME_LEN)
		  s[cnt] = c;
		else
		  s[LW_MAX_NAME_LEN-1] = 0;
		cnt++;
	} while (c != 0);
	/* if length of string (including \0) is odd skip another byte */
	if (cnt%2) {
		g3d_stream_read_int8(stream);
		cnt++;
	}
	return cnt;
}

guint32 lwo_read_vx(G3DStream *stream, guint *index)
{
	*index = g3d_stream_read_int16_be(stream);
	if((*index & 0xFF00) == 0xFF00) {
		*index <<= 16;
		*index += g3d_stream_read_int16_be(stream);
		*index &= 0x00FFFFFF;
		return 4;
	} else {
		return 2;
	}
}

static void lwo_fix_texfaces(G3DModel *model)
{
	GSList *olist, *flist;
	G3DObject *object;
	G3DFace *face;

	olist = model->objects;
	while(olist)
	{
		object = (G3DObject *)olist->data;
		olist = olist->next;

		flist = object->faces;
		while(flist)
		{
			face = (G3DFace *)flist->data;
			flist = flist->next;

			if(face->flags & G3D_FLAG_FAC_TEXMAP)
			{
				face->tex_image = face->material->tex_image;
				if(face->tex_image == NULL)
				{
					face->flags &= ~G3D_FLAG_FAC_TEXMAP;
				}
			}
		}
	}
}
