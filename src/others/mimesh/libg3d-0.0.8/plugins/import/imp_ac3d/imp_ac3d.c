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
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <glib.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/debug.h>

#define AC3D_FLAG_ACC    0x01

struct ac3d_transform {
	G3DFloat offx, offy, offz;
};

static gint32 ac3d_read_object(G3DStream *stream, G3DContext *context,
	G3DModel *model, gchar *line, struct ac3d_transform *transform,
	guint32 flags, GSList **objectlist, gint32 *rowcnt, guint32 level);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	struct ac3d_transform *transform;
	gchar buffer[2049], namebuf[257], *filename;
	guint32 version, ti1, flags = 0;
	G3DMaterial *material;
	G3DFloat tf1, tf2, tf3, tf4, tf5, tf6, trans;
	gint32 rowcnt = 0;

	setlocale(LC_NUMERIC, "C");

	rowcnt ++;
	g3d_stream_read_line(stream, buffer, 2048);

	if(strncmp(buffer, "AC3D", 4) != 0)
	{
		g_warning("file is not a AC3D model");
		return FALSE;
	}

	filename = g3d_stream_get_uri(stream);
	if(g_ascii_strcasecmp(filename + strlen(filename) - 4, ".acc") == 0)
	{
#if DEBUG > 0
		g_debug("AC3D: .acc file");
#endif
		flags |= AC3D_FLAG_ACC;
	}

	version = strtoul(buffer + 4, NULL, 16);

#if DEBUG > 0
	g_debug("AC3D: version %d", version);
#endif

	while(g3d_stream_read_line(stream, buffer, 2048))
	{
		rowcnt ++;
		if(strncmp(buffer, "MATERIAL", 8) == 0)
		{
			material = g3d_material_new();
			if(sscanf(buffer,
				"MATERIAL %s "
				"rgb " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
				"amb " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
				"emis " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
				"spec %f %f %f "
				"shi %u "
				"trans " G3D_SCANF_FLOAT,
				namebuf,
				&(material->r), &(material->g), &(material->b),
				&tf1, &tf2, &tf3,
				&tf4, &tf5, &tf6,
				&(material->specular[0]),
				&(material->specular[1]),
				&(material->specular[2]),
				&ti1,
				&trans) != 15)
			{
				g_warning("AC3D: error reading material line (%s)", buffer);
			}

			material->name = g_strdup(namebuf);
			material->a = 1.0 - trans;

			model->materials = g_slist_append(model->materials, material);
			g_debug("\\(0) Material (line %d)", rowcnt);
		}
		else if(strncmp(buffer, "OBJECT", 6) == 0)
		{
			transform = g_new0(struct ac3d_transform, 1);
			g_debug("\\(0) Object (line %d)", rowcnt);
			ac3d_read_object(stream, context, model, buffer, transform, flags,
				&(model->objects), &rowcnt, 1);
		}
		else
		{
#if DEBUG > 0
			g_warning("AC3D: unhandled line: %s", buffer);
#endif
		}
	}

	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("AC3D models.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("ac:acc", ":", 0);
}

/*
 * AC3D specific
 */

static gchar *ac3d_remove_quotes(gchar *text)
{
	if(text[0] == '"')
		return g_strndup(text + 1, strlen(text) - 2);
	else
		return g_strdup(text);
}

/**
 * ac3d_read_object:
 * returns: number of objects (including sub-objects) read, 0 in case of
 *          error.
 */

