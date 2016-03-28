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
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include <glib.h>
#include <g3d/types.h>

#include "imp_acf_airfoil.h"

static AcfAirfoil *acf_airfoil_read(const gchar *path);

AcfAirfoilDb *acf_airfoil_init(void)
{
	AcfAirfoilDb *db;
	AcfAirfoil *afl;
	GDir *dir;
	const gchar *dirname, *filename;
	gchar *path;
	GError *error = NULL;

	setlocale(LC_NUMERIC, "C");

	dirname = g_getenv("AIRFOIL_DIR");
	if(!(dirname && g_file_test(dirname, G_FILE_TEST_IS_DIR))) {
#if DEBUG > 0
		g_warning("ACF: could not load airfoils");
#endif
		return NULL;
	}

	dir = g_dir_open(dirname, 0, &error);
	if(error != NULL) {
		g_warning("ACF: failed to open airfoil directory '%s': %s",
			dirname, error->message);
		g_error_free(error);
		return NULL;
	}

	db = g_new0(AcfAirfoilDb, 1);
	db->db = g_hash_table_new(g_str_hash, g_str_equal);

	filename = g_dir_read_name(dir);
	while(filename != NULL) {
		if(strcmp(filename + strlen(filename) - 4, ".dat") == 0) {
			path = g_strdup_printf("%s%c%s", dirname, G_DIR_SEPARATOR,
				filename);
			afl = acf_airfoil_read(path);
			g_free(path);
			if(afl != NULL) {
				g_hash_table_insert(db->db, afl->filename, afl);
				db->airfoils = g_slist_append(db->airfoils, afl);
#if DEBUG > 2
				g_debug("ACF: airfoil %s loaded", filename);
#endif
			}
		}
		filename = g_dir_read_name(dir);
	}

#if DEBUG > 0
	g_debug("ACF: %d airfoils loaded", g_slist_length(db->airfoils));
#endif
	g_dir_close(dir);

	return db;
}

void acf_airfoil_free(AcfAirfoil *afl)
{
	if(afl->filename)
		g_free(afl->filename);
	if(afl->description)
		g_free(afl->description);
	if(afl->vertex_data)
		g_free(afl->vertex_data);
	g_free(afl);
}

void acf_airfoil_cleanup(AcfAirfoilDb *db)
{
	AcfAirfoil *afl;
	GSList *item;

	g_hash_table_destroy(db->db);
	item = db->airfoils;
	while(item) {
		afl = item->data;
		item = g_slist_remove(item, afl);
		acf_airfoil_free(afl);
	}
	g_free(db);
}

AcfAirfoil *acf_airfoil_lookup(AcfAirfoilDb *db, const gchar *aflname)
{
	return g_hash_table_lookup(db->db, aflname);
}

static AcfAirfoil *acf_airfoil_read(const gchar *path)
{
	AcfAirfoil *afl;
	FILE *f;
	gchar buffer[BUFSIZ + 1];
	guint32 off;
	G3DFloat x, y;

	f = fopen(path, "r");
	if(f == NULL)
		return NULL;

	afl = g_new0(AcfAirfoil, 1);
	fgets(buffer, BUFSIZ, f);
	buffer[BUFSIZ] = '\0';
	g_strstrip(buffer);
	afl->filename = g_path_get_basename(path);
	afl->description = g_strdup(buffer);

	while(!feof(f)) {
		memset(buffer, '\0', BUFSIZ + 1);
		fgets(buffer, BUFSIZ, f);
		g_strstrip(buffer);
		if(strlen(buffer) == 0)
			continue;
		if(sscanf(buffer, G3D_SCANF_FLOAT " " G3D_SCANF_FLOAT, &x, &y) == 2) {
			if((x < 0.0) || (x > 1.0))
				continue;
			off = afl->vertex_count;
			afl->vertex_count ++;
			afl->vertex_data = g_realloc(afl->vertex_data,
				afl->vertex_count * 2 * sizeof(G3DFloat));
			afl->vertex_data[off * 2 + 0] = x;
			afl->vertex_data[off * 2 + 1] = y;
		} else {
#if DEBUG > 2
			g_debug("ACF: airfoil: failed to parse line in %s: %s",
				afl->filename, buffer);
#endif
		}
	}

	fclose(f);

	return afl;
}
