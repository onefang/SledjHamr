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

#include <string.h>
#include <g3d/material.h>
#include <g3d/stream.h>
#include <g3d/debug.h>

#include "imp_max_callbacks.h"

static gchar *max_read_wchar(G3DStream *stream, guint32 n)
{
	gint32 i;
	gunichar2 *u16text;
	gchar *text;
	GError *error = NULL;

	u16text = g_new0(gunichar2, n + 1);
	for(i = 0; i < n; i ++) {
		u16text[i] = g3d_stream_read_int16_le(stream);
	}

	text = g_utf16_to_utf8(u16text, n, NULL, NULL, &error);
	if(error != NULL) {
		g_warning("UTF-16 to UTF-8 conversion failed: %s",
			error->message);
		g_error_free(error);
	}
	g_free(u16text);

	return text;
}

gboolean max_cb_debug_int32(MaxGlobalData *global, MaxLocalData *local)
{
	union {
		gint32 i;
		G3DFloat f;
	} u;

	while(local->nb >= 4) {
		u.i = g3d_stream_read_int32_le(global->stream);
		local->nb -= 4;
#if DEBUG > 0
		g_debug("|%s[D32] 0x%08x, %d, %.2f", debug_pad(local->level),
			u.i, u.i, u.f);
#endif
	}
	return TRUE;
}

gboolean max_cb_debug_wchars(MaxGlobalData *global, MaxLocalData *local)
{
	gchar *str;
	guint32 len;

	len = local->nb / 2;
	str = max_read_wchar(global->stream, len);
	local->nb -= len * 2;
#if DEBUG > 0
	g_debug("|%s[TEXT] %s (%d)", debug_pad(local->level),
		str, len);
#endif
	g_free(str);

	return TRUE;
}

gboolean max_cb_debug_string(MaxGlobalData *global, MaxLocalData *local)
{
	gchar *str;
	guint32 len;

	g_return_val_if_fail(local->nb >= 4, FALSE);
	len = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;
	if(len > local->nb)
		len = local->nb;

	str = g_new0(gchar, len + 1);
	g3d_stream_read(global->stream, str, len);
	local->nb -= len;
#if DEBUG > 0
	g_debug("|%s[TEXT] %s (%d)", debug_pad(local->level),
		str, len);
#endif
	g_free(str);

	return TRUE;
}

gboolean max_cb_0x0001_0x0005(MaxGlobalData *global, MaxLocalData *local)
{
	gchar *str;
	gint32 len, i, w3[3], cnt = 0;

	if(local->nb < 4)
		return FALSE;

	/* flags? */
	i = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;

#if DEBUG > 0
	g_debug("|%s[PROP] 0x%08x", debug_pad(local->level), i);
#endif

	while(local->nb > 0) {
		len = g3d_stream_read_int32_le(global->stream);
		local->nb -= 4;
		str = g_malloc0(len + 1);
		g3d_stream_read(global->stream, str, len);
		local->nb -= len;
		for(i = 0; i < 3; i ++)
			w3[i] = g3d_stream_read_int16_le(global->stream);
		local->nb -= 6;
		cnt ++;
#if DEBUG > 0
		g_debug("|%s[PROP]  %04d: '%s' (%d bytes) [%d, %d, %d]",
			debug_pad(local->level),
			cnt, str, len,
			w3[0], w3[1], w3[2]);
#endif
		g_free(str);
	}
	return TRUE;
}

gboolean max_cb_IDFILE_0x1201(MaxGlobalData *global, MaxLocalData *local)
{
	guint16 width, height;

	width = g3d_stream_read_int16_le(global->stream);
	height = g3d_stream_read_int16_le(global->stream);
	local->nb -= 4;
#if DEBUG > 0
	g_debug("|%s[IMG] %u x %u", debug_pad(local->level), width, height);
#endif
	return TRUE;
}

