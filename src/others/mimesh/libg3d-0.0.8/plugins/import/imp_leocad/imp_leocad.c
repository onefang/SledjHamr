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
#include <locale.h>

#include <math.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/model.h>
#include <g3d/read.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>

#include "imp_leocad_library.h"

static int leocad_load_lcd(G3DStream *stream, G3DModel *model,
	LeoCadLibrary *library, G3DContext *context);

EAPI
gpointer plugin_init(G3DContext *context)
{
	LeoCadLibrary *library;
	const gchar *libdir;

	libdir = g_getenv("LEOCAD_LIB");
	if(libdir == NULL)
		libdir = "/usr/local/share/leocad";

	library = leocad_library_load(libdir);

	if(library == NULL)
	{
#if DEBUG > 1
		g_warning("LeoCAD: failed to load library");
#endif
		return NULL;
	}

	return library;
}

EAPI
void plugin_cleanup(gpointer user_data)
{
	LeoCadLibrary *library;

#if DEBUG > 1
	g_debug("LeoCAD: cleaning up library\n");
#endif

	library = (LeoCadLibrary *)user_data;

	if(library)
		leocad_library_free(library);
}

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	LeoCadLibrary *library;

	library = (LeoCadLibrary *)user_data;

	if(library == NULL)
	{
		g_warning("LeoCAD: library not loaded");
		return FALSE;
	}

	setlocale(LC_NUMERIC, "C");

	return leocad_load_lcd(stream, model, library, context);
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("LeoCAD models.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("lcd", ":", 0);
}

/*
 * LeoCAD specific stuff
 */

static gboolean leocad_change_key(guint16 ktime, G3DFloat *param, guint8 ktype,
	G3DFloat *matrix, G3DFloat *mloc, gboolean *valid_matrix)
{
	/* get first frame */
	if(ktime == 1)
	{
		switch(ktype)
		{
			case 0x00: /* translation */
				g3d_matrix_identity(mloc);
				g3d_matrix_translate(param[0], param[1], param[2], mloc);
				g3d_matrix_multiply(matrix, mloc, matrix);
				*valid_matrix = TRUE;
				break;

			case 0x01: /* rotation */
				g3d_matrix_rotate((G3DFloat)param[3] * G_PI / 180.0,
					param[0], param[1], param[2], matrix);
				g3d_matrix_multiply(mloc, matrix, matrix);
				*valid_matrix = TRUE;
				break;

			default:
				break;
		}
	}
#if DEBUG > 0
	g_debug("LeoCAD: key 0x%02x (%d): %+2.2f %+2.2f %+2.2f %+2.2f",
		ktype, ktime, param[0], param[1], param[2], param[3]);
#endif
	return TRUE;
}

