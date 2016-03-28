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
#include <glib.h>
#include <g3d/g3d.h>
#include <g3d/stream.h>

static guint8 test_buffer[] = {
	0x01, 0x23, 0x45, 0x67, /* 0x01234567 MSB */
	0x67, 0x45, 0x23, 0x01, /* 0x01234567 LSB */
	0x00
};
static gsize tbsize = sizeof(test_buffer);

int main(int argc, char *argv[])
{
	G3DStream *stream;
	guint32 u;

	stream = g3d_stream_from_buffer(test_buffer, tbsize, "testbuf", FALSE);

	/* read 2 x 32-bit integer */
	u = g3d_stream_read_int32_be(stream);
	g_print("MSB value: 0x%08x\n", u);
	u = g3d_stream_read_int32_le(stream);
	g_print("LSB value: 0x%08x\n", u);
	/* seek back 4 bytes */
	g3d_stream_seek(stream, -4, G_SEEK_CUR);
	u = g3d_stream_read_int32_le(stream);
	g_print("LSB value: 0x%08x\n", u);
	/* rewind stream */
	g3d_stream_seek(stream, 0, G_SEEK_SET);
	u = g3d_stream_read_int32_be(stream);
	g_print("MSB value: 0x%08x\n", u);

	g3d_stream_close(stream);

	return 0;
}