/* vertex data */
gboolean max_cb_0x08FE_0x0100(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 num;
	gint i, j;
	G3DObject *object;

	if(local->nb < 4)
		return FALSE;

	/* vertices */
	num = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;
	object = (G3DObject *)local->object;
	if(object == NULL) {
		g_warning("MAX: 0x08FE::0x0100: no object");
		return FALSE;
	}

#if DEBUG > 0
	g_debug("|%s[VERT] %d vertices", debug_pad(local->level), num);
#endif
	global->vertex_offset = object->vertex_count;
	object->vertex_count += num;
	object->vertex_data = g_realloc(object->vertex_data,
		object->vertex_count * 3 * sizeof(G3DFloat));

	for(i = 0; i < num; i ++) {
		if(local->nb < 16)
			return FALSE;
		g3d_stream_read_int32_le(global->stream); /* always 0 */
		for(j = 0; j < 3; j ++)
			object->vertex_data[(global->vertex_offset + i) * 3 + j] =
				g3d_stream_read_float_le(global->stream);
		local->nb -= 16;
	}
	return TRUE;
}

/* lines (vertex indices) */
gboolean max_cb_0x08FE_0x010A(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 num;
	gint i, j;
	guint32 v[3];

	if(local->nb < 4)
		return FALSE;

	num = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;

#if DEBUG > 0
	g_debug("|%s[LINE] %d lines", debug_pad(local->level), num);
#endif
	for(i = 0; i < num; i ++) {
		if(local->nb < 12)
			return FALSE;
		for(j = 0; j < 3; j ++) {
			v[j] = g3d_stream_read_int32_le(global->stream);
		}
		local->nb -= 12;

#if DEBUG > 1
		g_debug("|%s[LINE] 0x%08x: (%d => %d)", debug_pad(local->level),
			v[0], v[1], v[2]);
#endif
	}
	return TRUE;
}

/* polygon data */
gboolean max_cb_0x08FE_0x011A(MaxGlobalData *global, MaxLocalData *local)
{
	gint32 i;
	guint32 numpoly, type, numvert, cntpoly = 0;
	G3DObject *object = (G3DObject *)local->object;
	G3DFace *face;
	G3DMaterial *material;

	g_return_val_if_fail(local->nb >= 4, FALSE);
	g_return_val_if_fail(object != NULL, FALSE);

	material = (G3DMaterial *)g_slist_nth_data(global->model->materials,
		(global->vertex_offset ? 1 : 0));
	g_return_val_if_fail(material != NULL, FALSE);

	numpoly = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;
#if DEBUG > 0
	g_debug("|%s[POLY] %d polygons to read", debug_pad(local->level),
		numpoly);
#endif
	while(local->nb >= 4) {
		numvert = g3d_stream_read_int32_le(global->stream);
		local->nb -= 4;
#if DEBUG > 0
		g_debug("|%s[POLY] %04d: %d vertices", debug_pad(local->level),
		cntpoly, numvert);
#endif
		g_return_val_if_fail(numvert >= 3, FALSE);

		face = g_new0(G3DFace, 1);
		face->material = material;
		face->vertex_count = numvert;
		face->vertex_indices = g_new0(guint32, numvert);
		object->faces = g_slist_append(object->faces, face);
		for(i = 0; i < numvert; i ++) {
			face->vertex_indices[i] =
				global->vertex_offset +
				g3d_stream_read_int32_le(global->stream);
			local->nb -= 4;
			g_return_val_if_fail(
				face->vertex_indices[i] < object->vertex_count, FALSE);
		}
		type = g3d_stream_read_int16_le(global->stream);
		local->nb -= 2;

		if(type & 0xFFC6) {
			g_warning("MAX: 0x011A: unhandled 0x%08x", type);
#if DEBUG > 0
			numvert = MIN(local->nb / 2, 20);
			for(i = 0; i < numvert; i ++) {
				g_debug("|%s[POLY] 0x%04x", debug_pad(local->level),
					g3d_stream_read_int16_le(global->stream));
				local->nb -= 2;
			}
#endif
			return FALSE;
		}

		/* FIXME: order of additional data most likely wrong */
		for(i = 3; i < numvert; i ++) {
			g3d_stream_read_int32_le(global->stream);
			g3d_stream_read_int32_le(global->stream);
			local->nb -= 8;
		}
		if(type & 0x0001) {
			g3d_stream_read_int32_le(global->stream);
			local->nb -= 4;
		}
		if(type & 0x0008) {
			g3d_stream_read_int16_le(global->stream);
			local->nb -= 2;
		}
		if(type & 0x0010) {
			g3d_stream_read_int32_le(global->stream);
			local->nb -= 4;
		}
		cntpoly ++;
	}
#if DEBUG > 0
	g_debug("|%s[POLY] %d faces added to object", debug_pad(local->level),
		cntpoly);
#endif
	return TRUE;
}

