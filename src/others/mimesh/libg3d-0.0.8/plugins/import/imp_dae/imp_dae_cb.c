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

#include <g3d/config.h>

#include <g3d/types.h>
#include <g3d/material.h>
#include <g3d/matrix.h>
#include <g3d/object.h>
#include <g3d/texture.h>
#include <g3d/stream.h>

#include "imp_dae_cb.h"
#include "imp_dae_chunks.h"
#include "imp_dae_xml.h"

typedef struct {
	guint32 offset;
	enum {
		SEM_UNKNOWN,
		SEM_VERTEX,
		SEM_NORMAL,
		SEM_TEXCOORD
	} semantic;
	gchar *source;
} DaeInput;

/*****************************************************************************/

static G3DMaterial *dae_get_material_by_name(DaeGlobalData *global,
	const gchar *id, guint32 level)
{
	G3DMaterial *material;
	GSList *mitem;
	xmlNodePtr matnode;

	/* try to find material */
	for(mitem = global->model->materials; mitem != NULL; mitem = mitem->next) {
		material = (G3DMaterial *)mitem->data;
		if(strcmp(material->name, id) == 0)
			return material;
	}

	material = g3d_material_new();
	material->name = g_strdup(id);
	global->model->materials = g_slist_append(global->model->materials,
		material);

	/* find material in library */
	matnode = dae_library_lookup(global->lib, "symbol", id);

	if(matnode) {
		dae_xml_parse(global, matnode, dae_chunks_material,
			level, material);
	}
	return material;
}

static GSList *dae_get_inputs(xmlNodePtr node)
{
	xmlNodePtr inode = NULL;
	gchar *soff, *ssem;
	DaeInput *input;
	GSList *inputs = NULL;

	while(dae_xml_next_child_by_tagname(node, &inode, "input")) {
		input = g_new0(DaeInput, 1);
		soff = dae_xml_get_attr(inode, "offset");
		if(soff) {
			input->offset = atoi(soff);
			g_free(soff);
		}
		ssem = dae_xml_get_attr(inode, "semantic");
		if(ssem) {
			if(strcmp(ssem, "VERTEX") == 0)
				input->semantic = SEM_VERTEX;
			else if(strcmp(ssem, "NORMAL") == 0)
				input->semantic = SEM_NORMAL;
			else if(strcmp(ssem, "TEXCOORD") == 0)
				input->semantic = SEM_TEXCOORD;
			else {
				g_warning("DAE: unknown input semantic '%s'", ssem);
				input->semantic = SEM_UNKNOWN;
			}
			g_free(ssem);
		}
		input->source = dae_xml_get_attr(inode, "source");
		inputs = g_slist_append(inputs, input);
	}
	return inputs;
}

static void dae_inputs_free(GSList *inputs)
{
	GSList *item;
	DaeInput *input;

	for(item = inputs; item != NULL; item = item->next) {
		input = (DaeInput *)item->data;
		if(input->source)
			g_free(input->source);
		g_free(input);
		item->data = NULL;
	}
	g_slist_free(inputs);
}

static gboolean dae_load_source(DaeLibrary *lib, gchar *id,
	G3DFloat **asrc, guint32 *nsrc)
{
	xmlNodePtr snode, fnode;
	gchar *scnt, *next = NULL;
	gint i;

	snode = dae_library_lookup(lib, "source", id + 1);
#if DEBUG > 0
	g_debug("DAE: source '%s': %p", id + 1, (void *)snode);
#endif
	if(snode == NULL)
		return FALSE;

	fnode = dae_xml_get_child_by_tagname(snode, "float_array");
	if(fnode == NULL)
		return FALSE;

	scnt = dae_xml_get_attr(fnode, "count");
#if DEBUG > 0
	g_debug("DAE: float_array count=\"%s\"", scnt);
#endif
	if(scnt == NULL)
		return FALSE;
	*nsrc = atoi(scnt);
	g_free(scnt);
	if(*nsrc == 0)
		return FALSE;

	*asrc = g_new0(G3DFloat, *nsrc);
	for(i = 0; i < *nsrc; i ++)
#if G3D_FLOAT_IS_DOUBLE
		if(!dae_xml_next_double(fnode, &next, &((*asrc)[i])))
#else
		if(!dae_xml_next_float(fnode, &next, &((*asrc)[i])))
#endif
			return FALSE;

	return TRUE;
}

