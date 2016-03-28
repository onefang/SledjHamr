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
#include <math.h>

#include <g3d/types.h>
#include <g3d/stream.h>
#include <g3d/object.h>
#include <g3d/face.h>
#include <g3d/material.h>
#include <g3d/matrix.h>
#include <g3d/vector.h>

#include "imp_acf.h"
#include "imp_acf_airfoil.h"
#include "imp_acf_wing.h"
#include "imp_acf_def.h"
#include "imp_acf_detect.h"

static gboolean acf_load_body(AcfGlobalData *global);
static gboolean acf_load_wings(AcfGlobalData *global);

EAPI
gpointer plugin_init(G3DContext *context)
{
	return acf_airfoil_init();
}

EAPI
void plugin_cleanup(gpointer user_data)
{
	AcfAirfoilDb *db = user_data;

	if(db)
		acf_airfoil_cleanup(db);
}

EAPI
gboolean plugin_load_model_from_stream(G3DContext *context, G3DStream *stream,
	G3DModel *model, gpointer user_data)
{
	AcfGlobalData *global;
	const AcfDef *def;
	gboolean bigendian;
	gchar magic;

	magic = g3d_stream_read_int8(stream);
	if(magic == 'i') {
		bigendian = FALSE;
		g_debug("ACF: intel file found");
	} else if(magic == 'a') {
		bigendian = TRUE;
		g_debug("ACF: apple file found");
	} else {
		g_warning("ACF: not a valid .acf file");
		return FALSE;
	}

	g3d_stream_seek(stream, 0, G_SEEK_SET);

	global = g_new0(AcfGlobalData, 1);
	global->context = context;
	global->model = model;
	global->stream = stream;
	global->afldb = user_data;

	def = acf_detect_version(global);
	if(def == NULL) {
		g_free(global);
		return FALSE;
	}

	global->acf = acf_def_read(stream, def, bigendian);
	if(global->acf == NULL) {
		g_free(global);
		return FALSE;
	}

	if(!acf_load_body(global)) {
		acf_def_free(global->acf);
		g_free(global);
		return FALSE;
	}
	if(!acf_load_wings(global)) {
		acf_def_free(global->acf);
		g_free(global);
		return FALSE;
	}

	acf_def_free(global->acf);
	g_free(global);

	return TRUE;
}

EAPI
gchar *plugin_description(void)
{
	return g_strdup("XPlane models.");
}

EAPI
gchar **plugin_extensions(void)
{
	return g_strsplit("acf", ":", 0);
}

/*****************************************************************************/

#define ACF_OBJECT_MIN 44
#define ACF_OBJECT_MAX 66
#define ACF_BODY_NUMSEC 20
#define ACF_BODY_SECVER 18
#define ACF_VERTS_PER_OBJECT (ACF_BODY_NUMSEC * ACF_BODY_SECVER)

#define ACF_REQUIRE_PART(var, name, t) do { \
	var = acf_def_lookup(global->acf, name); \
	if((var == NULL) || (var->type != t)) { \
		g_warning("ACF: %s is missing or has wrong type", name); \
		return FALSE; \
	} } while(0);

#define ACF_USE_PART(var, name, t) do { \
	var = acf_def_lookup(global->acf, name); \
	if((var != NULL) && (var->type != t)) { \
		g_warning("ACF: %s has wrong type", name); \
		var = NULL; \
	} } while(0);

