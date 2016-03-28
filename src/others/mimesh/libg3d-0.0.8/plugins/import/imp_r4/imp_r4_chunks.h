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
#ifndef _IMP_R4_CHUNKS_H
#define _IMP_R4_CHUNKS_H

#include <g3d/iff.h>

#include "imp_r4_callbacks.h"

static G3DIffChunkInfo r4_chunks[] = {
	{ "AFX1", "unknown",                           0, NULL }, /* mon */
	{ "DRE2", "triangles",                         0, r4_cb_DRE2 },
	{ "DSP2", "unknown",                           0, NULL }, /* R4.3 */
	{ "GLOW", "unknown",                           0, NULL }, /* R4.3 */
	{ "GMA1", "material",                          0, r4_cb_GMAx },
	{ "GMAT", "material",                          0, r4_cb_GMAx },
	{ "INFO", "information",                       0, NULL },
	{ "KSYS", "coordinate system",                 0, r4_cb_KSYS },
	{ "KUG1", "sphere",                            0, r4_cb_KUG1 },
	{ "LGH3", "light",                             0, r4_cb_LGH3 },
	{ "MTEX", "unknown",                           0, NULL }, /* mon */
	{ "NEB1", "fog?",                              0, NULL }, /* mon */
	{ "NEBL", "fog?",                              0, NULL },
	{ "PKL1", "unknown",                           0, NULL }, /* R4.3 */
	{ "PKTM", "points",                            0, r4_cb_PKTM },
	{ "POLY", "polygons",                          0, NULL }, /* mon */
	{ "RGE1", "geometry",                          0, r4_cb_RGE1 },
	{ "RKA2", "camera",                            0, r4_cb_RKAx },
	{ "RKA3", "camera",                            0, r4_cb_RKAx },
	{ "ROBJ", "object name",                       0, r4_cb_ROBJ },
	{ "RSPG", "unknown",                           0, NULL }, /* R4.3 */
	{ "SEQ1", "sequence",                          0, NULL },
	{ "SEQ2", "sequence",                          0, NULL },
	{ "SKE2", "unknown",                           0, NULL }, /* R4.3 */
	{ "SUR1", "surface",                           0, r4_cb_SURx },
	{ "SURF", "surface",                           0, r4_cb_SURx },
	{ "TXM1", "unknown",                           0, r4_cb_TXM1 },
	{ "TXO1", "unknown",                           0, r4_cb_TXO1 },

	{ NULL, NULL, 0, NULL }
};

#endif /* _IMP_R4_CHUNKS_H */
