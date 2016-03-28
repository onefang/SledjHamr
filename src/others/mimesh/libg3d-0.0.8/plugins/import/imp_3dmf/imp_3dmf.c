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

#include <string.h>

#include <g3d/types.h>
#include <g3d/context.h>
#include <g3d/stream.h>
#include <g3d/object.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>
#include <g3d/material.h>
#include <g3d/iff.h>
#include <g3d/debug.h>

#include "imp_3dmf_chunks.h"

#define X3DMF_CHUNK_CHAR(id, shift) \
	((((id) >> (shift)) & 0xFF) == 0) ? \
	' ' : ((id) >> (shift)) & 0xFF

typedef struct {
	guint32 id;
	guint32 offset;
	guint32 type;
}
X3dmfTocEntry;

typedef struct {
	guint32 num_entries;
	X3dmfTocEntry *entries;
}
X3dmfToc;


static gboolean x3dmf_read_container(G3DStream *stream, guint32 length,
	G3DModel *model, G3DObject *object, guint32 level, X3dmfToc *toc,
	G3DContext *context);
static X3dmfToc *x3dmf_read_toc(G3DStream *stream, X3dmfToc *prev_toc,
	G3DContext *context);

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	guint32 id, flags, tocloc, pos;
	gsize len;
	guint16 ver_min, ver_maj;
	gchar txthead[10];
	X3dmfToc *toc = NULL;

	g3d_iff_read_chunk(stream, &id, &len, 0);
	if((id != G3D_IFF_MKID('3', 'D', 'M', 'F')) || (len != 16)) {
		g3d_stream_seek(stream, 0, G_SEEK_SET);
		g3d_stream_read(stream, txthead, 10);
		if(strncmp(txthead, "3DMetafile", 10) == 0) {
			g_warning("file %s is an ASCII 3D Metafile (unhandled)\n",
				stream->uri);
		} else {
			g_warning("file %s is not a 3D Metafile\n", stream->uri);
		}
		return FALSE;
	}

	ver_maj = g3d_stream_read_int16_be(stream);
	ver_min = g3d_stream_read_int16_be(stream);

	flags = g3d_stream_read_int32_be(stream);

	g3d_stream_skip(stream, 4); /* FIXME: 64bit file offsets */
	tocloc = g3d_stream_read_int32_be(stream);

	/* read TOC if available */
	if(tocloc > 0) {
		pos = g3d_stream_tell(stream);
		g3d_stream_seek(stream, tocloc, G_SEEK_SET);
		toc = x3dmf_read_toc(stream, NULL, context);
		g3d_stream_seek(stream, pos, G_SEEK_SET);
	}

#if DEBUG > 0
	g_debug("3DMF: version %d.%d (0x%08x) TOC @ 0x%08x",
		ver_maj, ver_min, flags, tocloc);
#endif

	x3dmf_read_container(stream, (guint32) -1, model, NULL, 0, toc, context);

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("3D Metafiles.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("b3d:3mf:3dmf", ":", 0);
}

/******************************************************************************
 * 3DMF specific
 */

static X3dmfChunkDesc *x3dmf_get_chunk_info(guint32 id)
{
	gint32 i;

	for(i = 0; x3dmf_chunks[i].id != 0; i ++)
		if(x3dmf_chunks[i].id == id)
			return &(x3dmf_chunks[i]);
	return NULL;
}