/* texture vertices */
gboolean max_cb_0x08FE_0x0128(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 num;
	G3DObject *object = (G3DObject *)local->object;

	g_return_val_if_fail(local->nb >= 4, FALSE);
	g_return_val_if_fail(object != NULL, FALSE);

	num = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;

#if DEBUG > 0
	g_debug("|%s[TEXV] %d texture vertices", debug_pad(local->level), num);
#endif
	return TRUE;
}

/* texture indices */
gboolean max_cb_0x08FE_0x012B(MaxGlobalData *global, MaxLocalData *local)
{
	gint i;
	guint32 *vdata, vcnt, maxidx = 0, numpoly = 0, maxvcnt = 0;
#if 0
	G3DObject *object = (G3DObject *)local->object;
	G3DFace *face;

	g_return_val_if_fail(object != NULL, FALSE);
#endif

	while(local->nb >= 4) {
		vcnt = g3d_stream_read_int32_le(global->stream);
		local->nb -= 4;
		if(vcnt > maxvcnt)
			maxvcnt = vcnt;
#if 0
		face = g_new0(G3DFace, 1);
		face->vertex_count = vcnt;
#endif
		if(local->nb < (vcnt * 4)) {
#if DEBUG > 0
			g_debug("|%s[TIDX] %d polygons, max index: %d, max vcnt: %d "
				"(nb=%d, vcnt=%d)", debug_pad(local->level),
				numpoly, maxidx, maxvcnt, local->nb, vcnt);
#endif

			return FALSE;
		}
		vdata = g_new0(guint32, vcnt);
		numpoly ++;
		for(i = 0; i < vcnt; i ++) {
			vdata[i] = g3d_stream_read_int32_le(global->stream);
			local->nb -= 4;
			if(vdata[i] > maxidx)
				maxidx = vdata[i];
		}
#if 1
		g_free(vdata);
#endif
	}
#if DEBUG > 0
	g_debug("|%s[TIDX] %d polygons, max index: %d, max vcnt: %d",
		debug_pad(local->level), numpoly, maxidx, maxvcnt);
#endif
	return TRUE;
}

/* geometric object */
gboolean max_cb_IDROOT_IDGEOM(MaxGlobalData *global, MaxLocalData *local)
{
	G3DObject *object;

	object = g_new0(G3DObject, 1);
	object->name = g_strdup_printf("0x%04X object @ 0x%08x",
		local->id, (guint32)g3d_stream_tell(global->stream));
	local->object = object;
	global->model->objects = g_slist_append(global->model->objects, object);

	global->object = object;
	global->vertex_offset = 0;

	return TRUE;
}

/* mesh */
gboolean max_cb_IDGEOM_0x08FE(MaxGlobalData *global, MaxLocalData *local)
{
	g_return_val_if_fail(global->object != NULL, FALSE);
	return TRUE;
}

/* object name */
gboolean max_cb_IDGEOM_0x0962(MaxGlobalData *global, MaxLocalData *local)
{
	G3DObject *object = (G3DObject *)local->object;
	gchar *name;
	gint32 len;

	g_return_val_if_fail(object != NULL, FALSE);

	g_free(object->name);
	len = local->nb / 2;
	name = max_read_wchar(global->stream, len);
	object->name = g_strdup_printf("%s (0x%08x)",
		name, (guint32)g3d_stream_tell(global->stream) - len - 6);
	g_free(name);
#if DEBUG > 0
	g_debug("|%s[NAME] %s", debug_pad(local->level), object->name);
#endif
	local->nb -= len * 2;

	return TRUE;
}

