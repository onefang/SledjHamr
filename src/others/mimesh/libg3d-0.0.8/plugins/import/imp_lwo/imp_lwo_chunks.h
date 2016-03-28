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
#ifndef _IMP_LWO_CHUNKS_H
#define _IMP_LWO_CHUNKS_H

#include <g3d/iff.h>

#include "imp_lwo_callbacks.h"

static G3DIffChunkInfo lwo_chunks[] = {
	{ "AAST", "antialiasing strength",       0, NULL },
	{ "ADTR", "additive transparency",       0, NULL },
	{ "ALPH", "alpha mode",                  0, NULL },
	{ "AXIS", "major axis",                  0, NULL },
	{ "BBOX", "bounding box",                0, NULL },
	{ "BLOK", "block",                       1, NULL },
	{ "BTEX", "unknown",                     0, NULL },
	{ "BUMP", "bump intensity",              0, NULL },
	{ "CLIP", "image or image sequence",     1, lwo_cb_CLIP },
	{ "CLRF", "color filter",                0, NULL },
	{ "CLRH", "color highlights",            0, NULL },
	{ "CMNT", "unknown",                     0, NULL },
	{ "COLR", "color",                       0, lwo_cb_COLR },
	{ "CTEX", "unknown",                     0, NULL },
	{ "DESC", "description",                 0, NULL },
	{ "DIFF", "diffuse",                     0, NULL },
	{ "DTEX", "unknown",                     0, NULL },
	{ "FKEY", "gradient key values",         0, NULL },
	{ "FLAG", "flags",                       0, NULL },
	{ "FUNC", "algorithm and parameters",    0, NULL },
	{ "GLOS", "specular glossiness",         0, NULL },
	{ "GRAD", "gradient texture (header)",   0, NULL },
	{ "GREN", "gradient end",                0, NULL },
	{ "GRPT", "gradient repeat mode",        0, NULL },
	{ "GRST", "gradient start",              0, NULL },
	{ "GVAL", "glow value",                  0, NULL },
	{ "ICON", "icon",                        0, NULL },
	{ "IKEY", "gradient key parameters",     0, NULL },
	{ "IMAG", "image index",                 0, lwo_cb_IMAG },
	{ "IMAP", "image map (header)",          0, NULL },
	{ "INAM", "item name",                   0, NULL },
	{ "LAYR", "layer",                       0, NULL },
	{ "LTEX", "unknown",                     0, NULL },
	{ "LUMI", "luminosity",                  0, NULL },
	{ "PIXB", "pixel blending",              0, NULL },
	{ "PNAM", "parameter name",              0, NULL },
	{ "PNTS", "points",                      0, lwo_cb_PNTS },
	{ "POLS", "polygons",                    0, lwo_cb_POLS },
	{ "PROC", "procedural texture (header)", 0, NULL },
	{ "PROJ", "projection mode",             0, NULL },
	{ "PTAG", "polygon tag mapping",         0, lwo_cb_PTAG },
	{ "RBLR", "reflection blurring",         0, NULL },
	{ "REFL", "reflection",                  0, NULL },
	{ "RFLT", "unknown",                     0, NULL },
	{ "RFOP", "reflection options",          0, NULL },
	{ "RIMG", "reflection map image",        0, NULL },
	{ "RIND", "refractive index",            0, NULL },
	{ "RSAN", "reflection map seam angle",   0, NULL },
	{ "SHDR", "shader",                      0, NULL },
	{ "SIDE", "polygon sidedness",           0, NULL },
	{ "SMAN", "max smooting angle",          0, NULL },
	{ "SPEC", "specular",                    0, lwo_cb_SPEC },
	{ "SRFS", "surface materials",           0, lwo_cb_SRFS },
	{ "STEX", "unknown",                     0, NULL },
	{ "STIL", "still image",                 0, lwo_cb_STIL },
	{ "SURF", "surface material",            1, lwo_cb_SURF },
	{ "TAAS", "unknown",                     0, NULL },
	{ "TAGS", "tags",                        0, lwo_cb_TAGS },
	{ "TAMP", "texture amplitude",           0, NULL },
	{ "TCLR", "unknown",                     0, NULL },
	{ "TCTR", "unknown",                     0, NULL },
	{ "TEXT", "commentary text",             0, NULL },
	{ "TFAL", "unknown",                     0, NULL },
	{ "TFLG", "unknown",                     0, NULL },
	{ "TFP0", "unknown",                     0, NULL },
	{ "TFP1", "unknown",                     0, NULL },
	{ "TFP2", "unknown",                     0, NULL },
	{ "TFP3", "unknown",                     0, NULL },
	{ "TFRQ", "unknown",                     0, NULL },
	{ "TIMG", "refraction map image",        0, NULL },
	{ "TIP0", "unknown",                     0, NULL },
	{ "TMAP", "texture map",                 0, NULL },
	{ "TRAN", "transparency",                0, lwo_cb_TRAN },
	{ "TRNL", "translucency",                0, NULL },
	{ "TROP", "transparency options",        0, NULL },
	{ "TSIZ", "unknown",                     0, NULL },
	{ "TSP0", "unknown",                     0, NULL },
	{ "TTEX", "unknown",                     0, NULL },
	{ "TVAL", "unknown",                     0, NULL },
	{ "VALU", "basic value",                 0, NULL },
	{ "VDIF", "unknown",                     0, NULL },
	{ "VMAD", "discontinuous vertex map",    0, NULL },
	{ "VMAP", "uv vertex map",               0, lwo_cb_VMAP },
	{ "VMPA", "vertex map parameters",       0, NULL },
	{ "VRFL", "unknown",                     0, NULL },
	{ "VSPC", "unknown",                     0, NULL },
	{ "VTRN", "unknown",                     0, NULL },
	{ "WRAP", "image wrap options",          0, NULL },
	{ "WRPH", "wrap amount (width)",         0, NULL },
	{ "WRPW", "wrap amount (height)",        0, NULL },

	{ NULL, NULL, 0, NULL }
};

#endif /* _IMP_LWO_CHUNKS_H */