static gboolean leocad_load_lcd_piece(G3DStream *stream, G3DModel *model,
	LeoCadLibrary *library, G3DFloat lcdversion)
{
	guint32 i, j, k, nkeys, nobjs;
	guint16 ktime;
	guint8 pver, over, ktype, color = 0, len8;
	gchar name[9];
	G3DFloat param[4], matrix[16], mloc[16];
	G3DFloat offx = 0.0, offy = 0.0, offz = 0.0;
	G3DFloat rotx = 0.0, roty = 0.0, rotz = 0.0;
	G3DObject *object;
	G3DMaterial *mat_change;
	G3DFace *face;
	GSList *fitem;
	gboolean valid_matrix = FALSE;

	g3d_matrix_identity(mloc);
	g3d_matrix_identity(matrix);

	mat_change = leocad_library_get_nth_material(library, 0x10);

	if(lcdversion > 0.4)
	{
		pver = g3d_stream_read_int8(stream);

		if(pver >= 9)
		{
			/* object stuff */
			over = g3d_stream_read_int8(stream);
			nobjs = g3d_stream_read_int32_le(stream);
			for(i = 0; i < nobjs; i ++)
			{
				/* time */
				ktime = g3d_stream_read_int16_le(stream);
				/* param */
				param[0] = g3d_stream_read_float_le(stream);
				param[1] = g3d_stream_read_float_le(stream);
				param[2] = g3d_stream_read_float_le(stream);
				param[3] = g3d_stream_read_float_le(stream);
				/* type */
				ktype = g3d_stream_read_int8(stream);

				leocad_change_key(ktime, param, ktype, matrix, mloc,
					&valid_matrix);
			}

			if(over == 1)
			{
				nobjs = g3d_stream_read_int32_le(stream);
				for(i = 0; i < nobjs; i ++)
				{
					ktime = g3d_stream_read_int16_le(stream);
					param[0] = g3d_stream_read_float_le(stream);
					param[1] = g3d_stream_read_float_le(stream);
					param[2] = g3d_stream_read_float_le(stream);
					param[3] = g3d_stream_read_float_le(stream);
					ktype = g3d_stream_read_int8(stream);
				}
			}
		}
		else /* pver < 9 */
		{
			if(pver > 5)
			{
				nkeys = g3d_stream_read_int32_le(stream);
				for(i = 0; i < nkeys; i ++)
				{
					/* param */
					param[0] = g3d_stream_read_float_le(stream);
					param[1] = g3d_stream_read_float_le(stream);
					param[2] = g3d_stream_read_float_le(stream);
					param[3] = g3d_stream_read_float_le(stream);

					/* time */
					ktime = g3d_stream_read_int16_le(stream);

					/* type */
					ktype = g3d_stream_read_int8(stream);

					leocad_change_key(ktime, param, ktype, matrix, mloc,
						&valid_matrix);

				} /* keys */

				nkeys = g3d_stream_read_int32_le(stream);
				for(i = 0; i < nkeys; i ++)
				{
					/* param */
					param[0] = g3d_stream_read_float_le(stream);
					param[1] = g3d_stream_read_float_le(stream);
					param[2] = g3d_stream_read_float_le(stream);
					param[3] = g3d_stream_read_float_le(stream);

					/* time */
					ktime = g3d_stream_read_int16_le(stream);

					/* type */
					ktype = g3d_stream_read_int8(stream);
				}
			} /* pver > 5 */
			else /* pver <= 5 */
			{
				if(pver > 2)
				{
					nkeys = g3d_stream_read_int8(stream);
					for(i = 0; i < nkeys; i ++)
					{
						if(pver > 3)
						{
#if DEBUG > 2
							g_debug("LeoCAD: matrix\n");
#endif
							/* matrix */
							for(j = 0; j < 4; j ++)
								for(k = 0; k < 4; k ++)
									matrix[j * 4 + k] =
										g3d_stream_read_float_le(stream);

							valid_matrix = TRUE;
						}
						else
						{
							/* move: 3 x float */
							offx = g3d_stream_read_float_le(stream);
							offy = g3d_stream_read_float_le(stream);
							offz = g3d_stream_read_float_le(stream);

							/* rotate: 3 x float */
							rotx = g3d_stream_read_float_le(stream);
							roty = g3d_stream_read_float_le(stream);
							rotz = g3d_stream_read_float_le(stream);
						}

						/* time */
						ktime = g3d_stream_read_int8(stream);

						/* bl? */
						g3d_stream_read_int32_le(stream);
					} /* .. nkeys */
				} /* pver > 2 */
				else /* pver <= 2 */
				{
					/* move: 3 x float */
					offx = g3d_stream_read_float_le(stream);
					offy = g3d_stream_read_float_le(stream);
					offz = g3d_stream_read_float_le(stream);

					/* rotate: 3 x float */
					rotx = g3d_stream_read_float_le(stream);
					roty = g3d_stream_read_float_le(stream);
					rotz = g3d_stream_read_float_le(stream);
				}
			} /* pver <= 5 */
		} /* pver < 9 */

		/* common stuff */

		/* name of piece */
		g3d_stream_read(stream, name, 9);

		/* color */
		color = g3d_stream_read_int8(stream);

		if(pver < 5)
			color = leocad_library_convert_color(color);

#if DEBUG > 0
		g_debug("LeoCAD: [%d]: '%-8s', color 0x%02x", pver, name, color);
#endif

		/* step show */
		g3d_stream_read_int8(stream);

		/* step hide */
		if(pver > 1)
			g3d_stream_read_int8(stream);

		if(pver > 5)
		{
			/* frame show */
			g3d_stream_read_int16_le(stream);
			/* frame hide */
			g3d_stream_read_int16_le(stream);

			if(pver > 7) {
				/* state */
				g3d_stream_read_int8(stream);

				len8 = g3d_stream_read_int8(stream);
				g3d_stream_skip(stream, len8);
			} else { /* pver <= 7 */
				/* hide */
				g3d_stream_read_int32_le(stream);
				g3d_stream_skip(stream, 81);
			} /* pver <= 7 */

			if(pver > 6) {
				/* group pointer ?! */
				g3d_stream_read_int32_le(stream);
			}
		} /* pver > 5 */
		else /* pver <= 5 */
		{
			/* group pointer ?! */
			g3d_stream_read_int8(stream);

			/* hide */
			g3d_stream_read_int8(stream);
		}

	} /* lcdversion > 0.4 */

	object = leocad_library_get_piece(library, name);
	if(object == NULL)
	{
		g_warning("LeoCAD: failed to load piece '%s'", name);
		return FALSE;
	}

	/* matrix */
	if(!valid_matrix)
	{
		/* translation */
		g3d_matrix_identity(mloc);
		g3d_matrix_translate(offx, offy, offz, mloc);
		/* rotation */
		rotx = (G3DFloat)(rotx * G_PI) / 180.0;
		roty = (G3DFloat)(roty * G_PI) / 180.0;
		rotz = (G3DFloat)(rotz * G_PI) / 180.0;
		g3d_matrix_identity(matrix);
		g3d_matrix_rotate_xyz(rotx, roty, rotz, matrix);

		/* combine */
		g3d_matrix_multiply(mloc, matrix, matrix);
	}

	/*g3d_matrix_dump(matrix);*/

	/* transform vertices */
	for(i = 0; i < object->vertex_count; i ++)
		g3d_vector_transform(
			&(object->vertex_data[i * 3 + 0]),
			&(object->vertex_data[i * 3 + 1]),
			&(object->vertex_data[i * 3 + 2]),
			matrix);

	/* change color */
	fitem = object->faces;
	while(fitem)
	{
		face = (G3DFace *)fitem->data;
		if(face->material == mat_change)
		{
			face->material = leocad_library_get_nth_material(library, color);
		}

		if(face->material == NULL)
		{
			face->material = leocad_library_get_nth_material(library, 0);
		}

		fitem = fitem->next;
	}

	/* add to model object list */
	model->objects = g_slist_append(model->objects, object);

	return TRUE;
}