/*****************************************************************************/

gboolean dae_cb_bind_material(DaeGlobalData *global, DaeLocalData *local)
{
	return dae_xml_parse(global, local->node, dae_chunks_bind_material,
		local->level, local->user_data);
}

gboolean dae_cb_effect(DaeGlobalData *global, DaeLocalData *local)
{
	return dae_xml_parse(global, local->node, dae_chunks_effect,
		local->level, local->user_data);
}

gboolean dae_cb_geometry(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object;
	G3DMaterial *material;

	object = (G3DObject *)local->user_data;
	g_return_val_if_fail(object != NULL, FALSE);

	material = g3d_material_new();
	material->name = g_strdup("(default material)");
	object->materials = g_slist_append(object->materials, material);

	if(local->instance) {
		/* parse original node */
		dae_xml_parse(global, local->instance,
			dae_chunks_geometry, local->level, object);
	}

	/* parse instanced stuff */
	return dae_xml_parse(global, local->node, dae_chunks_geometry,
		local->level, object);
}

gboolean dae_cb_matrix(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object = local->user_data;
	G3DTransformation *transform;
	gchar *next = NULL;
	gint i;

	g_return_val_if_fail(object != NULL, FALSE);

	transform = object->transformation;
	if(transform == NULL) {
		transform = g_new0(G3DTransformation, 1);
		g3d_matrix_identity(transform->matrix);
		object->transformation = transform;
	}

	for(i = 0; i < 16; i ++)
#if G3D_FLOAT_IS_DOUBLE
		dae_xml_next_double(local->node, &next, transform->matrix + i);
#else
		dae_xml_next_float(local->node, &next, transform->matrix + i);
#endif
	g3d_matrix_transpose(transform->matrix);
#if DEBUG > 3
	g_debug("DAE: matrix for '%s':", object->name);
	g3d_matrix_dump(transform->matrix);
#endif
	return TRUE;
}

gboolean dae_cb_mesh(DaeGlobalData *global, DaeLocalData *local)
{
	return dae_xml_parse(global, local->node, dae_chunks_mesh,
		local->level, local->user_data);
}

gboolean dae_cb_newparam(DaeGlobalData *global, DaeLocalData *local)
{
	G3DMaterial *material = (G3DMaterial *)local->user_data;
	G3DStream *imgstream = NULL;
	xmlNodePtr n1, n2;
	gchar *siid = NULL, *filename, *subfile;
#ifdef HAVE_LIBGSF
	gchar *container, *pipe;
#endif

	g_return_val_if_fail(material != NULL, FALSE);

	n1 = dae_xml_get_child_by_tagname(local->node, "surface");
	if(n1 != NULL) {
		n2 = dae_xml_get_child_by_tagname(n1, "init_from");
		if(n2 != NULL)
			siid = g_strdup((gchar *)n2->children->content);
	}
	if(siid == NULL)
		return FALSE;

	n1 = dae_library_lookup(global->lib, "image", siid);
	g_free(siid);
	if(n1 == NULL)
		return FALSE;

	n2 = dae_xml_get_child_by_tagname(n1, "init_from");
	if(n2 == NULL)
		return FALSE;

	filename = (gchar *)n2->children->content;

	if(strncmp(global->stream->uri, "zip://", 6) == 0) {
#ifdef HAVE_LIBGSF
		/* .dae was loaded from .kmz, get texture from .kmz, too */
		pipe = strchr(global->stream->uri, '|');
		if(pipe != NULL) {
			container = g_strndup(global->stream->uri + 6,
				(pipe - global->stream->uri) - 6);
			subfile = filename;
			while(strncmp(subfile, "../", 3) == 0)
				subfile += 3;
			g_debug("DAE: loading '%s' from: '%s'", subfile, container);
			imgstream = g3d_stream_open_zip_from_stream(global->stream->zip_container, subfile);
		}
#endif
	} else {
		imgstream = g3d_stream_open_file(filename, "rb");
		if(imgstream == NULL) {
			/* if opened from unpacked doc.kml, textures are found in
			 * ../images/, so strip leading ../ */
			subfile = filename;
			while(strncmp(subfile, "../", 3) == 0)
				subfile += 3;
			imgstream = g3d_stream_open_file(subfile, "rb");
		}
	}

	if(imgstream != NULL) {
		material->tex_image = g3d_texture_load_from_stream(global->context,
			global->model, imgstream);
		if(material->tex_image) {
			material->tex_image->tex_env = G3D_TEXENV_DECAL;
		}
		g3d_stream_close(imgstream);
		return TRUE;
	}

	return FALSE;
}

