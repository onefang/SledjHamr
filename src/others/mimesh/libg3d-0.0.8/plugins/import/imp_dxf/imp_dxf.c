/* $Id: imp_dxf.c 256 2008-09-04 12:02:58Z mmmaddd $ */

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
#include <locale.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/model.h>
#include <g3d/matrix.h>

#include "imp_dxf.h"
#include "imp_dxf_section.h"
#include "imp_dxf_def.h"

static void dxf_cleanup(DxfGlobalData *global);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	gchar binmagic[22];
	G3DObject *object;
	G3DMaterial *material;
	DxfGlobalData *global;
	G3DMatrix rmatrix[16];

	global = g_new0(DxfGlobalData, 1);
	global->context = context;
	global->model = model;
	global->stream = stream;
	global->blocks = g_hash_table_new(g_str_hash, g_str_equal);

	setlocale(LC_NUMERIC, "C");

	if((g3d_stream_read(stream, binmagic, 22) == 22) &&
		 (strncmp(binmagic, "AutoCAD Binary DXF", 18) == 0))
		global->binary = TRUE;
	else {
		setlocale(LC_NUMERIC, "C");
		g3d_stream_seek(stream, 0, G_SEEK_SET);
	}

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("DXF Object");
	model->objects = g_slist_append(model->objects, object);

	material = g3d_material_new();
	material->name = g_strdup("default material");
	material->flags |= G3D_FLAG_MAT_TWOSIDE;
	object->materials = g_slist_append(object->materials, material);

	while(!g3d_stream_eof(stream)) {
		int retval = dxf_read_section(global, object);
		if(retval != TRUE) {
			if(retval == 0xE0F) {
				g3d_matrix_identity(rmatrix);
				g3d_matrix_rotate_xyz(G_PI * -90.0 / 180, 0.0, 0.0, rmatrix);
				g3d_model_transform(model, rmatrix);
				dxf_cleanup(global);
				return TRUE;
			}
			g_printerr("error in section..\n");
			dxf_cleanup(global);
			return FALSE;
		}
	}

	g3d_matrix_identity(rmatrix);
	g3d_matrix_rotate_xyz(G_PI * -90.0 / 180, 0.0, 0.0, rmatrix);
	g3d_model_transform(model, rmatrix);
	dxf_cleanup(global);

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("AutoCAD models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("dxf", ":", 0);
}

/*****************************************************************************/

static void dxf_cleanup(DxfGlobalData *global)
{
	g_hash_table_destroy(global->blocks);
	g_free(global);
}

gboolean dxf_read_section(DxfGlobalData *global, G3DObject *object)
{
	gint grpcode;
	gchar val_str[DXF_MAX_LINE + 1];

	grpcode = dxf_read_code(global);
	if(grpcode != 0) {
#if DEBUG > 0
		g_printerr("unexpected group code: %d (0 expected)\n", grpcode);
#endif
		return FALSE;
	}
	dxf_read_string(global, val_str);
	if(strcmp("EOF", val_str) == 0)
		return 0xE0F;
	if(strcmp("SECTION", val_str) != 0) {
#if DEBUG > 0
		g_printerr("SECTION expected, found: %s\n", val_str);
#endif
		return FALSE;
	}
	grpcode = dxf_read_code(global);
	if(grpcode != 2) {
#if DEBUG > 0
		g_printerr("unexpected group code: %d (2 expected)\n", grpcode);
#endif
		return FALSE;
	}
	dxf_read_string(global, val_str);

	if(strcmp(val_str, "HEADER") == 0)
		return dxf_section_HEADER(global);
	else if(strcmp(val_str, "TABLES") == 0)
		return dxf_section_TABLES(global);
	else if(strcmp(val_str, "ENTITIES") == 0)
		return dxf_section_ENTITIES(global);
	else if(strcmp(val_str, "BLOCKS") == 0)
		return dxf_section_BLOCKS(global);
	else if(strcmp(val_str, "OBJECTS") == 0)
		return dxf_section_OBJECTS(global);
	else if(strcmp(val_str, "CLASSES") == 0)
		return  dxf_section_CLASSES(global);
	else {
#if DEBUG > 0
		g_printerr("unknown section '%s', skipping...\n", val_str);
#endif
		dxf_skip_section(global);
	}
	return TRUE;
}

gchar *dxf_read_string(DxfGlobalData *global, gchar *value)
{
	if(global->binary) {
		int pos = 0;
		int c;
		do
		{
			c = g3d_stream_read_int8(global->stream);
			value[pos] = (char)c;
			pos++;
		} while(c != '\0');
		return value;
	} else {
		gchar line[DXF_MAX_LINE + 1];

		g3d_stream_read_line(global->stream, line, DXF_MAX_LINE);
		line[DXF_MAX_LINE] = '\0';
		if(sscanf(line, "%s", value) == 1)
			return g_strchomp(value);
		if(sscanf(line, " %s", value) == 1)
			return g_strchomp(value);
		return NULL;
	}
}

gint32 dxf_read_code(DxfGlobalData *global)
{
	gint32 val = DXF_CODE_INVALID;
	gchar line[DXF_MAX_LINE + 1];

	if(global->binary)
		return g3d_stream_read_int8(global->stream);
	else {
		g3d_stream_read_line(global->stream, line, DXF_MAX_LINE);
		if((sscanf(line, "%d", &val) != 1) &&
			(sscanf(line, " %d", &val) != 1))
			return DXF_CODE_INVALID;
		if(val == 999) { /* comment */
			g3d_stream_read_line(global->stream, line, DXF_MAX_LINE);
			return dxf_read_code(global);
		}
		return val;
	}
}

gint32 dxf_read_int16(DxfGlobalData *global)
{
	if(global->binary)
		return g3d_stream_read_int16_le(global->stream);
	else
	{
		gint32 val;
		gchar line[DXF_MAX_LINE];

		g3d_stream_read_line(global->stream, line, DXF_MAX_LINE);
		if(sscanf(line, "%i", &val) == 1)
			return val;
		if(sscanf(line, " %i", &val) == 1)
			return val;
		else
			return DXF_CODE_INVALID;
	}
}

gint32 dxf_read_int32(DxfGlobalData *global)
{
	if(global->binary)
		return g3d_stream_read_int32_le(global->stream);
	else
	{
		gint32 val;
		gchar line[DXF_MAX_LINE];

		g3d_stream_read_line(global->stream, line, DXF_MAX_LINE);
		if(sscanf(line, "%i", &val) == 1)
			return val;
		if(sscanf(line, " %i", &val) == 1)
			return val;
		else
			return DXF_CODE_INVALID;
	}
}

gdouble dxf_read_float64(DxfGlobalData *global)
{
	if(global->binary)
		return g3d_stream_read_double_le(global->stream);
	else
	{
		gdouble val;
		gchar line[DXF_MAX_LINE];

		g3d_stream_read_line(global->stream, line, DXF_MAX_LINE);
		if(sscanf(line, "%lf", &val) == 1)
			return val;
		if(sscanf(line, " %lf", &val) == 1)
			return val;
		else
			return 0.0;
	}
}

gboolean dxf_skip_section(DxfGlobalData *global)
{
	gchar c, buf[7], line[DXF_MAX_LINE];
	gsize read;

	while(!g3d_stream_eof(global->stream))
		if(global->binary) {
			do { c = g3d_stream_read_int8(global->stream); } while(c != 0);
			read = g3d_stream_read(global->stream, buf, 7);
			if((read == 7) && (strncmp(buf, "ENDSEC", 6) == 0))
				return TRUE;
			else
				g3d_stream_seek(global->stream, -read, G_SEEK_CUR);
		} else {
			g3d_stream_read_line(global->stream, line, DXF_MAX_LINE);
			if(strncmp(line, "ENDSEC", 6) == 0)
				return TRUE;
		}
	return TRUE;
}

