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
#include <g3d/types.h>

gint32 g3d_read_int8(FILE *f)
{
	int c = fgetc(f);
	if(c == EOF)
		return 0;
	else
		return c;
}

gint32 g3d_read_int16_be(FILE *f)
{
	return (g3d_read_int8(f)<<8) | g3d_read_int8(f);
}

gint32 g3d_read_int16_le(FILE *f)
{
	return g3d_read_int8(f) | (g3d_read_int8(f)<<8);
}

gint32 g3d_read_int32_be(FILE *f)
{
	return (g3d_read_int8(f) << 24) | (g3d_read_int8(f) << 16) |
		(g3d_read_int8(f) << 8) | g3d_read_int8(f);
}

gint32 g3d_read_int32_le(FILE *f)
{
	return  g3d_read_int8(f) | (g3d_read_int8(f) << 8) |
		(g3d_read_int8(f) << 16) | (g3d_read_int8(f) << 24);
}

static void g3d_read_bytes(FILE *f, guint8 *buf, gsize n)
{
	gint32 i;
	for(i = 0; i < n; i ++)
		buf[i] = g3d_read_int8(f);
}

static void g3d_read_bytes_swap(FILE *f, guint8 *buf, gsize n)
{
	gint32 i;
	for(i = (n - 1); i >= 0; i --)
		buf[i] = g3d_read_int8(f);
}

G3DFloat g3d_read_float_be(FILE *f)
{
	union {
		G3DFloat f;
		guint8 u[4];
	} u;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	g3d_read_bytes_swap(f, u.u, 4);
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	g3d_read_bytes(f, u.u, 4);
#endif
	return u.f;
}

G3DFloat g3d_read_float_le(FILE *f)
{
	union {
		G3DFloat f;
		guint8 u[4];
	} u;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	g3d_read_bytes(f, u.u, 4);
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	g3d_read_bytes_swap(f, u.u, 4);
#endif
	return u.f;
}

G3DDouble g3d_read_double_be(FILE *f)
{
	union {
		G3DDouble d;
		guint8 u[8];
	} u;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	g3d_read_bytes_swap(f, u.u, 8);
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	g3d_read_bytes(f, u.u, 8);
#endif
	return u.d;
}

G3DDouble g3d_read_double_le(FILE *f)
{
	union {
		G3DDouble d;
		guint8 u[8];
	} u;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	g3d_read_bytes(f, u.u, 8);
#elif G_BYTE_ORDER == G_BIG_ENDIAN
	g3d_read_bytes_swap(f, u.u, 8);
#endif
	return u.d;
}

gint32 g3d_read_cstr(FILE *f, gchar *buffer, gint32 max_len)
{
	gint32 n = 0;
	gchar c;

	do
	{
		c = g3d_read_int8(f);
		buffer[n] = c;
		n ++;
	}
	while((c != 0) && (n < max_len));

	buffer[max_len - 1] = '\0';

	return n;
}
