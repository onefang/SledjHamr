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
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-impl.h>
#include <gsf/gsf-impl-utils.h>

#include <g3d/types.h>
#include <g3d/stream.h>

#include "stream_gsf_class.h"

static GObjectClass *parent_class;

struct _G3DGsfInputStream {
	GsfInput input;
	G3DStream *stream;
	guint8 *buf;
	gsize buf_size;
};

typedef struct {
	GsfInputClass input_class;
} G3DGsfInputStreamClass;

EAPI
GsfInput *g3d_gsf_input_stream_new(G3DStream *stream)
{
	G3DGsfInputStream *g3dsf;

	g3dsf = g_object_new(G3D_GSF_INPUT_STREAM_TYPE, NULL);
	g3dsf->stream = stream;

	gsf_input_set_size(GSF_INPUT(g3dsf), g3d_stream_size(stream));
	gsf_input_set_name_from_filename(GSF_INPUT(g3dsf), stream->uri);

	return GSF_INPUT(g3dsf);
}

EAPI G3DStream *g3d_gsf_input_stream_get_stream(GsfInput *input)
{
	G3DGsfInputStream *g3dsf = G3D_GSF_INPUT_STREAM(input);

	return g3dsf->stream;
}

static void g3d_gsf_input_stream_finalize(GObject *obj)
{
	G3DGsfInputStream *g3dsf = G3D_GSF_INPUT_STREAM(obj);

	if(g3dsf->buf)
		g_free(g3dsf->buf);

	parent_class->finalize(obj);
}

static GsfInput *g3d_gsf_input_stream_dup(GsfInput *src_input, GError **err)
{
	G3DGsfInputStream *g3dsf = G3D_GSF_INPUT_STREAM(src_input);

	return g3d_gsf_input_stream_new(g3dsf->stream);
}

static const guint8 *g3d_gsf_input_stream_read(GsfInput *input,
	size_t num_bytes, guint8 *buffer)
{
	G3DGsfInputStream *g3dsf = G3D_GSF_INPUT_STREAM(input);

	if(buffer == NULL) {
		if(g3dsf->buf_size < num_bytes) {
			g3dsf->buf_size = num_bytes;
			if(g3dsf->buf)
				g_free(g3dsf->buf);
			g3dsf->buf = g_new0(guint8, num_bytes);
		}
		buffer = g3dsf->buf;
	}

	g3d_stream_read(g3dsf->stream, buffer, num_bytes);
	return buffer;
}

static gboolean g3d_gsf_input_stream_seek(GsfInput *input, gsf_off_t offset,
	GSeekType whence)
{
	G3DGsfInputStream *g3dsf = G3D_GSF_INPUT_STREAM(input);

	return g3d_stream_seek(g3dsf->stream, offset, whence);
}

static void g3d_gsf_input_stream_init(GObject *obj)
{
	G3DGsfInputStream *g3dsf = G3D_GSF_INPUT_STREAM(obj);

	g3dsf->stream = NULL;
	g3dsf->buf = NULL;
	g3dsf->buf_size = 0;
}

static void g3d_gsf_input_stream_class_init(GObjectClass *gobject_class)
{
	GsfInputClass *input_class = GSF_INPUT_CLASS(gobject_class);

	gobject_class->finalize = g3d_gsf_input_stream_finalize;
	input_class->Dup  = g3d_gsf_input_stream_dup;
	input_class->Read = g3d_gsf_input_stream_read;
	input_class->Seek = g3d_gsf_input_stream_seek;

	parent_class = g_type_class_peek_parent(gobject_class);
}

GSF_CLASS(G3DGsfInputStream,
	g3d_gsf_input_stream,
	g3d_gsf_input_stream_class_init,
	g3d_gsf_input_stream_init,
	GSF_INPUT_TYPE)

