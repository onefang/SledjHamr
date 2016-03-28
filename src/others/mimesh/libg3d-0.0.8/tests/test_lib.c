/* $Id: test_lib.c,v 1.1.2.2 2006/01/23 17:04:37 dahms Exp $ */

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
#include <stdlib.h>

#include <g3d/g3d.h>

#if 0
#define DEBUG_MEM 1
#endif

#ifdef DEBUG_MEM
static gpointer (*def_malloc)(gsize size) = NULL;

static void break_here(gsize size)
{
	g_printerr("I'm here (%d bytes)\n", size);
}

static gpointer malloc_debug(gsize size)
{
	if(size == 84)
	{
		break_here(size);
	}

	return def_malloc(size);
}
#endif

int main(int argc, char *argv[])
{
	G3DContext *context;
	G3DModel *model;
	G3DObject *object;
	GSList *oitem;
	guint32 i;

#ifdef DEBUG_MEM
	atexit(g_mem_profile);
	def_malloc = glib_mem_profiler_table->malloc;
	glib_mem_profiler_table->malloc = malloc_debug;
	g_mem_set_vtable(glib_mem_profiler_table);
#endif

	context = g3d_context_new();

	if(argc > 1)
	{
		model = g3d_model_load(context, argv[1]);

		if(model)
		{
			g_print("%s: %d objects:\n", argv[1],
				g_slist_length(model->objects));

			i = 0;
			oitem = model->objects;
			while(oitem)
			{
				object = (G3DObject *)oitem->data;
				g_print("  [%2u] %-50s (%d faces, %d mats)\n",
					i,
					object->name ? object->name : "(NULL)",
					g_slist_length(object->faces),
					g_slist_length(object->materials));

				oitem = oitem->next;
				i ++;
			}

			g3d_model_free(model);
		}
	}

	g3d_context_free(context);

	return EXIT_SUCCESS;
}
