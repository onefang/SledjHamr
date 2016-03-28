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

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/matrix.h>
#include <g3d/object.h>

#include "imp_ldraw_types.h"
#include "imp_ldraw_part.h"
#include "imp_ldraw_library.h"
#include "imp_ldraw_color.h"
#include "imp_ldraw_misc.h"

static gboolean ldraw_part_parse_meta(G3DObject *object, gchar *buffer)
{
	if(object->name == NULL) {
		object->name = g_strdup(buffer);
		return TRUE;
	}
#if DEBUG > 2
	g_debug("META: %s", buffer);
#endif
	return TRUE;
}

static void ldraw_part_replace_material(G3DObject *object,
	G3DMaterial *material)
{
	GSList *item;
	G3DObject *sub;
	G3DFace *face;

	for(item = object->faces; item != NULL; item = item->next) {
		face = item->data;
		if(face->material == NULL)
			face->material = material;
	}
	for(item = object->objects; item != NULL; item = item->next) {
		sub = item->data;
		ldraw_part_replace_material(sub, material);
	}
}

static gboolean ldraw_part_parse_ref(G3DObject *object, gchar *buffer,
	LDrawLibrary *lib)
{
	G3DObject *subobj;
	G3DMaterial *material;
	G3DFloat m[16], x, y, z;
	guint32 colid;
	gchar fname[256], *strp;

	g3d_matrix_identity(m);
	memset(fname, 0, 256);

	if(sscanf(buffer, "%u "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " %255s", 
		&colid, &x, &y, &z,
		m + 0 * 4 + 0, m + 1 * 4 + 0, m + 2 * 4 + 0,
		m + 0 * 4 + 1, m + 1 * 4 + 1, m + 2 * 4 + 1,
		m + 0 * 4 + 2, m + 1 * 4 + 2, m + 2 * 4 + 2,
		fname) == 14) {

		strp = path_sep(fname);
		if(strp != NULL)
			strp[0] = G_DIR_SEPARATOR;

		subobj = ldraw_part_from_file(lib, fname);
		if(!subobj)
			subobj = ldraw_library_lookup(lib, fname);
		if(subobj != NULL) {
			g3d_object_transform(subobj, m);
			g3d_matrix_identity(m);
			g3d_matrix_translate(x, y, z, m);
			g3d_object_transform(subobj, m);
			if(colid != 16) {
				material = ldraw_color_lookup(lib, colid);
				ldraw_part_replace_material(subobj, material);
			}
			object->objects = g_slist_append(object->objects, subobj);
			return TRUE;
		}
	}
#if DEBUG > 1
	g_warning("LDraw: failed to parse/process reference: %s", buffer);
#endif
	return FALSE;
}

static gboolean ldraw_part_parse_tri(G3DObject *object, gchar *buffer,
	LDrawLibrary *lib)
{
	guint32 off, colid;
	G3DFace *face;

	off = object->vertex_count;
	object->vertex_count += 3;
	object->vertex_data = g_realloc(object->vertex_data,
		object->vertex_count * 3 * sizeof(G3DFloat));
	if(sscanf(buffer, "%u "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, 
		&colid,
		object->vertex_data + (off + 0) * 3 + 0,
		object->vertex_data + (off + 0) * 3 + 1,
		object->vertex_data + (off + 0) * 3 + 2,
		object->vertex_data + (off + 1) * 3 + 0,
		object->vertex_data + (off + 1) * 3 + 1,
		object->vertex_data + (off + 1) * 3 + 2,
		object->vertex_data + (off + 2) * 3 + 0,
		object->vertex_data + (off + 2) * 3 + 1,
		object->vertex_data + (off + 2) * 3 + 2) == 10) {

		face = g_new0(G3DFace, 1);
		face->material = ldraw_color_lookup(lib, colid);
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		face->vertex_indices[0] = off + 0;
		face->vertex_indices[1] = off + 1;
		face->vertex_indices[2] = off + 2;
		object->faces = g_slist_append(object->faces, face);
		return TRUE;
	}
	return FALSE;
}