static gboolean acf_load_body(AcfGlobalData *global)
{
	AcfValue *vpart_eq, *vbody_x, *vbody_y, *vbody_z, *vbody_r;
	AcfValue *vxarm, *vyarm, *vzarm, *vx_body, *vy_body, *vz_body;
	G3DObject *object;
	G3DMaterial *material;
	G3DFace *face;
	gint32 i, j;
	guint32 min_obj, max_obj;

	/* lookup some required data in library an return FALSE if one of
	 * them is missing */
	ACF_REQUIRE_PART(vpart_eq, "PARTS_part_eq", XINT);
	ACF_REQUIRE_PART(vbody_x,  "PARTS_body_X",  XFLT);
	ACF_REQUIRE_PART(vbody_y,  "PARTS_body_Y",  XFLT);
	ACF_REQUIRE_PART(vbody_z,  "PARTS_body_Z",  XFLT);
	ACF_REQUIRE_PART(vxarm,    "PARTS_Xarm",    XFLT);
	ACF_REQUIRE_PART(vyarm,    "PARTS_Yarm",    XFLT);
	ACF_REQUIRE_PART(vzarm,    "PARTS_Zarm",    XFLT);
	ACF_REQUIRE_PART(vx_body,  "PARTS_X_body_aero", XFLT);
	ACF_REQUIRE_PART(vy_body,  "PARTS_Y_body_aero", XFLT);
	ACF_REQUIRE_PART(vz_body,  "PARTS_Z_body_aero", XFLT);
	ACF_REQUIRE_PART(vbody_r,  "PARTS_body_r",  XFLT);

	material = g3d_material_new();
	material->name = g_strdup("(default material)");
	global->model->materials = g_slist_append(global->model->materials,
		material);

	min_obj = ACF_OBJECT_MIN;
	max_obj = MIN(ACF_OBJECT_MAX, (vpart_eq->num - 1));
	if(vpart_eq->num <= 57) {
		min_obj = 30;
		max_obj = 49;
	}

	for(i = min_obj; i <= max_obj; i ++) {
		if(!vpart_eq->xint[i])
			continue;

		g_debug("ACF: part[%i]: (%.2f, %.2f, %.2f), %.2f", i,
			vx_body->xflt[i], vy_body->xflt[i], vz_body->xflt[i],
			vbody_r->xflt[i]);

		object = g_new0(G3DObject, 1);
		object->name = g_strdup_printf("object[%d]", i);
		object->vertex_count = ACF_VERTS_PER_OBJECT;
		object->vertex_data = g_new0(G3DFloat, object->vertex_count * 3);
		for(j = 0; j < object->vertex_count; j ++) {
			object->vertex_data[j * 3 + 0] =
				vbody_x->xflt[i * ACF_VERTS_PER_OBJECT + j] + vxarm->xflt[i];
			object->vertex_data[j * 3 + 1] =
				vbody_y->xflt[i * ACF_VERTS_PER_OBJECT + j] + vyarm->xflt[i];
			object->vertex_data[j * 3 + 2] =
				vbody_z->xflt[i * ACF_VERTS_PER_OBJECT + j] + vzarm->xflt[i];
#if DEBUG > 3
			g_debug("\tVERT: (%04d) %.3f, %.3f, %.3f", j,
				object->vertex_data[j * 3 + 0],
				object->vertex_data[j * 3 + 1],
				object->vertex_data[j * 3 + 2]);
#endif
		}

		global->model->objects = g_slist_append(global->model->objects,
			object);

		for(j = 0; j < (ACF_VERTS_PER_OBJECT - ACF_BODY_SECVER - 1); j ++) {
			face = g3d_face_new_tri(material, j + 1, j, j + ACF_BODY_SECVER);
			object->faces = g_slist_prepend(object->faces, face);

			face = g3d_face_new_tri(material, j + ACF_BODY_SECVER,
				j + ACF_BODY_SECVER + 1, j + 1);
			object->faces = g_slist_prepend(object->faces, face);
		}
	}

	return TRUE;
}

