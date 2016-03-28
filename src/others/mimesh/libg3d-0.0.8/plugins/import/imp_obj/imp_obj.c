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
#include <errno.h>
#include <locale.h>

#include <g3d/context.h>
#include <g3d/types.h>
#include <g3d/material.h>
#include <g3d/stream.h>

#define OBJ_USE_GROUPING 0

typedef struct {
	goffset goff;
	goffset ooff;
	G3DObject *object;
} ObjGroupOffset;

static gboolean obj_tryloadmat(G3DModel *model, const gchar *filename);
static G3DMaterial *obj_usemat(G3DModel *model, const gchar *matname);

static G3DObject *obj_object_by_name(G3DModel *model, const gchar *name);
#if OBJ_USE_GROUPING
static G3DObject *obj_get_offset(GSList *group_list, guint32 *voffp,
	guint32 index, G3DObject *defobj);
#endif

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	gchar line[2048], matname[128], matfile[1024];
	gchar *filename;
	G3DObject *object = NULL;
	G3DMaterial *material = NULL;
	G3DFloat pcnt, prev_pcnt = 0.0;
	gdouble x,y,z;
	guint32 num_v, v_off = 1, v_cnt = 0;
#if OBJ_USE_GROUPING
	gchar oname[128];
	ObjGroupOffset *grpoff;
	GSList *group_list = NULL;
#endif
	goffset global_vertex_count = 0;

	setlocale(LC_NUMERIC, "C");
	filename = g3d_stream_get_uri(stream);

	strncpy(matfile, filename, strlen(filename) - 3);
	matfile[strlen(filename)-3] = '\0';
	strcat(matfile, "mtl");
	obj_tryloadmat(model, matfile);

	object = obj_object_by_name(model, "(default)");

	while(!g3d_stream_eof(stream))
	{
		memset(line, 0, 2048);
		g3d_stream_read_line(stream, line, 2048);
		/* remove leading and trailing whitespace characters */
		g_strstrip(line);
		if(strlen(line) > 0) {
			switch(line[0]) {
				case '#':
					continue;
					break;

				case 'g': /* group */
#if OBJ_USE_GROUPING
					if(strlen(line) == 1)
						strcpy(oname, "(default)");
					else
						sscanf(line, "g %s", oname);

					material = obj_usemat(model, oname);

					grpoff = g_new0(ObjGroupOffset, 1);
					grpoff->object = obj_object_by_name(model, oname);
					grpoff->goff = global_vertex_count;
					grpoff->ooff = grpoff->object->vertex_count;
					group_list = g_slist_append(group_list, grpoff);
#if DEBUG > 0
					g_debug("[g] 0x%08x / 0x%08x: \"%s\"",
						(guint32)grpoff->goff,
						(guint32)grpoff->ooff, grpoff->object->name);
#endif
					object = grpoff->object;
					v_cnt = grpoff->ooff;
#endif
					break;

				case 'l': /* line */
					break;

				case 'o': /* object */
					break;

				case 'v': /* vertex */
					if(strncmp(line, "vn ", 3) == 0)
					{
						/* normal ? */
					}
					else if(strncmp(line, "vt ", 3) == 0)
					{
						/* ?? */
					}
					else if(sscanf(line, "v %lf %lf %lf", &x, &y, &z) == 3)
					{
						object->vertex_count ++;
						object->vertex_data = g_realloc(object->vertex_data,
							object->vertex_count * 3 * sizeof(G3DFloat));
						object->vertex_data[v_cnt * 3 + 0] = x;
						object->vertex_data[v_cnt * 3 + 1] = y;
						object->vertex_data[v_cnt * 3 + 2] = z;

						v_cnt ++;
						global_vertex_count ++;
					}
					else g_warning("parse error in line: %s", line);
					break;

				case 'f': /* face */
					if(strncmp("f ", line, 2) == 0)
					{
						G3DFace *face;
						gchar **vertex, **vstrs = g_strsplit(line, " ", 0);
						int i;

						num_v = 0;
						face = g_new0(G3DFace, 1);
						if(material != NULL)
							face->material = material;
						else face->material =
							g_slist_nth_data(object->materials, 0);

						/* find number of vertices in line */
						vertex = vstrs;
						while(*vertex != NULL) { num_v++; vertex++; }
						face->vertex_count = num_v - 1;

						/* next one if # of vertices < 3 */
						if(face->vertex_count < 3)
							continue;

						/* calculate object-local vertex offset, indices
						 * in .obj files are absolute */
						i = strtol(vstrs[1], NULL, 10);
#if OBJ_USE_GROUPING
						object = obj_get_offset(group_list, &v_off,
							(i < 0) ? global_vertex_count - i - 1 : i,
							object);
#else
						v_off = 0;
#endif
						if(object == NULL) {
							g_warning("error: face before object");
							return FALSE;
						}

						/* read vertices */
						face->vertex_indices = g_new0(guint32, num_v - 1);
						for(i = 1; i < num_v; i ++) {
							gint32 index = strtol(vstrs[i], NULL, 10);

							if(index < 0)
								face->vertex_indices[i - 1] =
									global_vertex_count + index + v_off - 1;
							else
								face->vertex_indices[i - 1] = MIN(
									(index - 1) + v_off,
									object->vertex_count - 1);
						}
						g_strfreev(vstrs);
						object->faces = g_slist_prepend(object->faces, face);
					}
					else
						g_warning("parse error in line: %s", line);
					break;

				case 'u': /* usemat? */
				case 'm':
				case 's':
					if(sscanf(line, "usemtl %s", matname) == 1) {
						material = obj_usemat(model, matname);
					} else if(sscanf(line, "mtllib %s", matfile) == 1) {
						/* loads external material library */
						if(obj_tryloadmat(model, matfile) != TRUE)
							g_warning("error loading material library '%s'",
								matfile);
					}
					break;
				default:
#if DEBUG > 0
					g_debug("unknown type of line: %s", line);
#endif
					break;
			}
		}

#if 1
		pcnt = (G3DFloat)g3d_stream_tell(stream) /
			(G3DFloat)g3d_stream_size(stream);
		if((pcnt - prev_pcnt) > 0.01) {
			prev_pcnt = pcnt;
			g3d_context_update_progress_bar(context, pcnt, TRUE);
		}
#endif
		g3d_context_update_interface(context);
	} /* !eof(stream) */
	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("Maya .obj models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("obj", ":", 0);
}

