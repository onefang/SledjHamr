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
#include <glib.h>

#include "imp_maya_obj.h"

gboolean maya_var_set(MayaObject *obj, const gchar *var, gpointer value)
{
	g_hash_table_replace(obj->vars, g_strdup(var), value);

	return TRUE;
}

gpointer maya_var_get(MayaObject *obj, const gchar *var)
{
	gpointer val;

	val = g_hash_table_lookup(obj->vars, var);
	return val;
}

gboolean maya_var_set_double(MayaObject *obj, const gchar *var, gdouble value)
{
	gdouble *pval;

	pval = g_new0(gdouble, 1);
	*pval = value;

	return maya_var_set(obj, var, pval);
}

gdouble maya_var_get_double(MayaObject *obj, const gchar *var, gdouble defval)
{
	gdouble *pval;

	pval = maya_var_get(obj, var);
	if(pval == NULL)
		return defval;

	return *pval;
}