/* single face */
gboolean max_cb_0x0118_0x0110(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 num;
	gint i;
	G3DObject *object = (G3DObject *)local->object;
	G3DMaterial *mat;
	G3DFace *face;

	g_return_val_if_fail(local->nb >= 4, FALSE);
	g_return_val_if_fail(object != NULL, FALSE);

	mat = (G3DMaterial *)g_slist_nth_data(global->model->materials,
		(global->vertex_offset ? 1 : 0));

	num = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;

	face = g_new0(G3DFace, 1);
	face->vertex_count = num;
	face->vertex_indices = g_new0(guint32, num);
	face->material = mat;
	object->faces = g_slist_append(object->faces, face);

	g_return_val_if_fail(local->nb >= (num * 4), FALSE);
	for(i = 0; i < num; i ++) {
		face->vertex_indices[i] =
			global->vertex_offset +
			g3d_stream_read_int32_le(global->stream);
		local->nb -= 4;
		if(face->vertex_indices[i] >= object->vertex_count) {
			g_warning("MAX: 0x0118::0x0110: vertex index (%d) >= "
				"vertex count (%d)",
				face->vertex_indices[i],
				object->vertex_count);
			face->vertex_indices[i] = 0;
		}
	}
	return TRUE;
}

/* triangles */
gboolean max_cb_0x08FE_0x0912(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 num;
	gint i, j;
	G3DObject *object;
	G3DFace *face;
	G3DMaterial *mat;

	mat = (G3DMaterial *)g_slist_nth_data(global->model->materials,
		(global->vertex_offset ? 1 : 0));

	if(local->nb < 4)
		return FALSE;

	/* faces */
	num = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;
	object = (G3DObject *)local->object;
	if(object == NULL) {
		g_warning("MAX: 0x08FE::0x0912: no object");
		return FALSE;
	}
#if DEBUG > 0
	g_debug("|%s[TRIS] %d triangles", debug_pad(local->level), num);
#endif
	for(i = 0; i < num; i ++) {
		face = g_new0(G3DFace, 1);
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		face->material = mat;
		object->faces = g_slist_append(object->faces, face);

		if(local->nb < 20)
			return FALSE;
		for(j = 0; j < 3; j ++) {
			face->vertex_indices[j] =
				global->vertex_offset +
				g3d_stream_read_int32_le(global->stream);
			if(face->vertex_indices[j] >= object->vertex_count) {
				g_warning("MAX: 0x08FE::0x0912: vertex index too high"
					" (%d (0x%08x) >= %d)",
					face->vertex_indices[j], face->vertex_indices[j],
					object->vertex_count);
				face->vertex_indices[j] = 0;
			}
		}
		/* unknown for now */
		g3d_stream_read_int32_le(global->stream);
		g3d_stream_read_int32_le(global->stream);
		local->nb -= 20;
	}
	return TRUE;
}

/* vertices */
gboolean max_cb_0x08FE_0x0914(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 num;
	gint i, j;
	G3DObject *object = (G3DObject *)local->object;

	g_return_val_if_fail(local->nb >= 4, FALSE);
	g_return_val_if_fail(object != NULL, FALSE);

	/* vertices */
	num = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;
#if DEBUG > 0
	g_debug("|%s[VERT] %d vertices", debug_pad(local->level), num);
#endif
	global->vertex_offset = object->vertex_count;
	object->vertex_count += num;
	object->vertex_data = g_realloc(object->vertex_data,
		object->vertex_count * 3 * sizeof(G3DFloat));

	for(i = 0; i < num; i ++) {
		if(local->nb < 12)
			return FALSE;
		for(j = 0; j < 3; j ++)
			object->vertex_data[(global->vertex_offset + i) * 3 + j] =
				g3d_stream_read_float_le(global->stream);
		local->nb -= 12;
	}
	return TRUE;
}

/* texture vertices */
gboolean max_cb_0x08FE_0x0916(MaxGlobalData *global, MaxLocalData *local)
{
	return max_cb_0x08FE_0x0128(global, local);
}

