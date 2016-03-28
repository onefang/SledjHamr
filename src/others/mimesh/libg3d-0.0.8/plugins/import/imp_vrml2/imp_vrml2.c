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

/*
 * an experiment to parse VRML without flex, version 2 is tried as a starter,
 * v1 and Inventor stuff will be added if successful.
 */

#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <g3d/types.h>
#include <g3d/stream.h>

#include "imp_vrml2.h"
#include "imp_vrml_read.h"

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	gchar buf[128];
	gboolean retval;
	VrmlReader *reader;

	setlocale(LC_NUMERIC, "C");

	buf[127] = '\0';
	g3d_stream_read_line(stream, buf, 127);
	if(strncmp(buf, "#VRML V2.0 utf8", 15) != 0) {
		g_warning("%s is not a valid VRML 2.0 UTF-8 stream", stream->uri);
		return FALSE;
	}

	reader = g_new0(VrmlReader, 1);
	reader->max_bufsize = 2048;
	reader->buffer = g_new0(gchar, reader->max_bufsize);
	reader->bufp = reader->buffer;
	reader->line = 2;
	reader->stream = stream;
	reader->model = model;

	retval = vrml_read_global(reader);

	g_free(reader);

	return retval;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("VRML v2 models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("wrl", ":", 0);
}

/*****************************************************************************/
