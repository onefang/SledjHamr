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

#ifndef __G3D_IFF_H__
#define __G3D_IFF_H__

/* FIXME: still needed for FILE until all plugins are converted
 * #ifndef G3D_DISABLE_DEPRECATED */
#include <stdio.h>
/* #endif */
#include <glib.h>

#include <g3d/types.h>

/**
 * SECTION:iff
 * @short_description: IFF file helper functions
 * @include: g3d/iff.h
 *
 * These are helper functions to read data from the Interchange File Format
 * (IFF).
 */


/**
 * G3D_IFF_PAD1:
 *
 * No padding is done after chunks.
 */
#define G3D_IFF_PAD1   0x01
/**
 * G3D_IFF_PAD2:
 *
 * Chunks are 2-byte aligned
 */
#define G3D_IFF_PAD2   0x02
/**
 * G3D_IFF_PAD4:
 *
 * Chunks are 4-byte aligned
 */
#define G3D_IFF_PAD4   0x04
/**
 * G3D_IFF_PAD8:
 *
 * Chunks are 8-byte aligned
 */
#define G3D_IFF_PAD8   0x08

/**
 * G3D_IFF_SUBCHUNK_LEN16:
 *
 * All chunks except the toplevel ones have 16-bit sizes.
 */
#define G3D_IFF_SUBCHUNK_LEN16   0x10
/**
 * G3D_IFF_LEN16:
 *
 * All chunks have 16-bit sizes.
 */
#define G3D_IFF_LEN16            0x20
/**
 * G3D_IFF_LE:
 *
 * The file has little-endian data.
 */
#define G3D_IFF_LE               0x40 /* little endian */

/**
 * G3D_IFF_MKID:
 * @a: first byte
 * @b: second byte
 * @c: third byte
 * @d: fourth byte
 *
 * Generate an IFF chunk identifier from character representation, e.g.
 * G3D_IFF_MKID('F','O','R','M').
 *
 * Returns: unsigned integer identifier.
 */
#define G3D_IFF_MKID(a,b,c,d) ( \
	(((guint32)(a))<<24) | \
	(((guint32)(b))<<16) | \
	(((guint32)(c))<< 8) | \
	(((guint32)(d))    ) )

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

#ifndef G3D_DISABLE_DEPRECATED
/**
 * g3d_iff_gdata:
 *
 * IFF global data (deprecated).
 */
#define g3d_iff_gdata G3DIffGlobal
/**
 * g3d_iff_ldata:
 *
 * IFF local data (deprecated).
 */
#define g3d_iff_ldata G3DIffLocal
/**
 * g3d_iff_chunk_callback:
 *
 * IFF chunk callback (deprecated).
 */
#define g3d_iff_chunk_callback G3DIffChunkCallback
/**
 * g3d_iff_chunk_info:
 *
 * IFF chunk description (deprecated).
 */
#define g3d_iff_chunk_info G3DIffChunkInfo
#endif

/**
 * G3DIffGlobal:
 * @context: a valid context
 * @model: a model
 * @stream: the stream to read model from
 * @flags: IFF flags
 * @user_data: to be used by plugin
 * @f: file to read model from (DEPRECATED)
 * @max_fpos: maximum file position (DEPRECATED)
 *
 * The plugin-global data to be given to IFF callback functions.
 */
typedef struct {
	G3DContext *context;
	G3DModel *model;
	G3DStream *stream;
	guint32 flags;
	gpointer user_data;
	FILE *f;
	long int max_fpos;
} G3DIffGlobal;

/**
 * G3DIffLocal:
 * @id: chunk identifier
 * @parent_id: parent chunk identifier
 * @object: an object set by parent callbacks, may be NULL
 * @level: level of chunk
 * @level_object: object shared by callbacks on the same level, may be NULL
 * @nb: number of bytes remaining in chunk, has to be decremented after
 * correctly after reading from stream
 * @finalize: for container chunks the callback function is called before
 * and after processing possible sub-chunks, the second time @finalize is set
 * to TRUE
 *
 * The function-local data for IFF callback functions.
 */
typedef struct {
	guint32 id;
	guint32 parent_id;
	gpointer object;
	gint32 level;
	gpointer level_object;
	gint32 nb;
	gboolean finalize;
} G3DIffLocal;