static gint32 ac3d_read_object(G3DStream *stream, G3DContext *context,
	G3DModel *model, gchar *line, struct ac3d_transform *parent_transform,
	guint32 flags, GSList **objectlist, gint32 *rowcnt, guint32 level)
{
	struct ac3d_transform *transform;
	G3DObject *object;
	G3DMaterial *material = NULL;
	G3DFace *face;
	gchar buffer[2049], namebuf[257];
	guint32 nkids, ti1, i, surf_flags, surf_done;
	guint32 i1, i2, i3;
	G3DFloat u1, u2, u3, v1, v2, v3;
	G3DFloat locx = 0.0, locy = 0.0, locz = 0.0;
	G3DFloat texrepu = 1.0, texrepv = 1.0, texoffu = 0.0, texoffv = 0.0;
	G3DFloat texscaleu = 1.0, texscalev = 1.0;
	G3DFloat crease = 0.0;
	guint32 len, facecnt = 0;
	gchar *filename;
	gint32 kidsread, objectcount = 0;
	G3DFloat pcnt, prev_pcnt = 0.0;

	if(sscanf(line, "OBJECT %s", namebuf) != 1)
	{
		g_warning("AC3D: error reading object line (%s)", line);
	}

	transform = g_new0(struct ac3d_transform, 1);
	memcpy(transform, parent_transform, sizeof(struct ac3d_transform));

	object = g_new0(G3DObject, 1);
	objectcount ++;
	*(objectlist) = g_slist_append(*(objectlist), object);

	while(g3d_stream_read_line(stream, buffer, 2048))
	{
		*rowcnt += 1;
		if(strncmp(buffer, "kids", 4) == 0)
		{
			/* final line of object */
			if(sscanf(buffer, "kids %u", &nkids) != 1)
			{
				g_warning("AC3D: error reading kids line (%s)", buffer);
				return 0;
			}
			for(i = 0; i < nkids; i ++)
			{
				/* read kids */
				*rowcnt += 1;
				g3d_stream_read_line(stream, buffer, 2048);
#if DEBUG > 0
				g_debug("\\%s(%d) Object (line %d)", debug_pad(level),
					level, *rowcnt);
#endif
				kidsread = ac3d_read_object(stream, context, model, buffer,
					transform, flags, &(object->objects), rowcnt, level + 1);
				objectcount += kidsread;
			}

#if DEBUG > 0
			g_debug("AC3D: \"%s\": %d sub-objects read",
				object->name ? object->name : "unnamed",
				objectcount - 1);
#endif

			if(crease > 0.0)
			{
				/* doesn't really work */
#if 0
				g3d_object_smooth(object, crease);
#endif
			}
			return objectcount;
		}
		else if(strncmp(buffer, "name", 4) == 0)
		{
			if(sscanf(buffer, "name %s", namebuf) != 1)
			{
				g_warning("AC3D: error reading name line (%s)", buffer);
			}
			else
			{
				object->name = g_strdup(namebuf);
			}
		}
		else if(strncmp(buffer, "loc", 3) == 0)
		{
			if(sscanf(buffer, "loc " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &locx, &locy, &locz) != 3)
			{
				g_warning("AC3D: error reading loc line (%s)", buffer);
				locx = locy = locz = 0.0;
			}

			transform->offx += locx;
			transform->offy += locy;
			transform->offz += locz;
		}
		else if(strncmp(buffer, "numvert", 7) == 0)
		{
			if(sscanf(buffer, "numvert %u", &(object->vertex_count)) != 1)
			{
				g_warning("AC3D: error reading numvert line (%s)", buffer);
				object->vertex_count = 0;
			}
			else
			{
				object->vertex_data =
					g_new0(G3DFloat, object->vertex_count * 3);
				for(i = 0; i < object->vertex_count; i ++)
				{
					if(g3d_stream_read_line(stream, buffer, 2048))
					{
						*rowcnt += 1;
						if(sscanf(buffer, G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT,
							&(object->vertex_data[i * 3 + 0]),
							&(object->vertex_data[i * 3 + 1]),
							&(object->vertex_data[i * 3 + 2])) != 3)
						{
							g_warning("AC3D: error reading vertex (%s)",
								buffer);
						}

						object->vertex_data[i * 3 + 0] += transform->offx;
						object->vertex_data[i * 3 + 1] += transform->offy;
						object->vertex_data[i * 3 + 2] += transform->offz;
					}
				}
			}
			/* END numvert */
		}
		else if(strncmp(buffer, "numsurf", 7) == 0)
		{
			/* ignore for now */
		}
		else if(strncmp(buffer, "SURF", 4) == 0)
		{
			surf_done = 0;

			if(sscanf(buffer, "SURF %x", &surf_flags) != 1)
			{
				g_warning("AC3D: error reading surf (%s)", buffer);
			}

			while(!surf_done)
			{
				if(!g3d_stream_read_line(stream, buffer, 2048))
					return 0;
				*rowcnt += 1;

				if(sscanf(buffer, "refs %u", &ti1) == 1)
				{
					if(!(flags & AC3D_FLAG_ACC))
					{
						face = g_new0(G3DFace, 1);
						face->vertex_count = ti1;
						face->vertex_indices =
							g_new0(guint32, face->vertex_count);
						face->material = material;

						face->tex_image = object->tex_image;
						if(face->tex_image)
						{
							face->flags |= G3D_FLAG_FAC_TEXMAP;
						}

						face->tex_vertex_count = ti1;
						face->tex_vertex_data =
							g_new0(G3DFloat, 2 * face->tex_vertex_count);

						/* normal face */
						for(i = 0; i < face->vertex_count; i ++)
						{
							if(!g3d_stream_read_line(stream, buffer, 2048))
								return 0;
							*rowcnt += 1;

							if(sscanf(buffer, "%u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT,
								&(face->vertex_indices[i]),
								&(face->tex_vertex_data[i * 2 + 0]),
								&(face->tex_vertex_data[i * 2 + 1])) != 3)
							{
								g_warning(
									"AC3D: error reading vertex index (%s)",
									buffer);
							}
							face->tex_vertex_data[i * 2 + 0] *=
								(texrepu * texscaleu);
							face->tex_vertex_data[i * 2 + 1] *=
								(texrepv * texscalev);

							face->tex_vertex_data[i * 2 + 0] += texoffu;
							face->tex_vertex_data[i * 2 + 1] += texoffv;

#if 0
							face->tex_coords[i * 2 + 0] *= texscaleu;
							face->tex_coords[i * 2 + 1] *= texscalev;
#endif
						}

						if(face->material && (face->vertex_count >= 3))
							object->faces =
								g_slist_prepend(object->faces, face);

					} /* not .acc */
					else
					{
						/* triangle stripes */
						i = 0;
						while(i < ti1)
						{
							face = g_new0(G3DFace, 1);
							face->vertex_count = 3;
							face->vertex_indices = g_new0(guint32, 3);

							face->material = material;

							face->tex_image = object->tex_image;
							if(face->tex_image)
							{
								face->flags |= G3D_FLAG_FAC_TEXMAP;
							}

							face->tex_vertex_count = 3;
							face->tex_vertex_data =
								g_new0(G3DFloat, 2 * face->tex_vertex_count);

							if(i == 0)
							{
								/* TODO: error handling */
								g3d_stream_read_line(stream, buffer, 2048);
								sscanf(buffer, "%u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &i1, &u1, &v1);
								g3d_stream_read_line(stream, buffer, 2048);
								sscanf(buffer, "%u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &i2, &u2, &v2);
								g3d_stream_read_line(stream, buffer, 2048);
								sscanf(buffer, "%u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &i3, &u3, &v3);

								*rowcnt += 3;
								i += 3;
							}
							else
							{
								/* TODO: error handling */
								i1 = i2;
								u1 = u2;
								v1 = v2;
								i2 = i3;
								u2 = u3;
								v2 = v3;

								g3d_stream_read_line(stream, buffer, 2048);
								*rowcnt += 1;
								sscanf(buffer, "%u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &i3, &u3, &v3);

								i ++;
							}

							face->vertex_indices[0] = i1;
							face->tex_vertex_data[0] = u1;
							face->tex_vertex_data[1] = v1;
							if(facecnt % 2) {
								face->vertex_indices[1] = i3;
								face->vertex_indices[2] = i2;
								face->tex_vertex_data[2] = u3;
								face->tex_vertex_data[3] = v3;
								face->tex_vertex_data[4] = u2;
								face->tex_vertex_data[5] = v2;
							} else {
								face->vertex_indices[1] = i2;
								face->vertex_indices[2] = i3;
								face->tex_vertex_data[2] = u2;
								face->tex_vertex_data[3] = v2;
								face->tex_vertex_data[4] = u3;
								face->tex_vertex_data[5] = v3;
							}

							object->faces =
								g_slist_prepend(object->faces, face);
							facecnt ++;
						}
					} /* .acc */

					surf_done = 1;
				}
				else if(sscanf(buffer, "mat %u", &ti1) == 1)
				{
					material = g_slist_nth_data(model->materials, ti1);
				}
			}

			material = NULL;
			/* END SURF */
		}
		else if(strncmp(buffer, "texture", 7) == 0)
		{
			if(sscanf(buffer, "texture %s", namebuf) == 1)
			{
				filename = ac3d_remove_quotes(namebuf);
				object->tex_image = g3d_texture_load_cached(context, model,
					filename);
				if(object->tex_image)
				{
					g3d_texture_prepare(object->tex_image);

					texscaleu = object->tex_image->tex_scale_u;
					texscalev = object->tex_image->tex_scale_v;
				}
			}
			else
			{
				g_warning("error reading texture line (%s)", buffer);
			}
		}
		else if(strncmp(buffer, "texrep", 6) == 0)
		{
			if(sscanf(buffer, "texrep " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &texrepu, &texrepv) != 2)
			{
				g_warning("error reading texrep line (%s)", buffer);
				texrepu = 1.0;
				texrepv = 1.0;
			}

			if(texrepu == 0.0) texrepu = 1.0;
			if(texrepv == 0.0) texrepv = 1.0;
		}
		else if(strncmp(buffer, "texoff", 6) == 0)
		{
			if(sscanf(buffer, "texoff " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &texoffu, &texoffv) != 2)
			{
				g_warning("error reading texoff line (%s)", buffer);
			}
		}
		else if(strncmp(buffer, "crease", 6) == 0)
		{
			if(sscanf(buffer, "crease " G3D_SCANF_FLOAT, &crease) != 1)
			{
				g_warning("AC3D: error reading crease line (%i): %s",
					*rowcnt, buffer);
			}
		}
		else if(strncmp(buffer, "data ", 5) == 0)
		{
			/* object data */
			if(sscanf(buffer, "data %u", &len) != 1)
			{
				g_warning("AC3D: error in data line (%i): %s",
					*rowcnt, buffer);
			}
			/* object data on next line */
			g3d_stream_read_line(stream, buffer, 2048);
			*rowcnt += 1;
		}
		else
		{
#if DEBUG > 0
			g_warning("AC3D: unhandled line %d: %s", *rowcnt, buffer);
#endif
		}

		pcnt = (G3DFloat)g3d_stream_tell(stream) /
				(G3DFloat)g3d_stream_size(stream);
		if((pcnt - prev_pcnt) > 0.005) {
			prev_pcnt = pcnt;
			g3d_context_update_progress_bar(context, pcnt, TRUE);
		}
	}

	/* cleanup */
	g_free(transform);

	return 0;
}
