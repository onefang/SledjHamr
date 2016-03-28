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
#include <string.h>

#include <g3d/vector.h>
#include <g3d/matrix.h>
#include <g3d/material.h>

#include "imp_blend_types.h"
#include "imp_blend_def.h"
#include "imp_blend_sdna.h"

/* mesh data */
gboolean blend_cb_DATA_ME(BlendGlobal *global, BlendLocal *local)
{
	G3DFace *face;
	BlendSdnaPropData *spdata;
	gchar *stmp;
	gint32 i, j;

	if((local->ndata == 0) || (local->data[0] == NULL))
		return TRUE;
	g_return_val_if_fail(local->object != NULL, FALSE);

	/* all structs in one DATA chunk have the same type */
	if(strcmp(local->data[0]->sstruct->name, "MVert") == 0) {
		local->object->vertex_count = local->ndata;
		local->object->vertex_data = g3d_vector_new(3,
			local->object->vertex_count);
		for(i = 0; i < local->ndata; i ++) {
			spdata = blend_sdna_get_property(local->data[i], "co", T_FLOAT);
			if(spdata)
				for(j = 0; j < 3; j ++)
					local->object->vertex_data[i * 3 + j] = spdata->fval[j];
		}
	} else if(strcmp(local->data[0]->sstruct->name, "MFace") == 0) {
		for(i = 0; i < local->ndata; i ++) {
			face = g_new0(G3DFace, 1);
			face->vertex_count = 4;
			face->vertex_indices = g_new0(guint32, 4);
			face->material = g_slist_nth_data(local->object->materials, 0);
			local->object->faces = g_slist_prepend(local->object->faces, face);
			for(j = 0; j < 4; j ++) {
				stmp = g_strdup_printf("v%d", j);
				spdata = blend_sdna_get_property(local->data[i], stmp, T_INT);
				if(spdata)
					face->vertex_indices[j] = spdata->ival[0];
				g_free(stmp);
			}
#if DEBUG > 2
			g_debug("MFace: %03i (%02i, %02i, %02i, %02i)", i,
				face->vertex_indices[0], face->vertex_indices[1],
				face->vertex_indices[2], face->vertex_indices[3]);
#endif
			spdata = blend_sdna_get_property(local->data[i], "mat_nr", T_CHAR);
			/* TODO */
		}
	} else {
#if DEBUG > 0
		for(i = 0; i < local->ndata; i ++)
			blend_sdna_dump_data(local->data[i], 0);
#endif
	}
	return TRUE;
}

/* struct DNA */
gboolean blend_cb_DNA1(BlendGlobal *global, BlendLocal *local)
{
	if(global->sdna != NULL)
		return TRUE;
	global->sdna = blend_sdna_read_dna1(global->stream,
		global->flags, local->len);
	if(global->sdna == NULL) {
		g_warning("Blend: failed to read DNA1, giving up...");
		return FALSE;
	}
	/* rewind stream to really read content */
	g3d_stream_seek(global->stream, BLEND_HEADER_SIZE, G_SEEK_SET);
	local->len = 0;
	return TRUE;
}

static gchar *get_name_from_id(BlendSdnaData *sdata)
{
	BlendSdnaPropData *spdata, *spdata2;
	gchar *name = NULL;
	gint32 i;

	spdata = blend_sdna_get_property(sdata, "id", T_STRUCT);
	if(spdata) {
		spdata2 = blend_sdna_get_property(spdata->structval, "name", T_CHAR);
		if(spdata2) {
			name = g_new0(gchar, spdata2->sprop->nitems + 1);
			for(i = 0; i < spdata2->sprop->nitems; i ++)
				name[i] = spdata2->ival[i];
		}
	}
	return name;
}

static G3DFloat get_float(BlendSdnaData *sdata, const gchar *var)
{
	BlendSdnaPropData *spdata;
	spdata = blend_sdna_get_property(sdata, var, T_FLOAT);
	if(spdata)
		return spdata->fval[0];
	return 0.0;
}

/* material */
gboolean blend_cb_MA(BlendGlobal *global, BlendLocal *local)
{
	G3DMaterial *material;

	g_return_val_if_fail(local->data[0] != NULL, FALSE);

#if DEBUG > 0
	blend_sdna_dump_data(local->data[0], 0);
#endif

	material = g3d_material_new();
	material->name = get_name_from_id(local->data[0]);
	global->model->materials = g_slist_append(global->model->materials,
		material);
	material->r = get_float(local->data[0], "r");
	material->g = get_float(local->data[0], "g");
	material->b = get_float(local->data[0], "b");
	return TRUE;
}

/* mesh */
gboolean blend_cb_ME(BlendGlobal *global, BlendLocal *local)
{
	G3DObject *object;
	G3DMaterial *material;
	G3DVector v[3];
	BlendSdnaPropData *spdata;
	gint32 i;

	g_return_val_if_fail(local->data[0] != NULL, FALSE);

#if DEBUG > 0
	blend_sdna_dump_data(local->data[0], 0);
#endif

	object = g_new0(G3DObject, 1);
	if(local->grpobject)
		local->grpobject->objects = g_slist_append(local->grpobject->objects,
			object);
	else
		global->model->objects = g_slist_append(global->model->objects,
			object);
	local->object = object;
	material = g3d_material_new();
	material->name = g_strdup("(default material)");
	object->materials = g_slist_append(local->object->materials, material);

	/* name */
	object->name = get_name_from_id(local->data[0]);

	object->transformation = g_new0(G3DTransformation, 1);
	g3d_matrix_identity(object->transformation->matrix);

	/* rotation */
	spdata = blend_sdna_get_property(local->data[0], "rot", T_FLOAT);
	if(spdata) {
		for(i = 0; i < 3; i ++)
			v[i] = spdata->fval[i];
		g3d_matrix_rotate_xyz(v[0], v[1], v[2],
			object->transformation->matrix);
#if DEBUG > 0
		g_debug("ME: rotate %f, %f, %f", v[0], v[1], v[2]);
#endif
	}
	/* size */
	spdata = blend_sdna_get_property(local->data[0], "size", T_FLOAT);
	if(spdata) {
		for(i = 0; i < 3; i ++)
			v[i] = spdata->fval[i];
		g3d_matrix_scale(v[0], v[1], v[2], object->transformation->matrix);
#if DEBUG > 2
		g_debug("ME: scale %f, %f, %f", v[0], v[1], v[2]);
#endif
	}
	/* location */
	spdata = blend_sdna_get_property(local->data[0], "loc", T_FLOAT);
	if(spdata) {
		for(i = 0; i < 3; i ++)
			v[i] = spdata->fval[i];
		g3d_matrix_translate(v[0], v[1], v[2],
			object->transformation->matrix);
#if DEBUG > 2
		g_debug("ME: translate %f, %f, %f", v[0], v[1], v[2]);
#endif
	}
	return TRUE;
}

gboolean blend_cb_OB(BlendGlobal *global, BlendLocal *local)
{
	G3DObject *object;

	g_return_val_if_fail(local->data[0] != NULL, FALSE);

#if DEBUG > 0
	blend_sdna_dump_data(local->data[0], 0);
#endif

	object = g_new0(G3DObject, 1);
	object->name = get_name_from_id(local->data[0]);
	local->grpobject = object;
	global->model->objects = g_slist_append(global->model->objects, object);

	return TRUE;
}