static X3dmfToc *x3dmf_read_toc(G3DStream *stream, X3dmfToc *prev_toc,
	G3DContext *context)
{
	X3dmfToc *toc;
	guint32 off_next_toc, typeseed, refseed, entrytype, entrysize, nentries;
	guint32 noff, i;

	if(prev_toc)
		toc = prev_toc;
	else
		toc = g_new0(X3dmfToc, 1);

	/* skip tag and size (FIXME) */
	g3d_stream_skip(stream, 8);

	g3d_stream_skip(stream, 4); /* FIXME: 64bit file offsets */
	off_next_toc = g3d_stream_read_int32_be(stream);
	typeseed = g3d_stream_read_int32_be(stream);
	refseed = g3d_stream_read_int32_be(stream);
	entrytype = g3d_stream_read_int32_be(stream);
	entrysize = g3d_stream_read_int32_be(stream);
	nentries = g3d_stream_read_int32_be(stream);

	/* resize entry array */
	noff = toc->num_entries;
	toc->num_entries += nentries;
	toc->entries = (X3dmfTocEntry *)g_realloc(toc->entries,
		toc->num_entries * sizeof(X3dmfTocEntry));

	/* read TOC entries */
	for(i = 0; i < nentries; i ++) {
		toc->entries[noff + i].id = g3d_stream_read_int32_be(stream);
		g3d_stream_skip(stream, 4); /* FIXME: 64bit file offsets */
		toc->entries[noff + i].offset = g3d_stream_read_int32_be(stream);

		if((entrytype == 1) && (entrysize == 16)) {
			toc->entries[noff + i].type = g3d_stream_read_int32_be(stream);
		}
#if DEBUG > 0
		g_debug("3DMF: TOC: %06d @ 0x%08x",
			toc->entries[noff + i].id,
			toc->entries[noff + i].offset);
#endif
	}

	/* read next toc */
	if(off_next_toc > 0) {
		g3d_stream_seek(stream, off_next_toc, SEEK_SET);
		toc = x3dmf_read_toc(stream, toc, context);
	}

	return toc;
}

static guint32 x3dmf_read_mesh(G3DStream *stream, G3DObject *object,
	G3DContext *context)
{
	guint32 i, j, nconts, nfaces, nbytes = 0, ncverts, offv;
	G3DFace *face;

	g_return_val_if_fail(object != NULL, FALSE);

	offv = object->vertex_count;
	object->vertex_count += g3d_stream_read_int32_be(stream);
	object->vertex_data = g_realloc(object->vertex_data,
		object->vertex_count * 3 * sizeof(G3DFloat));
	nbytes += 4;

	for(i = offv; i < object->vertex_count; i ++) {
		for(j = 0; j < 3; j ++)
			object->vertex_data[i * 3 + j] = g3d_stream_read_float_be(stream);
		nbytes += 12;

		g3d_context_update_interface(context);
	}

	nfaces = g3d_stream_read_int32_be(stream);
	nconts = g3d_stream_read_int32_be(stream);
	nbytes += 8;

#if DEBUG > 0
	g_debug("|%u verts, %u faces, %u conts", object->vertex_count,
		nfaces, nconts);
#endif

	for(i = 0; i < nfaces; i ++) {
		face = g_new0(G3DFace, 1);

		face->vertex_count = g3d_stream_read_int32_be(stream);
		nbytes += 4;
		face->vertex_indices = g_new0(guint32, face->vertex_count);

		for(j = 0; j < face->vertex_count; j ++) {
			face->vertex_indices[j] = offv + g3d_stream_read_int32_be(stream);
			nbytes += 4;
			if(face->vertex_indices[j] >= object->vertex_count) {
				g_warning("face index wrong: %u >= %u",
					face->vertex_indices[j], object->vertex_count);
				face->vertex_indices[j] = 0;
			}
		}

#if DEBUG > 3
		g_debug("|face %u: %u %u %u", i, face->vertex_indices[0],
			face->vertex_indices[1], face->vertex_indices[2]);
#endif

		face->material = g_slist_nth_data(object->materials, 0);
		object->faces = g_slist_prepend(object->faces, face);

		g3d_context_update_interface(context);
	}

	/* contours */
	for(i = 0; i < nconts; i ++) {
		ncverts = g3d_stream_read_int32_be(stream);
		nbytes += 4;
		for(j = 0; j < ncverts; j ++) {
			g3d_stream_read_int32_be(stream);
			nbytes += 4;
		}
	}

	return nbytes;
}

static G3DObject *x3dmf_object_new(G3DStream *stream, G3DModel *model)
{
	G3DObject *object;
	G3DMaterial *material;

	object = g_new0(G3DObject, 1);
	material = g3d_material_new();

	object->name = g_strdup_printf("container @ 0x%08x",
		(guint32)g3d_stream_tell(stream) - 8);
	model->objects = g_slist_append(model->objects, object);
	object->materials =	g_slist_append(object->materials, material);

	return object;
}

