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

#include <stdio.h>
#include <glib/gstdio.h>
#include <g3d/stream.h>

typedef struct {
	FILE *f;
	goffset size;
} G3DStreamFile;

static gsize g3d_stream_file_read(gpointer ptr, gsize size, gpointer data)
{
	G3DStreamFile *sf = (G3DStreamFile *)data;
	return fread((void *)ptr, 1, size, sf->f);
}

static gchar *g3d_stream_file_read_line(gchar *buf, gsize size, gpointer data)
{
	G3DStreamFile *sf = (G3DStreamFile *)data;
	return fgets(buf, size, sf->f);
}

static gint g3d_stream_file_seek(gpointer data, goffset offset,
	GSeekType whence)
{
	G3DStreamFile *sf = (G3DStreamFile *)data;
	int w = SEEK_SET;
	switch(whence) {
		case G_SEEK_SET: w = SEEK_SET; break;
		case G_SEEK_CUR: w = SEEK_CUR; break;
		case G_SEEK_END: w = SEEK_END; break;
	}
	return fseek(sf->f, offset, w);
}

static goffset g3d_stream_file_tell(gpointer data)
{
	G3DStreamFile *sf = (G3DStreamFile *)data;
	return ftell(sf->f);
}

static goffset g3d_stream_file_size(gpointer data)
{
	G3DStreamFile *sf = (G3DStreamFile *)data;
	return sf->size;
}

static gboolean g3d_stream_file_eof(gpointer data)
{
	G3DStreamFile *sf = (G3DStreamFile *)data;
	return feof(sf->f);
}

static gint g3d_stream_file_close(gpointer data)
{
	G3DStreamFile *sf = (G3DStreamFile *)data;
	FILE *f = sf->f;
	g_free(sf);
	return fclose(f);
}

EAPI
G3DStream *g3d_stream_open_file(const gchar *filename, const gchar *mode)
{
	G3DStreamFile *sf;
	struct stat stats;
	guint32 flags = 0;

	sf = g_new0(G3DStreamFile, 1);

	/* open file; may be moved to some GLib interface */
	sf->f = fopen(filename, mode);
	if(sf->f == NULL) {
		g_free(sf);
		return NULL;
	}

	/* get file size */
	if(g_stat(filename, &stats) != 0)
		sf->size = -1;
	sf->size = stats.st_size;

	if(mode[0] == 'r')
		flags |= (1 << G3D_STREAM_READABLE);
	if(mode[0] == 'w')
		flags |= (1 << G3D_STREAM_WRITABLE);

	flags |= (1 << G3D_STREAM_SEEKABLE);

	return g3d_stream_new_custom(flags, filename,
		g3d_stream_file_read, g3d_stream_file_read_line,
		g3d_stream_file_seek, g3d_stream_file_tell,
		g3d_stream_file_size, g3d_stream_file_eof,
		g3d_stream_file_close, sf, NULL);
}