gboolean dae_cb_node(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object, *pobject;
	G3DMatrix matrix[16];
	gchar *name;

	name = dae_xml_get_attr(local->node, "name");
	if(name == NULL)
		name = dae_xml_get_attr(local->node, "id");
	if(name == NULL)
		return FALSE;

	pobject = (G3DObject *)local->user_data;
	object = g_new0(G3DObject, 1);
	object->name = name;
	if(pobject)
		pobject->objects = g_slist_append(pobject->objects, object);
	else
		global->model->objects = g_slist_append(global->model->objects,
			object);

	if(dae_xml_parse(global, local->node, dae_chunks_node, local->level,
		object)) {
		if(object->transformation) {
			g3d_object_transform(object, object->transformation->matrix);
#if DEBUG > 3
			g_debug("DAE: transforming object '%s'", object->name);
			g3d_matrix_dump(object->transformation->matrix);
#endif
			g_free(object->transformation);
			object->transformation = NULL;
		}

		if(!pobject) {
			g3d_matrix_identity(matrix);
			g3d_matrix_rotate(-90.0 * G_PI / 180.0, 1.0, 0.0, 0.0, matrix);
			g3d_object_transform(object, matrix);
		}
		return TRUE;
	}
	return FALSE;
}

gboolean dae_cb_phong(DaeGlobalData *global, DaeLocalData *local)
{
	G3DMaterial *material = (G3DMaterial *)local->user_data;
	xmlNodePtr n1, n2;
	gchar *next;

	g_return_val_if_fail(material != NULL, FALSE);

	/* diffuse */
	n1 = dae_xml_get_child_by_tagname(local->node, "diffuse");
	if(n1 != NULL) {
		n2 = dae_xml_get_child_by_tagname(n1, "color");
		if(n2 != NULL) {
			next = NULL;
#if G3D_FLOAT_IS_DOUBLE
			dae_xml_next_double(n2, &next, &(material->r));
			dae_xml_next_double(n2, &next, &(material->g));
			dae_xml_next_double(n2, &next, &(material->b));
			dae_xml_next_double(n2, &next, &(material->a));
#else
			dae_xml_next_float(n2, &next, &(material->r));
			dae_xml_next_float(n2, &next, &(material->g));
			dae_xml_next_float(n2, &next, &(material->b));
			dae_xml_next_float(n2, &next, &(material->a));
#endif
		}
	}

	/* specular */
	n1 = dae_xml_get_child_by_tagname(local->node, "specular");
	if(n1 != NULL) {
		n2 = dae_xml_get_child_by_tagname(n1, "color");
		if(n2 != NULL) {
			next = NULL;
			/* These are floats either way. */
			dae_xml_next_float(n2, &next, &(material->specular[0]));
			dae_xml_next_float(n2, &next, &(material->specular[1]));
			dae_xml_next_float(n2, &next, &(material->specular[2]));
			dae_xml_next_float(n2, &next, &(material->specular[3]));
		}
	}

	return TRUE;
}