/**
 * G3DIffChunkCallback:
 * @global: the global data
 * @local: the local data
 *
 * IFF callback function prototype.
 *
 * Returns: TRUE on success, FALSE else.
 */
typedef gboolean (* G3DIffChunkCallback)(
	G3DIffGlobal *global, G3DIffLocal *local);

/**
 * G3DIffChunkInfo:
 * @id: identifier of chunk
 * @description: human-readable description of chunk type
 * @container: TRUE if this chunk contains sub-chunks
 * @callback: function to be called if such a chunk is found
 *
 * A chunk type description.
 */
typedef struct {
	gchar *id;
	gchar *description;
	gboolean container;
	G3DIffChunkCallback callback;
} G3DIffChunkInfo;

/**
 * g3d_iff_check:
 * @stream: stream containing IFF file to check
 * @id: top level ID (out)
 * @len: length of top level container (out)
 *
 * Checks a stream for a valid IFF signature and reads the top level container.
 *
 * Returns: TRUE on success (valid IFF), FALSE else
 */
EAPI
gboolean g3d_iff_check(G3DStream *stream, guint32 *id, gsize *len);

/**
 * g3d_iff_read_chunk:
 * @stream: stream to read from
 * @id: ID of chunk (out)
 * @len: length of chunk (excluding header) (out)
 * @flags: flags
 *
 * Reads one chunk header from an IFF file.
 *
 * Returns: real length of chunk including header and possible padding byte
 */
EAPI
gsize g3d_iff_read_chunk(G3DStream *stream, guint32 *id, gsize *len,
	guint32 flags);

#ifndef G3D_DISABLE_DEPRECATED
/**
 * g3d_iff_open:
 * @filename: file name of IFF file
 * @id: top level ID (out)
 * @len: length of top level container (out)
 *
 * Opens an IFF file, checks it and reads its top level container.
 *
 * Returns: the file pointer of open file or NULL in case of an error
 */
FILE *g3d_iff_open(const gchar *filename, guint32 *id, guint32 *len);

/**
 * g3d_iff_readchunk:
 * @f: the open IFF file pointer
 * @id: ID of chunk (out)
 * @len: length of chunk (excluding header) (out)
 * @flags: flags
 *
 * Reads one chunk header from an IFF file.
 *
 * Returns: real length of chunk including header and possible padding byte
 */
int g3d_iff_readchunk(FILE *f, guint32 *id, guint32 *len, guint32 flags);
#endif /* G3D_DISABLE_DEPRECATED */

/**
 * g3d_iff_id_to_text:
 * @id: an IFF identifier
 *
 * Get the text representation of an IFF chunk identifier.
 *
 * Returns: a newly allocated string containing the text identifier.
 */
EAPI
gchar *g3d_iff_id_to_text(guint32 id);

/**
 * g3d_iff_chunk_matches:
 * @id: IFF identifier
 * @tid: textual representation of identifier
 *
 * Check whether @id and @tid match.
 *
 * Returns: TRUE on match, FALSE else.
 */
EAPI
gboolean g3d_iff_chunk_matches(guint32 id, gchar *tid);

/**
 * g3d_iff_handle_chunk:
 * @global: global data
 * @plocal: local data of parent chunk, must not be NULL
 * @chunks: chunk description list
 * @flags: IFF flags
 *
 * Handle an IFF chunk based on chunk description.
 *
 * Returns: level object for siblings, may be NULL.
 */
EAPI
gpointer g3d_iff_handle_chunk(G3DIffGlobal *global, G3DIffLocal *plocal,
	G3DIffChunkInfo *chunks, guint32 flags);

/**
 * g3d_iff_read_ctnr:
 * @global: global data
 * @local: local data of current chunk, must not be NULL
 * @chunks: chunk description list
 * @flags: IFF flags
 *
 * Read subchunks in current chunk and handle them appropriately.
 *
 * Returns: TRUE on success, FALSE else.
 */
EAPI
gboolean g3d_iff_read_ctnr(G3DIffGlobal *global, G3DIffLocal *local,
	G3DIffChunkInfo *chunks, guint32 flags);

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif

