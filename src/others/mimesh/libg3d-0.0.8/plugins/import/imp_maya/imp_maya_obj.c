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

#include <g3d/model.h>

#include "imp_maya_obj.h"

MayaObject *maya_obj_new(void)
{
	MayaObject *obj;

	obj = g_new0(MayaObject, 1);
	obj->vars = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	return obj;
}

void maya_obj_free(MayaObject *obj)
{
	g_hash_table_destroy(obj->vars);
	if(obj->name) g_free(obj->name);
	g_free(obj);
}

G3DObject *maya_obj_to_g3d(MayaObject *obj)
{
	G3DObject *object;

	object = g_new0(G3DObject, 1);
	object->name = obj->name ? g_strdup(obj->name) : "(unnamed)";

	return object;
}

static G3DObject *get_by_path(G3DModel *model, gchar *path)
{
	gchar **parts, **partp;
	G3DObject *object = NULL;
	GSList *olist;

	partp = parts = g_strsplit(path, "|", 0);
	olist = model->objects;
	while(*partp)
	{
		while(olist)
		{
			object = (G3DObject *)olist->data;

			if(strcmp(object->name, *partp) == 0) break;

			olist = olist->next;
			object = NULL;
		}

		if(object == NULL) return NULL;

		partp ++;
		olist = object->objects;
	}

	g_strfreev(parts);

	return object;
}

gboolean maya_obj_add_to_tree(MayaObject *obj, G3DModel *model,
	G3DObject *object)
{
	G3DObject *parent = NULL;

	if(obj->parent)
	{
		if(*(obj->parent) == '|')
			parent = get_by_path(model, obj->parent + 1);
		else
			parent = g3d_model_get_object_by_name(model, obj->parent);

		if(parent == NULL)
			g_warning(
				"[Maya] maya_obj_add_to_tree: parent object '%s' not found",
				obj->parent);
	}

	if(parent != NULL)
		parent->objects = g_slist_append(parent->objects, object);
	else
		model->objects = g_slist_append(model->objects, object);

	return TRUE;
}
