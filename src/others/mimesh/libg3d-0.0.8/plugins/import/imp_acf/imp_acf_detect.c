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

#include "imp_acf_detect.h"
#include "imp_acf_def_acf603.h"
#include "imp_acf_def_acf625.h"
#include "imp_acf_def_acf630.h"
#include "imp_acf_def_acf640.h"
#include "imp_acf_def_acf651.h"
#include "imp_acf_def_acf700.h"
#include "imp_acf_def_acf740.h"

typedef struct {
	goffset size;
	guint32 vmajor;
	guint32 vminor;
	const AcfDef *def;
} AcfDetectInfo;

static AcfDetectInfo acf_detect_info[] = {
	{ 433337, 6,  3, acf_def_acf603 },
	{ 433693, 6, 25, acf_def_acf625 },
	{ 453981, 6, 30, acf_def_acf630 },
	{ 454977, 6, 40, acf_def_acf640 },
	{ 454985, 6, 51, acf_def_acf651 },
	{ 646626, 7,  0, acf_def_acf700 },
	{ 647101, 7, 40, acf_def_acf740 },
	{ 0, 0, 0, NULL }
};

const AcfDef *acf_detect_version(AcfGlobalData *global)
{
	gint32 i;

	for(i = 0; acf_detect_info[i].size > 0; i ++) {
		if(g3d_stream_size(global->stream) == acf_detect_info[i].size) {
#if DEBUG > 0
			g_debug("ACF: version %d.%02d detected",
				acf_detect_info[i].vmajor, acf_detect_info[i].vminor);
#endif
			return acf_detect_info[i].def;
		}
	}

	for(i = 0; acf_detect_info[i].size > 0; i ++)
		if(acf_detect_info[i].size > g3d_stream_size(global->stream)) {
#if DEBUG > 0
			g_debug("ACF: falling back to version %d.%02d",
				acf_detect_info[i].vmajor, acf_detect_info[i].vminor);
#endif
			return acf_detect_info[i].def;
		}

	/* FIXME: return 7.40 for models newer than that */

	g_warning("ACF: could not detect version");
	return NULL;
}