gboolean dae_cb_polylist(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object = local->user_data;
	G3DFace *face;
	G3DMaterial *material;
	xmlNodePtr pnode, vnode;
	gchar *scnt, *smat, *nextp = NULL, *nextv = NULL;
	guint32 count, normal_count, tex_count, flags = 0;
	gint i, j, nv, tmp;
	GSList *inputs, *item;
	DaeInput *input;
	G3DFloat *normal_data = NULL, *tex_data = NULL;

	g_return_val_if_fail(object != NULL, FALSE);

	scnt = dae_xml_get_attr(local->node, "count");
	g_return_val_if_fail(scnt != NULL, FALSE);
	count = atoi(scnt);
	g_free(scnt);
	g_return_val_if_fail(count != 0, FALSE);

	pnode = dae_xml_get_child_by_tagname(local->node, "p");
	vnode = dae_xml_get_child_by_tagname(local->node, "vcount");
	g_return_val_if_fail((pnode != NULL) && (vnode != NULL), FALSE);

	/* material */
	material = g_slist_nth_data(object->materials, 0);
	smat = dae_xml_get_attr(local->node, "material");
	if(smat != NULL) {
		material = dae_get_material_by_name(global, smat, local->level);
		g_free(smat);
	}

	/* get all inputs */
	inputs = dae_get_inputs(local->node);
	for(item = inputs; item != NULL; item = item->next) {
		input = (DaeInput *)item->data;
		if(input->semantic == SEM_NORMAL)
			if(dae_load_source(global->lib, input->source,
				&normal_data, &normal_count))
				flags |= G3D_FLAG_FAC_NORMALS;
		if(input->semantic == SEM_TEXCOORD)
			if(dae_load_source(global->lib, input->source,
				&tex_data, &tex_count) && (material->tex_image != NULL))
				flags |= G3D_FLAG_FAC_TEXMAP;
	}

	for(i = 0; i < count; i ++) {
		if(dae_xml_next_int(vnode, &nextv, &nv) && (nv > 0)) {
			face = g_new0(G3DFace, 1);
			face->vertex_count = nv;
			face->vertex_indices = g_new0(guint32, nv);
			face->material = material;
			face->flags = flags;
			object->faces = g_slist_append(object->faces, face);

			if(face->flags & G3D_FLAG_FAC_NORMALS) {
				face->normals = g_new0(G3DFloat, nv * 3);
			}
			if(face->flags & G3D_FLAG_FAC_TEXMAP) {
				face->tex_image = material->tex_image;
				face->tex_vertex_count = nv;
				face->tex_vertex_data = g_new0(G3DFloat, nv * 2);
			}

			for(j = 0; j < nv; j ++) {
				for(item = inputs; item != NULL; item = item->next) {
					input = (DaeInput *)item->data;
					dae_xml_next_int(pnode, &nextp, &tmp);
					switch(input->semantic) {
						case SEM_VERTEX:
							face->vertex_indices[j] = tmp;
							if(face->vertex_indices[j] >= object->vertex_count)
							{
								g_warning("polylist: [%s] face[%d] (%d) >= %d",
									object->name, j, face->vertex_indices[j],
									object->vertex_count);
								face->vertex_indices[j] = 0;
							}
							break;
						case SEM_NORMAL:
							if(flags & G3D_FLAG_FAC_NORMALS) {
								face->normals[j * 3 + 0] =
									normal_data[tmp * 3 + 0];
								face->normals[j * 3 + 1] =
									normal_data[tmp * 3 + 1];
								face->normals[j * 3 + 2] =
									normal_data[tmp * 3 + 2];
							}
							break;
						case SEM_TEXCOORD:
							if(flags & G3D_FLAG_FAC_TEXMAP) {
								face->tex_vertex_data[j * 2 + 0] =
									tex_data[tmp * 2 + 0];
								face->tex_vertex_data[j * 2 + 1] = 1.0 -
									tex_data[tmp * 2 + 1];
							}
							break;
						case SEM_UNKNOWN:
							break;
					}
				} /* inputs */
			}
#if DEBUG > 3
			g_debug("DAE: %-2i, %-2i, %-2i, (%-2i)",
				face->vertex_indices[0],
				face->vertex_indices[1],
				face->vertex_indices[2],
				((nv > 3) ? face->vertex_indices[3] : -1));
#endif
		}
	}

	if(tex_data)
		g_free(tex_data);
	if(normal_data)
		g_free(normal_data);
	dae_inputs_free(inputs);

	return TRUE;
}

