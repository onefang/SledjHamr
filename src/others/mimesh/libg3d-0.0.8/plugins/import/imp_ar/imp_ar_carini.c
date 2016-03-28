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

#define _ISOC99_SOURCE 1 /* strtof() */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <g3d/types.h>

#define CARINI_IN_VAR     1
#define CARINI_IN_VALUE   2
#define CARINI_IN_COMMENT 3

static gchar *ar_carini_getpath(GQueue *queue)
{
	gchar *tmp, *path = NULL;
	gint32 i;

	for(i = 0; i < g_queue_get_length(queue); i ++)
	{
		if(path == NULL)
			path = g_strdup((gchar *)g_queue_peek_nth(queue, i));
		else
		{
			tmp = g_strdup_printf("%s.%s",
				(gchar *)g_queue_peek_nth(queue, i), path);
			g_free(path);
			path = tmp;
		}
	}

	return path;
}

GHashTable *ar_carini_load(void)
{
	GHashTable *ht;
	GQueue *q;
	FILE *f;
	gchar var[256], *varp; /* section or variable */
	gchar val[256], *valp; /* value */
	gchar *tmps, *varname;
	guint32 in = CARINI_IN_VAR;
	gint32 c;

	f = fopen("car.ini", "r");
	if(f == NULL)
		f = fopen("Car.ini", "r");

	if(f == NULL)
	{
		g_error("failed to read 'car.ini'\n");
		return NULL;
	}

	ht = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	if(ht == NULL)
	{
		fclose(f);
		g_error("could not create hash table");
		return NULL;
	}

	q = g_queue_new();

	valp = val;
	memset(val, 0, 256);
	varp = var;
	memset(var, 0, 256);

	while(!feof(f))
	{
		c = fgetc(f);
		switch(c)
		{
			case '\0':
			case EOF:
				break;

			case '{':
				/* push section */
				g_queue_push_head(q, g_strdup(var));

				in = CARINI_IN_VAR;
				varp = var;
				memset(var, 0, 256);
				break;

			case '}':
				/* pop section */
				tmps = (gchar *)g_queue_pop_head(q);
				g_free(tmps);

				in = CARINI_IN_VAR;
				varp = var;
				memset(var, 0, 256);
				break;

			case ';':
				in = CARINI_IN_COMMENT;
				break;

			case '=':
				if(in == CARINI_IN_VAR)
				{
					in = CARINI_IN_VALUE;
					valp = val;
					memset(val, 0, 256);
				}
				break;

			case '\n':
			case '\r':
				if(in == CARINI_IN_VALUE)
				{
					tmps = ar_carini_getpath(q);
					if(tmps)
					{
						varname = g_strdup_printf("%s.%s", tmps, var);
						g_free(tmps);
					}
					else
						varname = g_strdup(var);

					/* update hash table */
					g_hash_table_insert(ht, varname, g_strdup(val));
#if DEBUG > 2
					printf("D: %s = %s\n", varname, val);
#endif

					in = CARINI_IN_VAR;
					varp = var;
					memset(var, 0, 256);
				}
				else if(in == CARINI_IN_COMMENT)
				{
					in = CARINI_IN_VAR;
				}
				break;

			case ' ':
			case '\t':
				if(in == CARINI_IN_VALUE)
				{
					*valp = c;
					valp ++;
				}
				break;

			default:
				if(in == CARINI_IN_VALUE)
				{
					*valp = c;
					valp ++;
				}
				else if(in == CARINI_IN_VAR)
				{
					*varp = c;
					varp ++;
				}
				break;
		} /* switch(c) */
	} /* !feof(f) */

	/* clean up */
	g_queue_free(q);

	return ht;
}

void ar_carini_free(GHashTable *ht)
{
	g_hash_table_destroy(ht);
}

G3DFloat ar_carini_get_float(GHashTable *ht, gchar *path)
{
	gchar *value;
	G3DFloat valf;

	value = (gchar *)g_hash_table_lookup(ht, path);
	if(value == NULL)
		return 0.0;

	valf = strtof(value, NULL);
	return valf;
}

gboolean ar_carini_get_position(GHashTable *ht, gchar *prefix,
	G3DFloat *x, G3DFloat *y, G3DFloat *z)
{
	gchar *path, *value;

	/* x */
	path = g_strdup_printf("%s.x", prefix);
	value = (gchar *)g_hash_table_lookup(ht, path);
	if(value)
		*x = strtof(value, NULL);
	else
		*x = 0.0;
	g_free(path);

	/* y */
	path = g_strdup_printf("%s.y", prefix);
	value = (gchar *)g_hash_table_lookup(ht, path);
	if(value)
		*y = strtof(value, NULL);
	else
		*y = 0.0;
	g_free(path);

	/* z */
	path = g_strdup_printf("%s.z", prefix);
	value = (gchar *)g_hash_table_lookup(ht, path);
	if(value)
		*z = strtof(value, NULL);
	else
		*z = 0.0;
	g_free(path);

#if DEBUG > 0
	printf("D: position for '%s': %.2f, %.2f, %.2f\n", prefix, *x, *y, *z);
#endif

	return TRUE;
}
