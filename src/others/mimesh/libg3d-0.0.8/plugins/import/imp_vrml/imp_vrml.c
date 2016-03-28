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

#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <glib.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/material.h>

#include "imp_vrml_v1.h"

#define VRML_FT_VRML      0x01
#define VRML_FT_INVENTOR  0x02

#define MAX_LINE_SIZE     2048

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	yyscan_t scanner;
	YY_BUFFER_STATE bufstate;
	G3DMaterial *material;
	gchar line[MAX_LINE_SIZE + 1], *buffer, *bufp;
	guint32 ver_maj, ver_min, filetype, buflen;

	memset(line, 0, MAX_LINE_SIZE);
	g3d_stream_read_line(stream, line, MAX_LINE_SIZE);
	if(strncmp(line, "#VRML", 5) == 0)
		filetype = VRML_FT_VRML;
	else if(strncmp(line, "#Inventor", 9) == 0)
		filetype = VRML_FT_INVENTOR;
	else {
		g_warning("file '%s' is not a VRML file", stream->uri);
		return FALSE;
	}

	/* FIXME: more than one space between VRML and Vx.x? */
	ver_maj = line[7] - '0';
	ver_min = line[9] - '0';

#if DEBUG > 0
	g_debug("VRML: version %d.%d", ver_maj, ver_min);
#endif

	setlocale(LC_NUMERIC, "C");

	if((filetype == VRML_FT_INVENTOR) || (ver_maj == 1)) {
		/* Inventor / VRML 1.x */
		/* read complete file to buffer */
		buflen = g3d_stream_size(stream) + 1;
		buffer = g_new0(gchar, buflen);
		bufp = buffer;
		memset(buffer, 0, buflen);
		memset(line, 0, MAX_LINE_SIZE);
		while(!g3d_stream_eof(stream) &&
			g3d_stream_read_line(stream, line, MAX_LINE_SIZE)) {
			memcpy(bufp, line, strlen(line));
			bufp += strlen(line);
		}
		material = g3d_material_new();
		material->name = g_strdup("fallback material");
		model->materials = g_slist_append(model->materials, material);

		vrml_v1_yylex_init(&scanner);
		vrml_v1_yyset_extra(model, scanner);
		bufstate = vrml_v1_yy_scan_string(buffer, scanner);
		if(bufstate) {
			vrml_v1_yylex(scanner);
			vrml_v1_yy_delete_buffer(bufstate, scanner);
		}
		vrml_v1_yylex_destroy(scanner);
		g_free(buffer);
	} else if(ver_maj == 2) {
		g_warning("VRML 2 is not yet supported");
		return FALSE;
	} else {
		g_warning("unknown VRML version %d.%d", ver_maj, ver_min);
		return FALSE;
	}

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("VRML 1.x & SGI inventor models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("vrml:iv", ":", 0);
}

/* FIXME */
extern int yywrap(yyscan_t yyscanner);
int vrml_v1_yywrap(yyscan_t yyscanner) { return yywrap(yyscanner); }