gboolean dae_cb_profile_COMMON(DaeGlobalData *global, DaeLocalData *local)
{
	return dae_xml_parse(global, local->node, dae_chunks_profile_COMMON,
		local->level, local->user_data);
}

gboolean dae_cb_rotate(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object = local->user_data;
	G3DTransformation *transform;
	G3DFloat x = 0.0, y = 0.0, z = 0.0, a = 0.0, m[16];
	gchar *next = NULL;

	g_return_val_if_fail(object != NULL, FALSE);

	transform = object->transformation;
	if(transform == NULL) {
		transform = g_new0(G3DTransformation, 1);
		g3d_matrix_identity(transform->matrix);
		object->transformation = transform;
	}

#if G3D_FLOAT_IS_DOUBLE
	dae_xml_next_double(local->node, &next, &x);
	dae_xml_next_double(local->node, &next, &y);
	dae_xml_next_double(local->node, &next, &z);
	dae_xml_next_double(local->node, &next, &a);
#else
	dae_xml_next_float(local->node, &next, &x);
	dae_xml_next_float(local->node, &next, &y);
	dae_xml_next_float(local->node, &next, &z);
	dae_xml_next_float(local->node, &next, &a);
#endif
	g_return_val_if_fail((x != 0.0) || (y != 0.0) || (z != 0.0), FALSE);
	g3d_matrix_rotate(a, x, y, z, m);
	g3d_matrix_multiply(transform->matrix, m, transform->matrix);
#if DEBUG > 3
	g_debug("DAE: rotation for '%s': %.2f, %.2f, %.2f - %.2f", object->name,
		x, y, z, a);
	g3d_matrix_dump(transform->matrix);
#endif
	return TRUE;
}

gboolean dae_cb_scale(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object = local->user_data;
	G3DTransformation *transform;
	G3DFloat x = 0.0, y = 0.0, z = 0.0;
	gchar *next = NULL;

	g_return_val_if_fail(object != NULL, FALSE);

	transform = object->transformation;
	if(transform == NULL) {
		transform = g_new0(G3DTransformation, 1);
		g3d_matrix_identity(transform->matrix);
		object->transformation = transform;
	}

#if G3D_FLOAT_IS_DOUBLE
	dae_xml_next_double(local->node, &next, &x);
	dae_xml_next_double(local->node, &next, &y);
	dae_xml_next_double(local->node, &next, &z);
#else
	dae_xml_next_float(local->node, &next, &x);
	dae_xml_next_float(local->node, &next, &y);
	dae_xml_next_float(local->node, &next, &z);
#endif
	g3d_matrix_scale(x, y, z, transform->matrix);
#if DEBUG > 3
	g_debug("DAE: scale for '%s': %.2f, %.2f, %.2f", object->name,
		x, y, z);
	g3d_matrix_dump(transform->matrix);
#endif
	return TRUE;
}

gboolean dae_cb_source(DaeGlobalData *global, DaeLocalData *local)
{
	gchar *id;

	id = dae_xml_get_attr(local->node, "id");
	if(id) {
		dae_library_add(global->lib, "source", id, local->node);
		g_free(id);
		return TRUE;
	}
	return FALSE;
}

gboolean dae_cb_technique(DaeGlobalData *global, DaeLocalData *local)
{
	return dae_xml_parse(global, local->node, dae_chunks_technique,
		local->level, local->user_data);
}

/* instance_geometry::bind_material::technique_common */
gboolean dae_cb_technique_common(DaeGlobalData *global, DaeLocalData *local)
{
	xmlNodePtr node = NULL, tnode;
	gchar *symbol, *target;

	while(dae_xml_next_child_by_tagname(local->node, &node,
		"instance_material")) {
		symbol = dae_xml_get_attr(node, "symbol");
		if(symbol == NULL)
			continue;
		target = dae_xml_get_attr(node, "target");
		if(target == NULL) {
			g_free(symbol);
			continue;
		}
		tnode = dae_library_lookup(global->lib, "material", target + 1);
		if(tnode)
			dae_library_add(global->lib, "symbol", symbol, tnode);
		g_free(symbol);
		g_free(target);
	}
	return TRUE;
}