/*****************************************************************************/

/*****************************************************************************/
/* material file ops                                                         */
/*****************************************************************************/

int obj_tryloadmat(G3DModel *model, const char *filename)
{
	FILE *f;
	G3DMaterial *material = NULL;

	f = fopen(filename, "r");
	if(f == NULL) {
#if DEBUG > 1
		g_warning("obj_tryloadmat: loading '%s' failed: %s", filename,
			strerror(errno));
#endif
		return FALSE;
	}
#if DEBUG > 0
	g_debug("loading material library %s", filename);
#endif
	while(!feof(f)) {
		char line[2048];
		float r,g,b, t1,t2, ni;
		int tf, ns, il;

		fgets(line, 2048, f);
		g_strstrip(line);
		if(strlen(line))
		{
			char mname[128];

			if(line[0] == '#') continue;   /* comments */
			if(line[0] == '\n') continue;  /* empty lines */

			if(sscanf(line, "newmtl %s", mname) == 1)
			{
				/* new material */
				material = g3d_material_new();
				material->name = g_strdup(mname);
				model->materials = g_slist_append(model->materials, material);
			}
			else if(sscanf(line, " Kd %f %f %f", &r, &g, &b) == 3)
			{
				/* material color? */
				if(material != NULL)
				{
					material->r = r;
					material->g = g;
					material->b = b;
				}
			}
			else if(sscanf(line, " Ks %f %f %f", &r, &g, &b) == 3)
			{
				/* ?? */
			}
			else if(sscanf(line, " Tf %f %f %d", &t1, &t2, &tf) == 3)
			{
				/* transparency ?? */
				if(material != NULL)
				{
					if(tf == 1) material->a = 1.0 - t1;
				}
			}
			else if(sscanf(line, " Ns %d Ni %f", &ns, &ni) == 2)
			{
				/* ?? */
			}
			else if(sscanf(line, " illum %d", &il) == 1)
			{
				/* ?? */
			}
			else {
#if DEBUG > 0
				g_warning("unknown type of line: %s", line);
#endif
			}
		}
	} /* !feof */
	return TRUE;
}

G3DMaterial *obj_usemat(G3DModel *model, const gchar *matname)
{
	/* sets new active material from named list */
	GSList *mlist = model->materials;
	while(mlist != NULL)
	{
		G3DMaterial *mat = (G3DMaterial*)mlist->data;
		if(strcmp(matname, mat->name) == 0)
		{
			return mat;
		}
		mlist = mlist->next;
	}

	return NULL;
}

static G3DObject *obj_object_by_name(G3DModel *model, const gchar *name)
{
	G3DObject *object;
	G3DMaterial *material;
	GSList *oitem;

#if DEBUG > 4
	g_debug("looking for object '%s'", name);
#endif

	for(oitem = model->objects; oitem != NULL; oitem = oitem->next) {
		object = oitem->data;
		if(strcmp(object->name, name) == 0)
			return object;
	}

	material = g3d_material_new();
	material->name = g_strdup("(default material)");

	object = g_new0(G3DObject, 1);
	object->name = g_strdup(name);
	object->materials = g_slist_append(object->materials, material);
	model->objects = g_slist_append(model->objects, object);

	return object;
}

#if OBJ_USE_GROUPING
static G3DObject *obj_get_offset(GSList *group_list, guint32 *voffp,
	guint32 index, G3DObject *defobj)
{
	GSList *leitem, *gitem;
	ObjGroupOffset *grpoff;

	for(leitem = gitem = group_list; gitem != NULL; gitem = gitem->next) {
		grpoff = gitem->data;

		/* this one is too big */
		if(grpoff->goff > index) {
			grpoff = leitem->data;
			*voffp = grpoff->ooff - grpoff->goff;
#if DEBUG > 0
			g_debug("[o]: i=%-6d, go=%-6d, oo=%-6d, vo=%-6d (%s, %d vtxs)",
				index, (guint32)grpoff->goff, (guint32)grpoff->ooff, *voffp,
				grpoff->object->name, grpoff->object->vertex_count);
#endif
			return grpoff->object;
		}
		leitem = gitem;
	}

	*voffp = 0;
	return defobj;
}
#endif