static gboolean acf_load_wings(AcfGlobalData *global)
{
	AcfValue *vpart_eq, *vrafl0, *vtafl0;
	AcfValue *vctip, *vcroot, *vdihed, *vsweep, *vels;
	AcfValue *vxarm, *vyarm, *vzarm, *visleft, *vlatsign;
	AcfValue *vslseg, *vsljnd, *vpartss, *vpartse;
	AcfAirfoil *afrt, *aftp;
	G3DObject *object;
	G3DMaterial *material;
	gint32 i;
	guint32 cnt;
	G3DFloat m_dihed[16], m_sweep[16], m_trans[16];
	G3DFloat vecrt[3], vectp[3], lf, ls;
	gchar *title;

	ACF_REQUIRE_PART(vpart_eq, "PARTS_part_eq", XINT);
	ACF_REQUIRE_PART(vrafl0,   "PARTS_Rafl0",   XCHR);
	ACF_REQUIRE_PART(vtafl0,   "PARTS_Tafl0",   XCHR);
	ACF_REQUIRE_PART(vcroot,   "PARTS_Croot",   XFLT);
	ACF_REQUIRE_PART(vctip,    "PARTS_Ctip",    XFLT);
	ACF_REQUIRE_PART(vels,     "PARTS_els",     XINT);
	ACF_REQUIRE_PART(vxarm,    "PARTS_Xarm",    XFLT);
	ACF_REQUIRE_PART(vyarm,    "PARTS_Yarm",    XFLT);
	ACF_REQUIRE_PART(vzarm,    "PARTS_Zarm",    XFLT);

	ACF_USE_PART(vdihed, "PARTS_dihed",  XFLT);
	if(vdihed == NULL)
		ACF_REQUIRE_PART(vdihed, "PARTS_dihed1",  XFLT);
	ACF_USE_PART(vsweep, "PARTS_sweep",  XFLT);
	if(vsweep == NULL)
		ACF_REQUIRE_PART(vsweep, "PARTS_sweep1",  XFLT);

	ACF_REQUIRE_PART(vpartss,  "PARTS_s",           XFLT);
	ACF_REQUIRE_PART(vpartse,  "PARTS_e",           XFLT);
	ACF_REQUIRE_PART(vlatsign, "OVERFLOW_lat_sign", XFLT);
	ACF_REQUIRE_PART(vslseg,   "PARTS_semilen_SEG", XFLT);
	ACF_REQUIRE_PART(vsljnd,   "PARTS_semilen_JND", XFLT);

	ACF_USE_PART(visleft, "OVERFLOW_is_left",  XINT);


	material = g_slist_nth_data(global->model->materials, 0);

	cnt = vrafl0->num / vpart_eq->num;

	for(i = 8; i < vpart_eq->num; i ++) {
		if(strlen(vrafl0->xchr + i * cnt) == 0)
			continue;
		if(vels->xint[i] == 0)
			continue;
#if DEBUG > 0
		g_debug("PARTS_Rafl0[%d]: %s", i, vrafl0->xchr + i * cnt);
		g_debug("PARTS_Tafl0[%d]: %s", i, vtafl0->xchr + i * cnt);
		g_debug(
			"[%i] lat_sign=%.2f, Croot=%.2f, Ctip=%.2f, dihed=%.2f, els=%i", i,
			vlatsign->xflt[i],
			vcroot->xflt[i], vctip->xflt[i],
			vdihed ? vdihed->xflt[i] : -1337.0,
			vels->xint[i]);
		g_debug("[%i] semilen_SEG=%.2f, semilen_JND=%.2f, parts_e=%.2f", i,
			vslseg->xflt[i], vsljnd->xflt[i], vpartse->xflt[i]);
		g_debug("PARTS_s[%i]: %f, %f ... %f, %f", i,
			vpartss->xflt[i * 10 + 0], vpartss->xflt[i * 10 + 1],
			vpartss->xflt[i * 10 + 8], vpartss->xflt[i * 10 + 9]);
#endif
		afrt = acf_airfoil_lookup(global->afldb, "naca16006.dat");
		aftp = acf_airfoil_lookup(global->afldb, "naca16006.dat");
		if((afrt == NULL) || (aftp == NULL))
			continue;

		if(afrt->vertex_count != aftp->vertex_count) {
			g_warning("ACF: airfoil vertex count mismatch: %s=%d, %s=%d",
				afrt->filename, afrt->vertex_count,
				aftp->filename, aftp->vertex_count);
			continue;
		}

		lf = ((visleft && visleft->xint[i]) ? -1 : 1);
		ls = (vlatsign ? vlatsign->xflt[i] : 1.0);

		/* translation */
		g3d_matrix_identity(m_trans);
		g3d_matrix_translate(vxarm->xflt[i], vyarm->xflt[i], vzarm->xflt[i],
			m_trans);

		/* rotation matrices */
		g3d_matrix_rotate(lf * vdihed->xflt[i] * G_PI / 180.0,
			0.0, 0.0, 1.0, m_dihed);
		g3d_matrix_rotate(lf * -1.0 * vsweep->xflt[i] * G_PI / 180.0,
			0.0, 1.0, 0.0, m_sweep);

		/* wing root & tip center */
		memset(vecrt, 0, sizeof(vecrt));
		memset(vectp, 0, sizeof(vectp));
		vectp[0] = vslseg->xflt[i];
		g3d_vector_transform(vectp, vectp + 1, vectp + 2, m_dihed);
		g3d_vector_transform(vectp, vectp + 1, vectp + 2, m_sweep);

		title = g_strdup_printf("Wing[%d]", i);
		object = acf_wing(material, title,
			m_sweep, m_dihed, m_trans,
			vecrt, vectp,
			afrt, aftp,
			vcroot->xflt[i], vctip->xflt[i], lf);
		g_free(title);
		global->model->objects = g_slist_append(global->model->objects,
			object);
	}
	return TRUE;
}
