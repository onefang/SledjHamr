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

#include <g3d/iff.h>
#include <g3d/stream.h>
#include <g3d/material.h>
#include <g3d/debug.h>

static gchar *padding = "                    ";

/* header */
gboolean rbh_cb_RBHH(G3DIffGlobal *global, G3DIffLocal *local)
{
	guint32 x0, x1, x2;
	guint32 i, num;

	num = local->nb / 12;
	for(i = 0; i < num; i ++)
	{
		x0 = g3d_stream_read_int32_le(global->stream);
		x1 = g3d_stream_read_int32_le(global->stream);
		x2 = g3d_stream_read_int32_le(global->stream);
		local->nb -= 12;

		g_debug("\\%s[RBH][RBHH] %d: 0x%08x 0x%08x 0x%08x",
			padding + (strlen(padding) - local->level),
			i + 1, x0, x1, x2);
	}

	return TRUE;
}

/* body */
gboolean rbh_cb_BODY(G3DIffGlobal *global, G3DIffLocal *local)
{
	guint32 nverts, nfaces;
	guint32 maxx = 0, x;
	G3DObject *object;
	G3DMaterial *material;

	if(local->nb < 4) return TRUE; /* zero size BODY tags? */


	nverts = g3d_stream_read_int16_le(global->stream);

	nfaces = g3d_stream_read_int16_le(global->stream);
	local->nb -= 4;

	g_debug(
		"\\%s[RBH][BODY] %d verts, %d faces, %d bytes remaining (%d x 4 + %d)",
		padding + (strlen(padding) - local->level),
		nverts, nfaces, local->nb,
		local->nb / 4, local->nb % 4);

	if(nverts == 0) return TRUE; /* skip for now */

	object = g_new0(G3DObject, 1);
	object->name = g_strdup("BODY");

	material = g3d_material_new();
	object->materials = g_slist_append(object->materials, material);

	global->model->objects = g_slist_append(global->model->objects, object);

	while(local->nb >= 4)
	{
		x = g3d_stream_read_int32_le(global->stream);
		if(x > maxx) maxx = x;
		local->nb -= 4;
	}

	g_debug("\\%s[RBH][BODY] max. value: %d",
		padding + (strlen(padding) - local->level),
		maxx);

	return TRUE;
}

/* ?? */
gboolean rbh_cb_RELC(G3DIffGlobal *global, G3DIffLocal *local)
{
	guint32 maxx = 0, x;

	while(local->nb >= 4)
	{
		x = g3d_stream_read_int32_le(global->stream);
		if(x > maxx) maxx = x;
		local->nb -= 4;
	}

	g_debug("\\%s[RBH][RELC] max. value: %d",
		padding + (strlen(padding) - local->level),
		maxx);

	return TRUE;
}
