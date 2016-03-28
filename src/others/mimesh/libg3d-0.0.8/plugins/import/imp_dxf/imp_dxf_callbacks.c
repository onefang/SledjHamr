/* $Id:$ */

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
#include <string.h>
#include <math.h>

/* need a way to detect non-set float values */
#ifndef FP_NAN
#	ifdef HUGE
#		define FP_NAN HUGE
#	else
#		define FP_NAN 3.40282347e+38F
#	endif
#endif

#include <g3d/face.h>
#include <g3d/object.h>
#include <g3d/matrix.h>
#include <g3d/primitive.h>

#include "imp_dxf.h"
#include "imp_dxf_callbacks.h"
#include "imp_dxf_vars.h"
#include "imp_dxf_def.h"
#include "imp_dxf_prop.h"
#include "imp_dxf_color.h"

static gboolean dxf_str_in_array(gchar **array, const gchar *needle)
{
	gchar **p = array;
	while(*p != NULL) {
		if(strcmp(*p, needle) == 0)
			return TRUE;
		p ++;
	}
	return FALSE;
}

#define DXF_VAR_DEBUG 2

gboolean dxf_debug_var(DxfGlobalData *global, DxfLocalData *local)
{
	gint32 key;
	gint16 i16;
	gchar str[DXF_MAX_LINE + 1], strval[DXF_MAX_LINE + 1];
	gdouble x, y, z;

	dxf_read_string(global, str);
	if(dxf_str_in_array(dxf_vars_vector3d, str)) {
		key = dxf_read_code(global); /* 10 */
		x = dxf_read_float64(global);
		key = dxf_read_code(global); /* 20 */
		y = dxf_read_float64(global);
		key = dxf_read_code(global); /* 30 */
		z = dxf_read_float64(global);
#if DEBUG > DXF_VAR_DEBUG
		g_debug("DXF: [v3d] %s: %.2f, %.2f, %.2f", str, x, y, z);
#endif
	} else if(dxf_str_in_array(dxf_vars_vector2d, str)) {
		key = dxf_read_code(global); /* 10 */
		x = dxf_read_float64(global);
		key = dxf_read_code(global); /* 20 */
		y = dxf_read_float64(global);
#if DEBUG > DXF_VAR_DEBUG
		g_debug("DXF: [v2d] %s: %.2f, %.2f", str, x, y);
#endif
	} else {
		key = dxf_read_code(global);
		switch(key) {
			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				/* string */
				dxf_read_string(global, strval);
#if DEBUG > DXF_VAR_DEBUG
				g_debug("DXF: [str] %s: %s", str, strval);
#endif
				break;
			case 40:
			case 50:
				x = dxf_read_float64(global);
#if DEBUG > DXF_VAR_DEBUG
				g_debug("DXF: [dbl] %s: %.2f", str, x);
#endif
				break;
			case 62:
			case 70:
			case 280:
			case 290: /* FIXME: boolean */
			case 370:
			case 380:
				/* 16-bit integer */
				i16 = dxf_read_int16(global);
#if DEBUG > DXF_VAR_DEBUG
				g_debug("DXF: [i16] %s: %d", str, i16);
#endif
				break;
			default:
				DXF_HANDLE_UNKNOWN(global, key, strval, "** VARIABLE **");
				break;
		}
	}
	return TRUE;
}

static inline void dxf_object_append(DxfGlobalData *global,
	DxfLocalData *local, G3DObject *object)
{
	if(local->edata->block)
		local->edata->block->objects = g_slist_append(
			local->edata->block->objects, object);
	else
		global->model->objects = g_slist_append(global->model->objects,
			object);
}

gboolean dxf_e_3DFACE(DxfGlobalData *global, DxfLocalData *local)
{
	G3DObject *object;
	G3DFace *face;
	G3DMaterial *material;
	gint32 key, i, j, col;
	gboolean quad;

	col = dxf_prop_get_int(local->eprop, 62, 254);
	material = dxf_color_get_material(global->model, col);
	if(material == NULL)
		material = local->edata->material;

	object = g_slist_nth_data(global->model->objects, 0);
	local->edata->object = object;
	local->edata->polyline_flags = 0;

	quad = (dxf_prop_get_dbl(local->eprop, 13, FP_NAN) != FP_NAN);

	face = g_new0(G3DFace, 1);
	face->material = material;
	face->vertex_count = quad ? 4 : 3;
	face->vertex_indices = g_new0(guint32, face->vertex_count);
	local->edata->vertex_offset = object->vertex_count;
	for(i = 0; i < face->vertex_count; i ++)
		face->vertex_indices[i] = local->edata->vertex_offset + i;

	object->vertex_count += face->vertex_count;
	object->vertex_data = g_realloc(object->vertex_data,
		object->vertex_count * 3 * sizeof(G3DFloat));
	object->faces = g_slist_prepend(object->faces, face);

	for(i = 0; i < face->vertex_count; i ++) {
		for(j = 0; j < 3; j ++) {
			key = (j + 1) * 10 + i;
			object->vertex_data[(local->edata->vertex_offset + i) * 3 + j] =
				dxf_prop_get_dbl(local->eprop, key, 0.0);
#if DEBUG > 2
			g_debug("|   3DFACE: data[%d = o + %d * 3 + %d] = prop[%d] = %.2f",
				(local->edata->vertex_offset + i) * 3 + j,
				i, j, key,
				object->vertex_data[
					(local->edata->vertex_offset + i) * 3 + j]);
#endif
		}
	}
	return TRUE;
}

gboolean dxf_e_BLOCK(DxfGlobalData *global, DxfLocalData *local)
{
	G3DObject *object;
	const gchar *name;

	name = dxf_prop_get_str(local->eprop, 2, NULL);
	if(local->sid == DXF_ID_BLOCKS) {
		object = g_new0(G3DObject, 1);
		object->hide = TRUE;
		object->name = name ? g_strdup(name) :
			g_strdup_printf("unnamed block @ line %d",
				g3d_stream_line(global->stream));
		local->edata->block = object;
		global->model->objects = g_slist_append(global->model->objects,
			object);
		if(name)
			g_hash_table_insert(global->blocks, object->name, object);
	}
	return TRUE;
}

gboolean dxf_e_ENDBLK(DxfGlobalData *global, DxfLocalData *local)
{
	local->edata->block = NULL;
	return TRUE;
}

gboolean dxf_e_INSERT(DxfGlobalData *global, DxfLocalData *local)
{
	G3DObject *block, *object, *subobject;
	GSList *item;
	const gchar *name;
	G3DFloat matrix[16];

	name = dxf_prop_get_str(local->eprop, 2, "*** error ***");
	block = g_hash_table_lookup(global->blocks, name);
	if(block) {
		object = g_new0(G3DObject, 1);
		object->name = g_strdup_printf("copy of %s", name);
		for(item = block->objects; item != NULL; item = item->next) {
			subobject = g3d_object_duplicate(item->data);
			object->objects = g_slist_append(object->objects, subobject);
		}
		global->model->objects = g_slist_append(global->model->objects,
			object);
		local->edata->object = object;
		local->edata->vertex_offset = 0;

		/* scale */
		g3d_matrix_identity(matrix);
		g3d_matrix_scale(
			dxf_prop_get_dbl(local->eprop, 41, 1.0),
			dxf_prop_get_dbl(local->eprop, 42, 1.0),
			dxf_prop_get_dbl(local->eprop, 43, 1.0),
			matrix);
		g3d_object_transform(object, matrix);

		/* rotate */
		g3d_matrix_identity(matrix);
		g3d_matrix_rotate(
			dxf_prop_get_dbl(local->eprop, 50, 0.0),
			dxf_prop_get_dbl(local->eprop, 210, 0.0),
			dxf_prop_get_dbl(local->eprop, 220, 0.0),
			dxf_prop_get_dbl(local->eprop, 230, 1.0),
			matrix);
		g3d_object_transform(object, matrix);

		/* translate */
		g3d_matrix_identity(matrix);
		g3d_matrix_translate(
			dxf_prop_get_dbl(local->eprop, 10, 0.0),
			dxf_prop_get_dbl(local->eprop, 20, 0.0),
			dxf_prop_get_dbl(local->eprop, 30, 0.0),
			matrix);
		g3d_object_transform(object, matrix);
	}
	return TRUE;
}