gboolean dae_cb_translate(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object = local->user_data;
	G3DTransformation *transform;
	G3DFloat x = 0.0, y = 0.0, z = 0.0;
	gchar *next = NULL;

	g_return_val_if_fail(object != NULL, FALSE);

	transform = object->transformation;
	if(transform == NULL) {
		transform = g_new0(G3DTransformation, 1);
		g3d_matrix_identity(transform->matrix);
		object->transformation = transform;
	}

#if G3D_FLOAT_IS_DOUBLE
	dae_xml_next_double(local->node, &next, &x);
	dae_xml_next_double(local->node, &next, &y);
	dae_xml_next_double(local->node, &next, &z);
#else
	dae_xml_next_float(local->node, &next, &x);
	dae_xml_next_float(local->node, &next, &y);
	dae_xml_next_float(local->node, &next, &z);
#endif
	g3d_matrix_translate(x, y, z, transform->matrix);
#if DEBUG > 3
	g_debug("DAE: translation for '%s': %.2f, %.2f, %.2f", object->name,
		x, y, z);
	g3d_matrix_dump(transform->matrix);
#endif
	return TRUE;
}

gboolean dae_cb_triangles(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object = local->user_data;
	G3DFace *face;
	G3DMaterial *material;
	xmlNodePtr pnode;
	gchar *scnt, *smat, *nextp = NULL;
	guint32 count, normal_count, tex_count, flags = 0;
	G3DFloat *normal_data = NULL, *tex_data = NULL;
	gint i, j, tmp;
	GSList *inputs, *item;
	DaeInput *input;

	g_return_val_if_fail(object != NULL, FALSE);

	scnt = dae_xml_get_attr(local->node, "count");
	g_return_val_if_fail(scnt != NULL, FALSE);
	count = atoi(scnt);
	g_return_val_if_fail(count != 0, FALSE);
	g_free(scnt);

	pnode = dae_xml_get_child_by_tagname(local->node, "p");
	g_return_val_if_fail(pnode != NULL, FALSE);

	/* material */
	material = g_slist_nth_data(object->materials, 0);
	smat = dae_xml_get_attr(local->node, "material");
	if(smat != NULL) {
		material = dae_get_material_by_name(global, smat, local->level);
		g_free(smat);
	}

	/* get all inputs */
	inputs = dae_get_inputs(local->node);
	for(item = inputs; item != NULL; item = item->next) {
		input = (DaeInput *)item->data;
		if(input->semantic == SEM_NORMAL)
			if(dae_load_source(global->lib, input->source,
				&normal_data, &normal_count)) {
				flags |= G3D_FLAG_FAC_NORMALS;
			}
		if(input->semantic == SEM_TEXCOORD)
			if(dae_load_source(global->lib, input->source,
				&tex_data, &tex_count) && (material->tex_image != NULL)) {
				flags |= G3D_FLAG_FAC_TEXMAP;
			}
	}

	for(i = 0; i < count; i ++) {
		face = g_new0(G3DFace, 1);
		face->vertex_count = 3;
		face->vertex_indices = g_new0(guint32, 3);
		face->material = material;
		face->flags = flags;
		object->faces = g_slist_append(object->faces, face);

		if(face->flags & G3D_FLAG_FAC_NORMALS) {
			face->normals = g_new0(G3DFloat, 3 * 3);
		}
		if(face->flags & G3D_FLAG_FAC_TEXMAP) {
			face->tex_image = material->tex_image;
			face->tex_vertex_count = 3;
			face->tex_vertex_data = g_new0(G3DFloat, 3 * 2);
		}

		for(j = 0; j < 3; j ++) {
			for(item = inputs; item != NULL; item = item->next) {
				input = (DaeInput *)item->data;
				dae_xml_next_int(pnode, &nextp, &tmp);
				switch(input->semantic) {
					case SEM_VERTEX:
						face->vertex_indices[j] = tmp;
						if(face->vertex_indices[j] >= object->vertex_count) {
							g_warning("triangles: [%s] face[%d] (%d) >= %d",
								object->name, j, face->vertex_indices[j],
								object->vertex_count);
							face->vertex_indices[j] = 0;
						}
						break;
					case SEM_NORMAL:
						if(flags & G3D_FLAG_FAC_NORMALS) {
							face->normals[j * 3 + 0] =
								normal_data[tmp * 3 + 0];
							face->normals[j * 3 + 1] =
								normal_data[tmp * 3 + 1];
							face->normals[j * 3 + 2] =
								normal_data[tmp * 3 + 2];
						}
						break;
					case SEM_TEXCOORD:
						if(flags & G3D_FLAG_FAC_TEXMAP) {
							face->tex_vertex_data[j * 2 + 0] =
								tex_data[tmp * 2 + 0];
							face->tex_vertex_data[j * 2 + 1] = 1.0 -
								tex_data[tmp * 2 + 1];
						}
						break;
					case SEM_UNKNOWN:
						break;
				}
			} /* inputs */
#if DEBUG > 3
			g_debug("DAE: %-2i, %-2i, %-2i",
				face->vertex_indices[0],
				face->vertex_indices[1],
				face->vertex_indices[2]);
#endif
		}
	}
	if(tex_data)
		g_free(tex_data);
	if(normal_data)
		g_free(normal_data);
	dae_inputs_free(inputs);

	return TRUE;
}

