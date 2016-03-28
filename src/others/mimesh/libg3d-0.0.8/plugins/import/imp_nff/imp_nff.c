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
#include <math.h>
#include <locale.h>

#include <g3d/g3d.h>
#include <g3d/stream.h>

/*****************************************************************************/
/* plugin interface                                                          */
/*****************************************************************************/

#define NFF_SEC_NOSECTION	  0
#define NFF_SEC_VIEWPOINT	  1
#define NFF_SEC_BGCOLOR		  2
#define NFF_SEC_POSLIGHT	  3
#define NFF_SEC_MATERIAL	  4
#define NFF_SEC_CONECYL		  5
#define NFF_SEC_SPHERE		  6
#define NFF_SEC_POLYGON		  7
#define NFF_SEC_POLPATCH	  8

static gboolean nff_readline(G3DStream *stream, gchar *line, guint32 maxlen);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer plugin_data)
{
	gchar line[1024];
	G3DObject *object;
	G3DMaterial *material = NULL;
	G3DFace *face;
	guint32 section = NFF_SEC_NOSECTION;
	G3DFloat r,g,b, Kd, Ks, Sh, T, refr;
	G3DFloat v1,v2,v3, n1,n2,n3;
	gint32 i, num, index;
	gchar name[128];

	setlocale(LC_NUMERIC, "C");

	g_return_val_if_fail(model != NULL, FALSE);

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("NFF Object");
	model->objects = g_slist_append(model->objects, object);

	while(nff_readline(stream, line, 1024)) {
		if(strcmp(line, "v") == 0) {
			section = NFF_SEC_VIEWPOINT;
		} else if(sscanf(line, "b " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &r, &g, &b) == 3) {
			g3d_context_set_bgcolor(context, r, g, b, 1.0);
		} else if(sscanf(line, "f " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT,
			&r, &g, &b, &Kd, &Ks, &Sh, &T, &refr) == 8) {
			material = g3d_material_new();
			material->r = r;
			material->g = g;
			material->b = b;
			material->a = 1.0 - T;
			material->shininess = Sh;
			material->specular[0] = r * Ks;
			material->specular[1] = g * Ks;
			material->specular[2] = b * Ks;
			if(T > 0) material->flags |= G3D_FLAG_MAT_TWOSIDE;
			object->materials = g_slist_append(object->materials, material);
			g_snprintf(name, 128, "material #%d",
				g_slist_length(object->materials));
			material->name = g_strdup(name);
		} else if((sscanf(line, "pp %d", &num) == 1) ||
			(sscanf(line, "p %d", &num) == 1)) {
			face = g_new0(G3DFace, 1);

			if(strncmp(line, "pp", 2) == 0) section = NFF_SEC_POLPATCH;
			else section = NFF_SEC_POLYGON;

			object->vertex_count += num;
			object->vertex_data = g_realloc(object->vertex_data,
				object->vertex_count * 3 * sizeof(G3DFloat));

			face->material = material;
			face->vertex_count = num;
			face->vertex_indices = g_malloc0(num * sizeof(guint32));
			object->faces = g_slist_prepend(object->faces, face);
			if(section == NFF_SEC_POLPATCH) {
				face->flags |= G3D_FLAG_FAC_NORMALS;
				face->normals = g_malloc0(num * 3 * sizeof(G3DFloat));
			}
			/* most faces are in this direction, but there are wrong models */
			for(i = num - 1; i >= 0; i --) {
				if(!nff_readline(stream, line, 1024)) {
					g_warning("reading vertices failed");
					return FALSE;
				}
				if((section == NFF_SEC_POLPATCH) &&
					 (sscanf(line,  G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT,
						&v1,&v2,&v3,
						&n1,&n2,&n3) == 6)) {
					index = object->vertex_count - num + i;
					object->vertex_data[index*3+0] = v1;
					object->vertex_data[index*3+1] = v2;
					object->vertex_data[index*3+2] = v3;
					face->vertex_indices[i] = index;
					face->normals[i*3+0] = -n1;
					face->normals[i*3+1] = -n2;
					face->normals[i*3+2] = -n3;
				} else if(sscanf(line,  G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &v1,&v2,&v3) == 3) {
					index = object->vertex_count - num + i;
					object->vertex_data[index*3+0] = v1;
					object->vertex_data[index*3+1] = v2;
					object->vertex_data[index*3+2] = v3;
					face->vertex_indices[i] = index;
				} else
					g_warning("error in line '%s'", line);
			}
		}
	}
	return TRUE;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("Neutral File Format models.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	return g_strsplit("nff", ":", 0);
}

/*****************************************************************************/
/* private functions                                                         */
/*****************************************************************************/

static gboolean nff_readline(G3DStream *stream, gchar *line, guint32 maxlen)
{
	if(g3d_stream_read_line(stream, line, maxlen) == NULL)
		return FALSE;
	g_strstrip(line);

	if((strlen(line) == 0) || (line[0] == '#')) {
		/* get next line if empty or comment */
		return nff_readline(stream, line, maxlen);
	}
	return TRUE;
}