static guint32 x3dmf_read_packed(G3DStream *stream, guint32 maxx,
	guint32 *nread)
{
	if(maxx > 0xFFFE) {
		if(nread)
			(*nread) += 4;
		return g3d_stream_read_int32_be(stream);
	} else if(maxx > 0xFE) {
		if(nread)
			(*nread) += 2;
		return g3d_stream_read_int16_be(stream);
	} else {
		if(nread)
			(*nread) += 1;
		return g3d_stream_read_int8(stream);
	}
}

/*
 [tmsh] - TriMesh
 http://developer.apple.com/documentation/QuickTime/QD3D/qd3dmetafile.33.htm

 Uns32               numTriangles
 Uns32               numTriangleAttributeTypes
 Uns32               numEdges
 Uns32               numEdgeAttributeTypes
 Uns32               numPoints
 Uns32               numVertexAttributeTypes
 TriMeshTriangleData         triangles[numTriangles]
 TriMeshEdgeData             edges[numEdges]
 Point3D                     points[numPoints]
 BoundingBox                 bBox
*/

static guint32 x3dmf_read_tmsh(G3DStream *stream, G3DObject *object,
	G3DContext *context)
{
	G3DFace *face;
	guint32 nread = 0, nfaces, nverts, nedges;
	gint32 i, j;

	nfaces = g3d_stream_read_int32_be(stream); /* numTriangles */
	nread += 4;

	g3d_stream_read_int32_be(stream); /* numTriangleAttributeTypes */
	nread += 4;

	nedges = g3d_stream_read_int32_be(stream); /* numEdges */
	nread += 4;

	g3d_stream_read_int32_be(stream); /* numEdgeAttributeTypes */
	nread += 4;

	nverts = g3d_stream_read_int32_be(stream); /* numPoints */
	nread += 4;

	g3d_stream_read_int32_be(stream); /* numVertexAttributeTypes */
	nread += 4;

#if DEBUG > 3
	g_debug("3DMF: [tmsh] %d faces, %d edges, %d vertices",
		nfaces, nedges, nverts);
#endif

	/* triangles */
	for(i = 0; i < nfaces; i ++) {
		face = g_new0(G3DFace, 1);

		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		for(j = 0; j < 3; j ++) {
			face->vertex_indices[j] =
				x3dmf_read_packed(stream, nfaces, &nread);
			if(face->vertex_indices[j] >= nverts) {
				g_warning("face index error: %u >= %u",
					face->vertex_indices[j], nverts);
				face->vertex_indices[j] = 0;
			}
		}

#if DEBUG > 3
		g_debug("face %u (packed): %u %u %u", i, face->vertex_indices[0],
			face->vertex_indices[1], face->vertex_indices[2]);
#endif

		face->material = g_slist_nth_data(object->materials, 0);
		object->faces = g_slist_prepend(object->faces, face);
	}

	/* edges */
	for(i = 0; i < nedges; i ++) {
		/* pointIndices */
		x3dmf_read_packed(stream, nedges, &nread);
		x3dmf_read_packed(stream, nedges, &nread);
		/* triangleIndices */
		x3dmf_read_packed(stream, nedges, &nread);
		x3dmf_read_packed(stream, nedges, &nread);
	}

	/* points */
	object->vertex_count = nverts;
	object->vertex_data = g_new0(G3DFloat, 3 * nverts);
	for(i = 0; i < nverts; i ++) {
		object->vertex_data[i * 3 + 0] = g3d_stream_read_float_be(stream);
		object->vertex_data[i * 3 + 1] = g3d_stream_read_float_be(stream);
		object->vertex_data[i * 3 + 2] = g3d_stream_read_float_be(stream);
		nread += 12;
	}

	/* bBox */
	/* Point3D min */
	g3d_stream_read_float_be(stream);
	g3d_stream_read_float_be(stream);
	g3d_stream_read_float_be(stream);
	nread += 12;
	/* Point3D max */
	g3d_stream_read_float_be(stream);
	g3d_stream_read_float_be(stream);
	g3d_stream_read_float_be(stream);
	nread += 12;
	/* boolean isEmpty */
	g3d_stream_read_int32_be(stream);
	nread += 4;

	return nread;
}