gboolean dxf_e_POLYLINE(DxfGlobalData *global, DxfLocalData *local)
{
	G3DObject *object = NULL;
	G3DMaterial *material;
	guint32 flags;
	gint32 m, n, col;

	col = dxf_prop_get_int(local->eprop, 62, 254);
	material = dxf_color_get_material(global->model, col);
	if(material == NULL)
		material = local->edata->material;

	flags = dxf_prop_get_int(local->eprop, 70, 0);
	if(flags & DXF_POLY_POLYFACE) {
		object = g_new0(G3DObject, 1);
		object->name = g_strdup_printf("POLYFACE @ line %d",
			g3d_stream_line(global->stream));
		object->vertex_count = dxf_prop_get_int(local->eprop, 71, 0);
		object->vertex_data = g_new0(G3DFloat, 3 * object->vertex_count);
	} else if(flags & DXF_POLY_3D_POLYMESH) {
		m = dxf_prop_get_int(local->eprop, 71, 0);
		n = dxf_prop_get_int(local->eprop, 72, 0);
		object = g3d_primitive_mesh(n, m,
			(flags & DXF_POLY_CLOSED),
			(flags & DXF_POLY_N_CLOSED),
			material);
		object->name = g_strdup_printf("3D POLYMESH %d x %d @ line %d",
			m, n, g3d_stream_line(global->stream));
	}

	if(object)
		dxf_object_append(global, local, object);

	local->edata->object = object;
	local->edata->vertex_offset = 0;
	local->edata->polyline_flags = flags;
	local->edata->tmp_i1 = 0;
	return TRUE;
}

gboolean dxf_e_VERTEX(DxfGlobalData *global, DxfLocalData *local)
{
	G3DObject *object = local->edata->object;
	G3DFace *face;
	G3DMaterial *material;
	guint32 index, flags;
	gint32 i, col;

	if(object == NULL)
		return TRUE;

	index = local->edata->vertex_offset + local->edata->tmp_i1;

	if(local->edata->polyline_flags & DXF_POLY_3D_POLYMESH) {
		g_return_val_if_fail(index < object->vertex_count, FALSE);
		for(i = 0; i < 3; i ++)
			object->vertex_data[index * 3 + i] = dxf_prop_get_dbl(local->eprop,
				10 * (i + 1), 0.0);
		local->edata->tmp_i1 ++;
	} else if(local->edata->polyline_flags & DXF_POLY_POLYFACE) {
		flags = dxf_prop_get_int(local->eprop, 70, 0);
		if(flags & 64) { /* vertex coords */
			g_return_val_if_fail(index < object->vertex_count, FALSE);
			for(i = 0; i < 3; i ++)
				object->vertex_data[index * 3 + i] = dxf_prop_get_dbl(
					local->eprop, 10 * (i + 1), 0.0);
			local->edata->tmp_i1 ++;
		}
		if(flags & 128) {
			col = dxf_prop_get_int(local->eprop, 62, 254);
			material = dxf_color_get_material(global->model, col);
			if(material == NULL)
				material = local->edata->material;

			face = g_new0(G3DFace, 1);
			face->material = material;
			face->vertex_count =
				dxf_prop_get_int(local->eprop, 74, 0) ? 4 : 3;
			face->vertex_indices = g_new0(guint32, face->vertex_count);
			for(i = 0; i < face->vertex_count; i ++)
				face->vertex_indices[i] =
					MAX(ABS(dxf_prop_get_int(local->eprop, 71 + i, 0)) - 1, 0);
			object->faces = g_slist_prepend(object->faces, face);
		}
	}

	return TRUE;
}
