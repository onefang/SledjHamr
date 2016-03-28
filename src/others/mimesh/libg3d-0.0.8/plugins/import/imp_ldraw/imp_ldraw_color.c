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
#include <g3d/material.h>

#include "imp_ldraw_types.h"

typedef struct {
	gint32 id;
	const gchar *name;
	G3DFloat r;
	G3DFloat g;
	G3DFloat b;
	G3DFloat a;
} LDrawColorDef;

static LDrawColorDef ldraw_colors[] = {
	{   0, "Black",                 0.13, 0.13, 0.13, 1.0 },
	{   1, "Blue",                  0.00, 0.20, 0.70, 1.0 },
	{   2, "Green",                 0.00, 0.55, 0.08, 1.0 },
	{   3, "Teal",                  0.00, 0.60, 0.62, 1.0 },
	{   4, "Red",                   0.77, 0.00, 0.15, 1.0 },
	{   5, "Dark Pink",             0.87, 0.40, 0.58, 1.0 },
	{   6, "Brown",                 0.36, 0.13, 0.00, 1.0 },
	{   7, "Gray",                  0.76, 0.76, 0.76, 1.0 },
	{   8, "Dark Gray",             0.39, 0.37, 0.32, 1.0 },
	{   9, "Light Blue",            0.42, 0.67, 0.86, 1.0 },
	{  10, "Bright Green",          0.42, 0.93, 0.56, 1.0 },
	{  11, "Cyan",                  0.20, 0.65, 0.65, 1.0 },
	{  12, "Light Red",             1.00, 0.52, 0.48, 1.0 },
	{  13, "Pink",                  0.98, 0.64, 0.78, 1.0 },
	{  14, "Yellow",                1.00, 0.86, 0.00, 1.0 },
	{  15, "White",                 1.00, 1.00, 1.00, 1.0 },

	{  17, "Light Green",           0.73, 1.00, 0.81, 1.0 },
	{  18, "Light Yellow",          0.99, 0.91, 0.59, 1.0 },
	{  20, "Light Violet",          0.84, 0.77, 0.90, 1.0 },
	{  28, "Dark Tan",              0.77, 0.59, 0.31, 1.0 },
	{  32, "Trans Gray",            0.39, 0.37, 0.32, 0.9 },
	{  33, "Trans Blue",            0.00, 0.13, 0.63, 0.9 },
	{  36, "Trans Red",             0.77, 0.00, 0.15, 0.9 },
	{  39, "Trans Light Gray",      0.76, 0.76, 0.76, 0.9 }, /* FIXME */
	{  40, "Trans Gray",            0.39, 0.37, 0.32, 0.9 },
	{  41, "Trans Light Cyan",      0.68, 0.94, 0.93, 0.95 },
	{  46, "Trans Yellow",          0.79, 0.69, 0.00, 0.9 },
	{  47, "Clear (trans white)",   1.00, 1.00, 1.00, 0.9 },
	{  72, "Dark Stone Gray",       0.39, 0.37, 0.38, 1.0 },
	{ 272, "Dark Blue",             0.00, 0.11, 0.41, 1.0 },
	{ 288, "Dark Green",            0.15, 0.27, 0.17, 1.0 },
	{ 320, "Dark Red",              0.47, 0.00, 0.11, 1.0 },
	{ 334, "Chrome Gold",           0.88, 0.43, 0.07, 1.0 },
	{ 336, "Earth Orange",          0.82, 0.51, 0.02, 1.0 },
	{ 383, "Chrome Silver",         0.88, 0.88, 0.88, 1.0 },
	{ 431, "Light Green",           0.73, 1.00, 0.81, 1.0 },
	{ 463, "Light Red",             1.00, 0.52, 0.48, 1.0 },
	{ 484, "Dark Orange",           0.70, 0.24, 0.00, 1.0 },
	{ 494, "Electric Contact",      0.82, 0.82, 0.82, 1.0 },
	{ 495, "Light Yellow",          0.99, 0.91, 0.59, 1.0 },
	{ 503, "Light Gray",            0.90, 0.89, 0.85, 1.0 },

	{  -1, NULL, 0,0,0,0 }
};

gboolean ldraw_color_init(LDrawLibrary *lib)
{
	G3DMaterial *material;
	gint32 i;

	lib->colordb = g_hash_table_new(g_int_hash, g_int_equal);

	for(i = 0; ldraw_colors[i].id >= 0; i ++) {
		material = g3d_material_new();
		material->name = g_strdup(ldraw_colors[i].name);
		material->r = ldraw_colors[i].r;
		material->g = ldraw_colors[i].g;
		material->b = ldraw_colors[i].b;
		material->a = ldraw_colors[i].a;
		g_hash_table_insert(lib->colordb, &(ldraw_colors[i].id), material);
		lib->colorlist = g_slist_append(lib->colorlist, material);
#if DEBUG > 2
		g_debug("LDraw: adding color '%s' with id %d",
			ldraw_colors[i].name, ldraw_colors[i].id);
#endif
	}
	return TRUE;
}

G3DMaterial *ldraw_color_lookup(LDrawLibrary *lib, guint32 colid)
{
	G3DMaterial *material;
	guint32 fbid = 0;

	/* current color */
	if(colid == 16)
		return NULL;

	material = g_hash_table_lookup(lib->colordb, &colid);
	if(material == NULL) { /* fall back */
#if DEBUG > 0
		g_debug("LDraw: failed to lookup color %u", colid);
#endif
		material = g_hash_table_lookup(lib->colordb, &fbid);
	}
	return material;
}