static gboolean x3dmf_read_rfrn(G3DStream *stream, G3DModel *model,
	X3dmfToc *toc, G3DContext *context)
{
	G3DObject *object;
	guint32 id, i, refid, savedoffset;
	gsize len;
	X3dmfTocEntry *tocentry = NULL;

	refid = g3d_stream_read_int32_be(stream);
	if(refid == 0) {
		/* FIXME */
		return FALSE;
	}

	if(toc == NULL) {
		return FALSE;
	}

	/* find reference object */
	for(i = 0; i < toc->num_entries; i ++)
		if(toc->entries[i].id == refid)
			tocentry = &(toc->entries[i]);

	g_return_val_if_fail(tocentry != NULL, FALSE);

	savedoffset = g3d_stream_tell(stream);
	g3d_stream_seek(stream, tocentry->offset, G_SEEK_SET);

	object = x3dmf_object_new(stream, model);

	g3d_iff_read_chunk(stream, &id, &len, 0);
	switch(id) {
		case G3D_IFF_MKID('c', 't', 'n', 'r'):
			x3dmf_read_container(stream, len, model, NULL, 0xFF, toc, context);
			break;

		default:
			break;
	}

	g3d_stream_seek(stream, savedoffset, G_SEEK_SET);

	return TRUE;
}

