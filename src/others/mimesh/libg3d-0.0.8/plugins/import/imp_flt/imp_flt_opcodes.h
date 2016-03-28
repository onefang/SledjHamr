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
#ifndef _IMP_FLT_OPCODES_H
#define _IMP_FLT_OPCODES_H

#include <glib.h>

#include "imp_flt_callbacks.h"

typedef struct {
	guint32 opcode;
	gchar *description;
	gboolean container;
	FltCallbackFunc callback;
} FltOpcode;

static FltOpcode flt_opcodes[] = {
	{    1, "header",                              0, flt_cb_0001 },
	{    2, "group",                               0, flt_cb_0002 },

	{    4, "object",                              0, flt_cb_0004 },
	{    5, "face",                                0, flt_cb_0005 },

	{   10, "push level",                          0, flt_cb_0010 },
	{   11, "pop level",                           0, flt_cb_0011 },

	{   14, "degree of freedom",                   0, NULL },

	{   19, "push subface",                        0, NULL },
	{   20, "pop subface",                         0, NULL },
	{   21, "push extension",                      0, NULL },
	{   22, "pop extension",                       0, NULL },
	{   23, "continuation",                        0, NULL },

	{   31, "comment",                             0, NULL },
	{   32, "color palette",                       0, flt_cb_0032 },
	{   33, "long ID",                             0, flt_cb_0033 },

	{   49, "matrix",                              0, NULL },
	{   50, "vector",                              0, NULL },

	{   52, "multitexture",                        0, NULL },
	{   53, "UV list",                             0, NULL },

	{   55, "binary separating plane",             0, NULL },

	{   60, "replicate",                           0, NULL },
	{   61, "instance reference",                  0, NULL },
	{   62, "instance definition",                 0, NULL },
	{   63, "external reference",                  0, NULL },
	{   64, "texture palette",                     0, flt_cb_0064 },

	{   67, "vertex palette",                      0, flt_cb_0067 },
	{   68, "vertex with color",                   0, flt_cb_0068 },
	{   69, "vertex with color and normal",        0, flt_cb_0069 },
	{   70, "vertex with color, normal and UV",    0, flt_cb_0070 },
	{   71, "vertex with color and UV",            0, flt_cb_0071 },
	{   72, "vertex list",                         0, flt_cb_0072 },
	{   73, "level of detail",                     0, NULL },
	{   74, "bounding box",                        0, NULL },

	{   76, "rotate about edge",                   0, NULL },

	{   78, "translate",                           0, NULL },
	{   79, "scale",                               0, NULL },
	{   80, "rotate about point",                  0, NULL },
	{   81, "rotate and/or scale to point",        0, NULL },
	{   82, "put",                                 0, NULL },
	{   83, "eyepoint and trackplane palette",     0, NULL },
	{   84, "mesh",                                0, flt_cb_0084 },
	{   85, "local vertex pool",                   0, flt_cb_0085 },
	{   86, "mesh primitive",                      0, flt_cb_0086 },
	{   87, "road segment",                        0, NULL },
	{   88, "road zone",                           0, NULL },
	{   89, "morph vertex list",                   0, NULL },
	{   90, "linkage palette",                     0, NULL },
	{   91, "sound node",                          0, NULL },
	{   92, "road path",                           0, NULL },
	{   93, "sound palette",                       0, NULL },
	{   94, "general matrix",                      0, NULL },
	{   95, "text",                                0, NULL },
	{   96, "switch",                              0, NULL },
	{   97, "line style palette",                  0, NULL },
	{   98, "clip region",                         0, NULL },

	{  100, "extension",                           0, NULL },
	{  101, "light source",                        0, NULL },
	{  102, "light source palette",                0, NULL },

	{  105, "bounding sphere",                     0, NULL },
	{  106, "bounding cylinder",                   0, NULL },
	{  107, "bounding convex hull",                0, NULL },
	{  108, "bounding volume center",              0, NULL },
	{  109, "bounding volume orientation",         0, NULL },

	{  111, "light point",                         0, NULL },
	{  112, "texture mapping palette",             0, NULL },
	{  113, "material palette",                    0, flt_cb_0113 },
	{  114, "name table",                          0, NULL },
	{  115, "continuously adaptive terrain (CAT)", 0, NULL },
	{  116, "CAT data",                            0, NULL },

	{  119, "bounding histogram",                  0, NULL },

	{  126, "curve",                               0, NULL },
	{  127, "road construction",                   0, NULL },
	{  128, "light point appearance palette",      0, NULL },
	{  129, "light point animation",               0, NULL },
	{  130, "indexed light point",                 0, NULL },

	{  132, "indexed string",                      0, NULL },
	{  133, "shader",                              0, NULL },

	{ 0, NULL, 0, NULL }
};

#endif /* _IMP_FLT_OPCODES_H */
