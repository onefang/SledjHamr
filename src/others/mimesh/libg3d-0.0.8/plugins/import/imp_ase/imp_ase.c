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

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/texture.h>
#include <g3d/model.h>
#include <g3d/matrix.h>

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model)
{
	gchar line[2048], tmp[128], *s;
	guint32 i, j, a, b, c, ab, bc, ca, mtlid, glid = 0, tvertcnt = 0, lnum = 0;
	G3DVector x, y, z, *tverts = NULL;
	G3DObject *object = NULL;
	G3DMaterial *material;
	G3DFace *face = NULL;
	G3DMatrix rmatrix[16];

	setlocale(LC_NUMERIC, "C");

	while(g3d_stream_read_line(stream, line, 2048)) {
		lnum ++;
		g_strstrip(line);

#if DEBUG > 4
		g_debug("ASE: %s", line);
#endif

		if(strncmp(line, "*GEOMOBJECT ", 12) == 0)
		{
			object = g_new0(G3DObject, 1);
			object->name = g_strdup("(unnamed object)");

			model->objects = g_slist_append(model->objects, object);
		}
		else if(strncmp(line, "*MATERIAL ", 10) == 0)
		{
			material = g3d_material_new();
			material->name = g_strdup_printf("material %d",
				g_slist_length(model->materials));
			model->materials = g_slist_append(model->materials, material);

			if(sscanf(line, "*MATERIAL %u \"%s", &i, tmp) == 2)
			{
				tmp[strlen(tmp) - 1] = '\0';
#if DEBUG > 2
				g_debug("ASE: material file: %s", tmp);
#endif
				/* TODO: parse .fx file */
				s = g_strdup_printf("%.*s.jpg", ((int) strlen(tmp)) - 3, tmp);
				material->tex_image =
					g3d_texture_load_cached(context, model, s);
				if(material->tex_image)
					material->tex_image->tex_id = ++ glid;
				g_free(s);
			}
		}
		else if(strncmp(line, "*NODE_NAME ", 11) == 0)
		{
			if(object)
			{
				if(object->name)
					g_free(object->name);

				object->name = g_strdup(line + 11);
			}
		}
		else if(strncmp(line, "*MESH_NUMVERTEX ", 16) == 0)
		{
			if(sscanf(line, "*MESH_NUMVERTEX %u", &i) == 1)
			{
				if(object)
				{
					object->vertex_count = i;
					object->vertex_data = g_new0(G3DFloat, i * 3);
				}
			}
		}
		else if(strncmp(line, "*MESH_VERTEX ", 13) == 0)
		{
			if(sscanf(line, "*MESH_VERTEX %u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &i, &x, &y, &z) == 4)
			{
				if(object && (i < object->vertex_count))
				{
					object->vertex_data[i * 3 + 0] = x;
					object->vertex_data[i * 3 + 1] = y;
					object->vertex_data[i * 3 + 2] = z;
				}
			}
		}
		else if(strncmp(line, "*MESH_FACE ", 11) == 0)
		{
			if(object && (sscanf(line, "*MESH_FACE %u: A: %u B: %u C: %u "
				"AB: %u BC: %u CA: %u *MESH_SMOOTHING %s *MESH_MTLID %u",
				&i, &a, &b, &c, &ab, &bc, &ca, tmp, &mtlid) == 9))
			{
				face = g_new0(G3DFace, 1);
				face->vertex_count = 3;
				face->vertex_indices = g_new0(guint32, 3);
				face->vertex_indices[0] = a;
				face->vertex_indices[1] = b;
				face->vertex_indices[2] = c;
				face->material = g_slist_nth_data(model->materials, mtlid);
				if(face->material == NULL)
					face->material = g_slist_nth_data(model->materials, 0);
				if(face->material == NULL) {
					face->material = g3d_material_new();
					face->material->name = g_strdup("(fallback material)");
					model->materials = g_slist_append(model->materials,
						face->material);
				}

				object->faces = g_slist_append(object->faces, face);
				face = NULL;
			}
		}
		else if(strncmp(line, "*MESH_FACENORMAL ", 17) == 0)
		{
			if(object && (sscanf(line, "*MESH_FACENORMAL %u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT,
				&i, &x, &y, &z) == 4))
			{
				face = g_slist_nth_data(object->faces, i);
				if(face)
				{
					face->flags |= G3D_FLAG_FAC_NORMALS;
					face->normals = g_new0(G3DFloat, 3 * 3);
					for(j = 0; j < 3; j ++)
					{
						face->normals[j * 3 + 0] = x;
						face->normals[j * 3 + 1] = y;
						face->normals[j * 3 + 2] = z;
					}
				}
			}
		}
		else if(strncmp(line, "*MESH_VERTEXNORMAL ", 19) == 0)
		{
			if(face && face->normals && (sscanf(line,
				"*MESH_VERTEXNORMAL %u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &i, &x, &y, &z) == 4))
			{
				for(j = 0; j < 3; j ++)
				{
					if(face->vertex_indices[j] == i)
					{
						face->normals[j * 3 + 0] = x;
						face->normals[j * 3 + 1] = y;
						face->normals[j * 3 + 2] = z;
					}
				}
			}
		}
		else if(strncmp(line, "*MESH_NUMTVERTEX ", 17) == 0)
		{
			/* clear old tvertex data */
			if(tverts)
			{
				g_free(tverts);
				tvertcnt = 0;
			}

			if(sscanf(line, "*MESH_NUMTVERTEX %u", &tvertcnt) == 1)
				tverts = g_new0(G3DFloat, tvertcnt * 2);
		}
		else if(strncmp(line, "*MESH_TVERT ", 12) == 0)
		{
			if(sscanf(line, "*MESH_TVERT %u " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &i, &x, &y, &z) == 4)
			{
				if(i < tvertcnt)
				{
					tverts[i * 2 + 0] = x;
					tverts[i * 2 + 1] = y;
				}
			}
		}
		else if(strncmp(line, "*MESH_TFACE ", 12) == 0)
		{
			if(object && (sscanf(line, "*MESH_TFACE %u %u %u %u",
				&i, &a, &b, &c) == 4))
			{
				face = g_slist_nth_data(object->faces, i);
				if(face && face->material->tex_image &&
					(a < tvertcnt) && (b < tvertcnt) && (c < tvertcnt))
				{
					face->flags |= G3D_FLAG_FAC_TEXMAP;
					face->tex_image = face->material->tex_image;
					face->tex_vertex_count = 3;
					face->tex_vertex_data = g_new0(G3DFloat, 2 * 3);
					face->tex_vertex_data[0 * 2 + 0] = tverts[a * 2 + 0];
					face->tex_vertex_data[0 * 2 + 1] = 1.0 - tverts[a * 2 + 1];
					face->tex_vertex_data[1 * 2 + 0] = tverts[b * 2 + 0];
					face->tex_vertex_data[1 * 2 + 1] = 1.0 - tverts[b * 2 + 1];
					face->tex_vertex_data[2 * 2 + 0] = tverts[c * 2 + 0];
					face->tex_vertex_data[2 * 2 + 1] = 1.0 - tverts[c * 2 + 1];
				}
			}
		}
		if((lnum % 10) == 0)
			g3d_context_update_progress_bar(context,
				(G3DFloat)g3d_stream_tell(stream) /
				(G3DFloat)g3d_stream_size(stream), TRUE);
	} /* read line */

	/* clean up */
	if(tverts) {
		g_free(tverts);
		tvertcnt = 0;
	}
	g3d_context_update_progress_bar(context, 0.0, FALSE);

	g3d_matrix_identity(rmatrix);
	g3d_matrix_rotate_xyz(G_PI * -90.0 / 180, 0.0, 0.0, rmatrix);
	g3d_model_transform(model, rmatrix);

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("ASCII Scene Exporter models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("ase", ":", 0);
}