static gboolean x3dmf_read_container(G3DStream *stream, guint32 length,
	G3DModel *model, G3DObject *object, guint32 level, X3dmfToc *toc,
	G3DContext *context)
{
	G3DMaterial *material = NULL;
	X3dmfChunkDesc *chunkdesc;
	guint32 id, chk, i;
	gsize len;
	G3DFloat matrix[16];

	g3d_matrix_identity(matrix);

	while(length > 0) {
		if(g3d_stream_eof(stream))
			break;

		g3d_iff_read_chunk(stream, &id, &len, 0);
		length -= 8;

		if(id == 0)
			return FALSE;

		chunkdesc = x3dmf_get_chunk_info(id);

#if DEBUG > 0
		g_debug("\\%s[%c%c%c%c]: %s (%d bytes)", debug_pad(level),
			X3DMF_CHUNK_CHAR(id, 24), X3DMF_CHUNK_CHAR(id, 16),
			X3DMF_CHUNK_CHAR(id, 8), X3DMF_CHUNK_CHAR(id, 0),
			chunkdesc ? chunkdesc->description : "unknown chunk",
			len);
#endif
		length -= len;

		switch(id) {
			case G3D_IFF_MKID('c', 'n', 't', 'r'):
				/* container */
#if DEBUG > 0
				g_debug("|%snew container @ 0x%x (%d bytes)",
					debug_pad(level - 1),
					(guint32)g3d_stream_tell(stream) - 8, len);
#endif
				x3dmf_read_container(stream, len, model, object, level + 1,
					toc, context);
				break;

			case G3D_IFF_MKID('k', 'd', 'i', 'f'):
				/* diffuse color */
				if(object) {
#if DEBUG > 2
					g_debug("3DMF: kdif: got object");
#endif
					material = g_slist_nth_data(object->materials, 0);
					material->r = g3d_stream_read_float_be(stream);
					material->g = g3d_stream_read_float_be(stream);
					material->b = g3d_stream_read_float_be(stream);
				} else {
					g3d_stream_skip(stream, len);
				}
				break;

			case G3D_IFF_MKID('k', 's', 'p', 'c'):
				/* specular color */
				if(object) {
#if DEBUG > 2
					g_debug("3DMF: kspc: got object");
#endif
					material = g_slist_nth_data(object->materials, 0);
					material->specular[0] = g3d_stream_read_float_be(stream);
					material->specular[1] = g3d_stream_read_float_be(stream);
					material->specular[2] = g3d_stream_read_float_be(stream);
				} else {
					g3d_stream_skip(stream, len);
				}
				break;

			case G3D_IFF_MKID('k', 'x', 'p', 'r'):
				/* transparency color */
				if(object) {
					/* use average as alpha */
					material = g_slist_nth_data(object->materials, 0);
					material->a = 1.0 -
						(g3d_stream_read_float_be(stream) +
						g3d_stream_read_float_be(stream) +
						g3d_stream_read_float_be(stream)) / 3.0;

					if(material->a < 0.1)
						material->a = 0.1;
				} else {
					g3d_stream_skip(stream, len);
				}
				break;

			case G3D_IFF_MKID('m', 'e', 's', 'h'):
				/* mesh */
				if(object == NULL)
					object = x3dmf_object_new(stream, model);
				material = g_slist_nth_data(object->materials, 0);

				chk = x3dmf_read_mesh(stream, object, context);
				g3d_object_transform(object, matrix);
				if(chk != len) {
					g_warning("3DMF: mesh: wrong length (%u != %u)\n",
						chk, (unsigned int) len);
					return FALSE;
				}
				break;

			case G3D_IFF_MKID('m', 't', 'r', 'x'):
				/* matrix */
				for(i = 0; i < 16; i ++)
					matrix[i] = g3d_stream_read_float_be(stream);
				if(object) {
#if DEBUG > 2
					g_debug("3DMF: mtrx: object is set");
#endif
					g3d_object_transform(object, matrix);
				}
#if DEBUG > 3
				for(i = 0; i < 4; i ++)
					g_debug("3DMF: mtrx: %+1.2f %+1.2f %+1.2f %+1.2f",
						matrix[i * 4 + 0], matrix[i * 4 + 1],
						matrix[i * 4 + 2], matrix[i * 4 + 3]);
#endif
				break;

			case G3D_IFF_MKID('r', 'f', 'r', 'n'):
				/* reference */
				x3dmf_read_rfrn(stream, model, toc, context);
				break;

			case G3D_IFF_MKID('s', 'e', 't', ' '):
				/* ??: skip this cntr chunk */
				g3d_stream_skip(stream, length);
				length = 0;
				break;

			case G3D_IFF_MKID('t', 'm', 's', 'h'):
				/* triangle mesh */
				if(object == NULL)
					object = x3dmf_object_new(stream, model);
				material = g_slist_nth_data(object->materials, 0);

				chk = x3dmf_read_tmsh(stream, object, context);
				g3d_object_transform(object, matrix);
				if(chk != len) {
#if DEBUG > 0
					g_debug("3DMF: tmsh: offset %d bytes", len - chk);
#endif
					g3d_stream_skip(stream, len - chk);
				}
				break;

			case G3D_IFF_MKID('t', 'r', 'n', 's'):
				/* translate */
				if(object) {
					G3DFloat x,y,z;
					G3DFloat matrix[16];

					x = g3d_stream_read_float_be(stream);
					y = g3d_stream_read_float_be(stream);
					z = g3d_stream_read_float_be(stream);

					g3d_matrix_identity(matrix);
					g3d_matrix_translate(x, y, z, matrix);

					g3d_object_transform(object, matrix);
				} else {
#if DEBUG > 0
					g_warning("3DMF: [trns] no object");
#endif
					g3d_stream_skip(stream, 12);
				}
				break;

			default:
				if(chunkdesc) {
					g3d_stream_skip(stream, len);
				} else {
#if DEBUG > 0
				g_warning("3DMF: Container: unknown chunk '%c%c%c%c'/"
					"0x%02X%02X%02X%02X @ 0x%08x "
					"(%d bytes)",
					X3DMF_CHUNK_CHAR(id, 24), X3DMF_CHUNK_CHAR(id, 16),
					X3DMF_CHUNK_CHAR(id, 8), X3DMF_CHUNK_CHAR(id, 0),
					X3DMF_CHUNK_CHAR(id, 24), X3DMF_CHUNK_CHAR(id, 16),
					X3DMF_CHUNK_CHAR(id, 8), X3DMF_CHUNK_CHAR(id, 0),
					(guint32)g3d_stream_tell(stream) - 8, len);
#endif
					g3d_stream_skip(stream, len);
				}
				break;
		}
	}

	return TRUE;
}