gboolean dae_cb_vertices(DaeGlobalData *global, DaeLocalData *local)
{
	return dae_xml_parse(global, local->node, dae_chunks_vertices,
		local->level, local->user_data);
}

gboolean dae_cb_vertices__input(DaeGlobalData *global, DaeLocalData *local)
{
	G3DObject *object = (G3DObject *)(local->user_data);
	gchar *sid, *sem, *cnt, *next = NULL;
	gboolean skip = FALSE;
	xmlNodePtr snode, fanode;
	gint i, j;

	g_return_val_if_fail(object != NULL, FALSE);

	sem = dae_xml_get_attr(local->node, "semantic");
	sid = dae_xml_get_attr(local->node, "source");
	g_return_val_if_fail((sem != NULL) && (sid != NULL), FALSE);

	/* get 'source' node, skip leading '#' from 'source' attribute */
	snode = dae_library_lookup(global->lib, "source", sid + 1);
#if DEBUG > 0
	g_debug("DAE: looking up source '%s' from library: %p", sid + 1,
		(void *)snode);
#endif
	g_return_val_if_fail(snode != NULL, FALSE);

	if(sem)
		g_free(sem);
	if(sid)
		g_free(sid);

	/* TODO: check technique_common/accessor */

	fanode = dae_xml_get_child_by_tagname(snode, "float_array");
	if(fanode) {
		cnt = dae_xml_get_attr(fanode, "count");
#if DEBUG > 0
		g_debug("DAE: float_array count=\"%s\"", cnt);
#endif
		g_return_val_if_fail(cnt != NULL, FALSE);

		object->vertex_count = atoi(cnt);
		g_return_val_if_fail(object->vertex_count != 0, FALSE);

		object->vertex_data = g_new0(G3DFloat, 3 * object->vertex_count);
		for(i = 0; i < object->vertex_count / 3; i ++)
		{
			for(j = 0; j < 3; j ++)
			{
#if G3D_FLOAT_IS_DOUBLE
				if(!dae_xml_next_double(fanode, &next, &(object->vertex_data[i * 3 + j])))
#else
				if(!dae_xml_next_float(fanode, &next, &(object->vertex_data[i * 3 + j])))
#endif
				{
					skip = TRUE;
					break;
				}
			}
#if DEBUG > 3
			g_debug("DAE: %-3.2f, %-3.2f, %-3.2f",
				object->vertex_data[i * 3 + 0],
				object->vertex_data[i * 3 + 1],
				object->vertex_data[i * 3 + 2]);
#endif
			if(skip)
				break;
		}
	}

	return TRUE;
}

gboolean dae_cb_visual_scene(DaeGlobalData *global, DaeLocalData *local)
{
	return dae_xml_parse(global, local->node, dae_chunks_visual_scene,
		local->level, NULL);
}