static gboolean ldraw_part_parse_quad(G3DObject *object, gchar *buffer,
	LDrawLibrary *lib)
{
	guint32 off, colid;
	G3DFace *face;

	off = object->vertex_count;
	object->vertex_count += 4;
	object->vertex_data = g_realloc(object->vertex_data,
		object->vertex_count * 3 * sizeof(G3DFloat));
	if(sscanf(buffer, "%u "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " "
		G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, 
		&colid,
		object->vertex_data + (off + 0) * 3 + 0,
		object->vertex_data + (off + 0) * 3 + 1,
		object->vertex_data + (off + 0) * 3 + 2,
		object->vertex_data + (off + 1) * 3 + 0,
		object->vertex_data + (off + 1) * 3 + 1,
		object->vertex_data + (off + 1) * 3 + 2,
		object->vertex_data + (off + 2) * 3 + 0,
		object->vertex_data + (off + 2) * 3 + 1,
		object->vertex_data + (off + 2) * 3 + 2,
		object->vertex_data + (off + 3) * 3 + 0,
		object->vertex_data + (off + 3) * 3 + 1,
		object->vertex_data + (off + 3) * 3 + 2) == 13) {

		face = g_new0(G3DFace, 1);
		face->material = ldraw_color_lookup(lib, colid);
		face->vertex_count = 4;
		face->vertex_indices = g_new0(guint32, 4);
		face->vertex_indices[0] = off + 0;
		face->vertex_indices[1] = off + 1;
		face->vertex_indices[2] = off + 2;
		face->vertex_indices[3] = off + 3;
		object->faces = g_slist_append(object->faces, face);
		return TRUE;
	}
	return FALSE;
}

static inline G3DObject *ldraw_part_open_file(LDrawLibrary *lib,
	const gchar *filename)
{
	LDrawPart *part;

	part = g_new0(LDrawPart, 1);
	part->name = g_strdup(filename);
	part->stream = g3d_stream_open_file(filename, "r");
	if(part->stream == NULL) {
		g_free(part);
		return NULL;
	}
	part->object = ldraw_part_get_object(part, lib);
	g3d_stream_close(part->stream);
	part->stream = NULL;

	ldraw_library_insert(lib, part->name, part);

	return part->object;
}

G3DObject *ldraw_part_from_file(LDrawLibrary *lib, const gchar *filename)
{
	G3DObject *object;
	gchar *path;

	if(g_file_test(filename, G_FILE_TEST_EXISTS))
		return ldraw_part_open_file(lib, filename);
	path = g_ascii_strdown(filename, -1);
	if(g_file_test(path, G_FILE_TEST_EXISTS)) {
		object = ldraw_part_open_file(lib, path);
		g_free(path);
		return object;
	}
	g_free(path);
	return NULL;
}

G3DObject *ldraw_part_get_object(LDrawPart *part, LDrawLibrary *lib)
{
	G3DObject *object;
	G3DMaterial *material;
	gchar buffer[1024];

	object = g_new0(G3DObject, 1);
	material = g3d_material_new();
	material->name = g_strdup("default material");
	object->materials = g_slist_append(object->materials, material);

	while(!g3d_stream_eof(part->stream)) {
		memset(buffer, 0, 1024);
		g3d_stream_read_line(part->stream, buffer, 1023);
		g_strstrip(buffer);
		if(strlen(buffer) == 0)
			continue;
		switch(buffer[0] - 0x30) {
			case 0: /* meta data */
				ldraw_part_parse_meta(object, buffer + 2);
				break;
			case 1: /* sub-file reference */
				ldraw_part_parse_ref(object, buffer + 2, lib);
				break;
			case 2: /* line */
				break;
			case 3: /* triangle */
				ldraw_part_parse_tri(object, buffer + 2, lib);
				break;
			case 4: /* quadrilateral */
				ldraw_part_parse_quad(object, buffer + 2, lib);
				break;
			case 5: /* optional line */
				break;
			default:
				g_warning("LDraw: unknown type of line: %s", buffer);
				break;
		}
		if(part->master) {
			g3d_context_update_progress_bar(lib->context,
				(G3DFloat)g3d_stream_tell(part->stream) /
				(G3DFloat)g3d_stream_size(part->stream), TRUE);
			g3d_context_update_interface(lib->context);
		}
	}

	return object;
}

void ldraw_part_free(LDrawPart *part)
{
	if(part->stream)
		g3d_stream_close(part->stream);
	if(part->filename)
		g_free(part->filename);
	g_free(part->name);
	g_free(part);
}
