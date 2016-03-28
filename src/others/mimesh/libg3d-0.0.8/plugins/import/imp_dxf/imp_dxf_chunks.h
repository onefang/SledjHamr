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
#ifndef _IMP_DXF_CHUNKS_H
#define _IMP_DXF_CHUNKS_H

#include "imp_dxf_callbacks.h"
#include "imp_dxf_types.h"
#include "imp_dxf_def.h"

static DxfChunkInfo dxf_chunks[] = {
	/* app: <0 */
	/* string: 0-9 */
	{ 0,     "entity type",                           DXF_T_EMPTY },
	{ 1,     "primary text value",                    DXF_T_STRING },
	{ 2,     "name",                                  DXF_T_STRING },
	{ 3,     "other text (3)",                        DXF_T_STRING },
	{ 4,     "other text (4)",                        DXF_T_STRING },
	{ 5,     "entity handle",                         DXF_T_STRING },
	{ 6,     "linetype name",                         DXF_T_STRING },
	{ 7,     "text style name",                       DXF_T_STRING },
	{ 8,     "layer name",                            DXF_T_STRING },
	{ 9,     "variable name identifier",              DXF_T_UNKNOWN },
	/* double precision 3D point value: 10-39 */
	{ 10,    "primary point, X value",                DXF_T_FLOAT64 },
	{ 11,    "point #2, X value",                     DXF_T_FLOAT64 },
	{ 12,    "point #3, X value",                     DXF_T_FLOAT64 },
	{ 13,    "point #4, X value",                     DXF_T_FLOAT64 },
	{ 14,    "point #5, X value",                     DXF_T_FLOAT64 },
	{ 15,    "point #6, X value",                     DXF_T_FLOAT64 },
	{ 16,    "point #7, X value",                     DXF_T_FLOAT64 },
	{ 17,    "point #8, X value",                     DXF_T_FLOAT64 },
	{ 18,    "point #9, X value",                     DXF_T_FLOAT64 },
	{ 19,    "point #10, X value",                    DXF_T_FLOAT64 },
	{ 20,    "primary point, Y value",                DXF_T_FLOAT64 },
	{ 21,    "point #2, Y value",                     DXF_T_FLOAT64 },
	{ 22,    "point #3, Y value",                     DXF_T_FLOAT64 },
	{ 23,    "point #4, Y value",                     DXF_T_FLOAT64 },
	{ 24,    "point #5, Y value",                     DXF_T_FLOAT64 },
	{ 25,    "point #6, Y value",                     DXF_T_FLOAT64 },
	{ 26,    "point #7, Y value",                     DXF_T_FLOAT64 },
	{ 27,    "point #8, Y value",                     DXF_T_FLOAT64 },
	{ 28,    "point #9, Y value",                     DXF_T_FLOAT64 },
	{ 29,    "point #10, Y value",                    DXF_T_FLOAT64 },
	{ 30,    "primary point, Z value",                DXF_T_FLOAT64 },
	{ 31,    "point #2, Z value",                     DXF_T_FLOAT64 },
	{ 32,    "point #3, Z value",                     DXF_T_FLOAT64 },
	{ 33,    "point #4, Z value",                     DXF_T_FLOAT64 },
	{ 34,    "point #5, Z value",                     DXF_T_FLOAT64 },
	{ 35,    "point #6, Z value",                     DXF_T_FLOAT64 },
	{ 36,    "point #7, Z value",                     DXF_T_FLOAT64 },
	{ 37,    "point #8, Z value",                     DXF_T_FLOAT64 },
	{ 38,    "point #9, Z value",                     DXF_T_FLOAT64 },
	{ 39,    "point #10, Z value",                    DXF_T_FLOAT64 },
	/* double precision floating-point value: 40-59 */
	{ 40,    "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 41,    "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 42,    "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 43,    "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 44,    "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 45,    "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 46,    "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 47,    "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 48,    "linetype scale dbl-prec fp value",      DXF_T_FLOAT64 },
	{ 49,    "repeated dbl-prec fp value",            DXF_T_FLOAT64 },
	{ 50,    "angle",                                 DXF_T_FLOAT64 },
	{ 51,    "angle",                                 DXF_T_FLOAT64 },
	{ 52,    "angle",                                 DXF_T_FLOAT64 },
	/* 16-bit integer value: 60-79 */
	{ 60,    "entity visibility",                     DXF_T_INT16 },
	{ 62,    "color number",                          DXF_T_INT16 },
	{ 65,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 66,    "entities follow",                       DXF_T_INT16 },
	{ 67,    "space",                                 DXF_T_INT16 },
	{ 68,    "APP: viewport not visible",             DXF_T_INT16 },
	{ 69,    "APP: viewport identification number",   DXF_T_INT16 },
	{ 70,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 71,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 72,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 73,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 74,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 75,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 76,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 77,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 78,    "integer value, 16-bit",                 DXF_T_INT16 },
	{ 79,    "integer value, 16-bit",                 DXF_T_INT16 },
	/* 32-bit integer value: 90-99 */
	{ 90,    "integer value, 32-bit",                 DXF_T_INT32 },
	{ 91,    "integer value, 32-bit",                 DXF_T_INT32 },

	{ 100,   "subclass data marker",                  DXF_T_STRING },
	{ 102,   "control string",                        DXF_T_STRING },
	{ 105,   "DIMVAR object handle",                  DXF_T_STRING },

	{ 110,   "UCS origin, X value",                   DXF_T_FLOAT64 },
	{ 111,   "UCS X-axis, X value",                   DXF_T_FLOAT64 },
	{ 112,   "UCS Y-axis, X value",                   DXF_T_FLOAT64 },
	{ 120,   "UCS origin, Y value",                   DXF_T_FLOAT64 },
	{ 121,   "UCS X-axis, Y value",                   DXF_T_FLOAT64 },
	{ 122,   "UCS Y-axis, Y value",                   DXF_T_FLOAT64 },
	{ 130,   "UCS origin, Z value",                   DXF_T_FLOAT64 },
	{ 131,   "UCS X-axis, Z value",                   DXF_T_FLOAT64 },
	{ 132,   "UCS Y-axis, Z value",                   DXF_T_FLOAT64 },

	/* double precision floating-point value: 140-149 */
	{ 140,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 141,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 142,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 143,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 144,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 145,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 146,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 147,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 148,   "double-precision floating-point value", DXF_T_FLOAT64 },
	{ 149,   "double-precision floating-point value", DXF_T_FLOAT64 },
	/* 16-bit integer value: 170-179 */
	{ 170,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 171,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 172,   "integer value, 16-bit",                 DXF_T_INT16 },

	{ 210,   "extrusion direction, X value",          DXF_T_FLOAT64 },
	{ 220,   "extrusion direction, Y value",          DXF_T_FLOAT64 },
	{ 230,   "extrusion direction, Z value",          DXF_T_FLOAT64 },
	/* 16-bit integer value: 270-279 */
	{ 270,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 271,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 272,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 273,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 274,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 275,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 276,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 277,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 278,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 279,   "integer value, 16-bit",                 DXF_T_INT16 },
	/* 16-bit integer value: 280-289 */
	{ 280,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 281,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 282,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 283,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 284,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 285,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 286,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 287,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 288,   "integer value, 16-bit",                 DXF_T_INT16 },
	{ 289,   "integer value, 16-bit",                 DXF_T_INT16 },

	/* boolean flag value: 290-299 */

	{ 330,   "softpointer handle",                    DXF_T_STRING },
	{ 331,   "softpointer handle",                    DXF_T_STRING },
	{ 340,   "hardpointer handle",                    DXF_T_STRING },
	{ 350,   "softowner handle",                      DXF_T_STRING },

	{ 370,   "lineweight enum value",                 DXF_T_INT16 },

	{ 390,   "PlotStyleName handle",                  DXF_T_STRING },

	{ 420,   "integer value, 32-bit",                 DXF_T_INT32 },

	{ 1000,  "ASCII string",                          DXF_T_STRING },
	{ 1001,  "registered application name",           DXF_T_STRING },
	{ 1002,  "extended data control string",          DXF_T_STRING },

	{ 1010,  "extended data point, X value",          DXF_T_FLOAT64 },
	{ 1011,  "extended 3D world space, X value",      DXF_T_FLOAT64 },
	{ 1020,  "extended data point, Y value",          DXF_T_FLOAT64 },
	{ 1021,  "extended 3D world space, Y value",      DXF_T_FLOAT64 },
	{ 1030,  "extended data point, Z value",          DXF_T_FLOAT64 },
	{ 1031,  "extended 3D world space, Z value",      DXF_T_FLOAT64 },
	{ 1040,  "extended data dbl-prec fp value",       DXF_T_FLOAT64 },
	{ 1041,  "extended data distance value",          DXF_T_FLOAT64 },
	{ 1070,  "extended data 16-bit integer",          DXF_T_INT16 },
	{ 1071,  "extended data 32-bit integer",          DXF_T_INT32 },

	{ DXF_CODE_INVALID, NULL, DXF_T_UNKNOWN },
};

#endif /* _IMP_DXF_CHUNKS_H */