static gboolean leocad_load_lcd(G3DStream *stream, G3DModel *model,
	LeoCadLibrary *library, G3DContext *context)
{
	gchar magic[32];
	float version;
	guint32 i, count;
	G3DFloat r, g, b;
	G3DMatrix rmatrix[16];

	g3d_stream_read(stream, magic, 32);
	if(strncmp(magic, "LeoCAD", 6) != 0) {
		g_warning("LeoCAD: '%s' is not a valid LeoCAD project file",
			stream->uri);
		return FALSE;
	}

	sscanf(&magic[7], "%f", &version);

	if(version > 0.4) {
#if DEBUG > 0
		g_debug("LeoCAD: file version %.1f, getting next float", version);
#endif
		version = g3d_stream_read_float_le(stream);
	}

#if DEBUG > 0
	g_debug("LeoCAD: file version %.1f", version);
#endif

	r = g3d_stream_read_int8(stream) / 255.0;
	g = g3d_stream_read_int8(stream) / 255.0;
	b = g3d_stream_read_int8(stream) / 255.0;
	/* background color */
	g3d_context_set_bgcolor(context, r, g, b, 1.0);
	g3d_stream_read_int8(stream);

	/* view */
	if(version < 0.6) {
		/* eye: 3 x double */
		g3d_stream_skip(stream, 24);

		/* target: 3 x double */
		g3d_stream_skip(stream, 24);
	}

	/* angle snap */
	g3d_stream_read_int32_le(stream);
	/* snap */
	g3d_stream_read_int32_le(stream);
	/* line width */
	g3d_stream_read_float_le(stream);
	/* detail */
	g3d_stream_read_int32_le(stream);
	/* cur group */
	g3d_stream_read_int32_le(stream);
	/* cur color */
	g3d_stream_read_int32_le(stream);
	/* action */
	g3d_stream_read_int32_le(stream);
	/* cur step */
	g3d_stream_read_int32_le(stream);

	if(version > 0.8) {
		/* scene */
		g3d_stream_read_int32_le(stream);
	}

	/* piece count */
	count = g3d_stream_read_int32_le(stream);
	for(i = 0; i < count; i ++) {
		/* load piece */
		leocad_load_lcd_piece(stream, model, library, version);
	}

	g3d_matrix_identity(rmatrix);
	g3d_matrix_rotate_xyz(G_PI * -90.0 / 180, 0.0, 0.0, rmatrix);
	g3d_model_transform(model, rmatrix);

	return TRUE;
}
