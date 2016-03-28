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

#ifndef __G3D_READ_H__
#define __G3D_READ_H__

#ifndef G3D_DISABLE_DEPRECATED

#include <stdio.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:read
 * @short_description: file reading abstraction (deprecated)
 * @see_also: g3d_stream_open_file
 * @include: g3d/read.h
 *
 * The g3d_read_* family of functions is deprecated in favour of the #G3DStream
 * interface.
 */


/**
 * g3d_read_int8:
 * @f: the file to read from
 *
 * Read a 1 byte signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
gint32 g3d_read_int8(FILE *f);

/**
 * g3d_read_int16_be:
 * @f: the file to read from
 *
 * Read a 2 byte big-endian signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
gint32 g3d_read_int16_be(FILE *f);

/**
 * g3d_read_int16_le:
 * @f: the file to read from
 *
 * Read a 2 byte little-endian signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
gint32 g3d_read_int16_le(FILE *f);

/**
 * g3d_read_int32_be:
 * @f: the file to read from
 *
 * Read a 4 byte big-endian signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
gint32 g3d_read_int32_be(FILE *f);

/**
 * g3d_read_int32_le:
 * @f: the file to read from
 *
 * Read a 4 byte little-endian signed integer from file.
 *
 * Returns: The read value, 0 in case of error
 */
gint32 g3d_read_int32_le(FILE *f);

/**
 * g3d_read_float_be:
 * @f: the file to read from
 *
 * Read a 4 byte big-endian floating point number from file.
 *
 * Returns: The read value, 0 in case of error
 */
G3DFloat g3d_read_float_be(FILE *f);

/**
 * g3d_read_float_le:
 * @f: the file to read from
 *
 * Read a 4 byte little-endian floating point number from file.
 *
 * Returns: The read value, 0 in case of error
 */
G3DFloat g3d_read_float_le(FILE *f);

/**
 * g3d_read_double_be:
 * @f: the file to read from
 *
 * Read a 8 byte big-endian double-precision floating point number from file.
 *
 * Returns: The read value, 0 in case of error
 */
G3DDouble g3d_read_double_be(FILE *f);

/**
 * g3d_read_double_le:
 * @f: the file to read from
 *
 * Read a 8 byte little-endian double-precision floating point number from
 * file.
 *
 * Returns: The read value, 0 in case of error
 */
G3DDouble g3d_read_double_le(FILE *f);

/**
 * g3d_read_cstr:
 * @f: the file to read from
 * @buffer: the buffer to store line in
 * @max_len: maximum length of string including termination character
 *
 * Read a zero-terminated string from file.
 *
 * Returns: number of bytes read from file.
 */
gint32 g3d_read_cstr(FILE *f, gchar *buffer, gint32 max_len);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* G3D_DISABLE_DEPRECATED */

#endif /* __G3D_READ_H__ */