/* texture triangles */
gboolean max_cb_0x08FE_0x0918(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 cnttris = 0;

	while(local->nb >= 12) {
		cnttris ++;

		g3d_stream_read_int32_le(global->stream);
		g3d_stream_read_int32_le(global->stream);
		g3d_stream_read_int32_le(global->stream);
		local->nb -= 12;
	}

#if DEBUG > 0
	g_debug("|%s[TEXI] %d textured triangles (%d bytes left)",
		debug_pad(local->level), cnttris, local->nb);
#endif
	return 0;
}

/* vertices */
gboolean max_cb_0x08FE_0x2394(MaxGlobalData *global, MaxLocalData *local)
{
	return max_cb_0x08FE_0x0914(global, local);
}

/* triangles */
gboolean max_cb_0x08FE_0x2396(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 num;
	gint32 i, j;
	G3DObject *object = (G3DObject *)local->object;
	G3DFace *face;
	G3DMaterial *mat;

	mat = (G3DMaterial *)g_slist_nth_data(global->model->materials,
		(global->vertex_offset ? 1 : 0));

	g_return_val_if_fail(local->nb >= 4, FALSE);
	g_return_val_if_fail(object != NULL, FALSE);

	/* faces */
	num = g3d_stream_read_int32_le(global->stream);
	local->nb -= 4;
#if DEBUG > 0
	g_debug("|%s[TRIS] %d triangles", debug_pad(local->level), num);
#endif
	for(i = 0; i < num; i ++) {
		g_return_val_if_fail(local->nb >= 12, FALSE);

		face = g_new0(G3DFace, 1);
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		face->material = mat;
		object->faces = g_slist_append(object->faces, face);

		g_return_val_if_fail(local->nb >= 12, FALSE);
		for(j = 0; j < 3; j ++) {
			face->vertex_indices[j] =
				global->vertex_offset +
				g3d_stream_read_int32_le(global->stream);
			local->nb -= 4;
			if(face->vertex_indices[j] >= object->vertex_count) {
				g_warning("MAX: 0x08FE::0x2396: vertex index too high"
					" (%d (0x%08x) >= %d)",
					face->vertex_indices[j], face->vertex_indices[j],
					object->vertex_count);
				face->vertex_indices[j] = 0;
			}
		}
	}
	return TRUE;
}

/* material */
gboolean max_cb_IDMATG_0x4000(MaxGlobalData *global, MaxLocalData *local)
{
	G3DMaterial *material;

	if(strcmp(global->subfile, "Scene") != 0)
		return FALSE;

	material = g3d_material_new();
	material->name = g_strdup_printf("0x4000 material @ 0x%08x",
		(guint32)g3d_stream_tell(global->stream));
	local->object = material;
	global->model->materials = g_slist_append(global->model->materials,
		material);
	return TRUE;
}

/* material name */
gboolean max_cb_0x4000_0x4001(MaxGlobalData *global, MaxLocalData *local)
{
	guint32 len;
	G3DMaterial *material = (G3DMaterial *)local->object;

	g_return_val_if_fail(material != NULL, FALSE);
	if(material->name)
		g_free(material->name);
	len = local->nb / 2;
	material->name = max_read_wchar(global->stream, len);
	local->nb -= len * 2;
#if DEBUG > 0
	g_debug("|%s[MATN] %s", debug_pad(local->level), material->name);
#endif
	return TRUE;
}

/* material color */
gboolean max_cb_0x4000_0x4030(MaxGlobalData *global, MaxLocalData *local)
{
	G3DMaterial *material = (G3DMaterial *)local->object;

	g_return_val_if_fail(material != NULL, FALSE);
	g_return_val_if_fail(local->nb >= 16, FALSE);

	material->r = g3d_stream_read_float_le(global->stream);
	material->g = g3d_stream_read_float_le(global->stream);
	material->b = g3d_stream_read_float_le(global->stream);
	material->a = g3d_stream_read_float_le(global->stream);
	local->nb -= 16;
#if DEBUG > 0
	g_debug("|%s[MATC] %.2f, %.2f, %.2f, %.2f", debug_pad(local->level),
		material->r, material->g, material->b, material->a);
#endif
	return TRUE;
}

