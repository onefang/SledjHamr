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

#include <g3d/object.h>

#include "imp_ldraw_types.h"
#include "imp_ldraw_part.h"
#include "imp_ldraw_color.h"
#include "imp_ldraw_misc.h"

static gboolean ldraw_library_add_dir(LDrawLibrary *lib, const gchar *subdir)
{
	LDrawPart *part;
	const gchar *filename;
	gchar *partdir, *strup;
	GDir *dir;
	GError *error;
	gboolean prefix = FALSE;

	partdir = g_strdup_printf("%s%c%s", lib->libdir, G_DIR_SEPARATOR, subdir);
	dir = g_dir_open(partdir, 0, &error);
	if(dir == NULL) {
		g_warning("LDraw: failed to open directory '%s': %s", partdir,
			error->message);
		g_error_free(error);
		g_free(partdir);
		return FALSE;
	}

	if(path_sep(subdir))
		prefix = TRUE;

	filename = g_dir_read_name(dir);
	while(filename) {
		if(g_ascii_strcasecmp(filename + strlen(filename) - 4, ".dat") == 0) {
#if DEBUG > 3
			g_debug("LDraw: Library: adding '%s'", filename);
#endif
			part = g_new0(LDrawPart, 1);
			part->filename = g_strdup_printf("%s%c%s",
				partdir, G_DIR_SEPARATOR, filename);
			strup = g_ascii_strup(filename, -1);
			if(prefix)
				part->name = g_strdup_printf("%s%c%s",
					path_sep(subdir) + 1,
					G_DIR_SEPARATOR, strup);
			else
				part->name = g_strdup(strup);
			g_free(strup);
			g_hash_table_insert(lib->partdb, part->name, part);
			lib->partlist = g_slist_append(lib->partlist, part);
		}
		filename = g_dir_read_name(dir);
	}
	g_dir_close(dir);
	g_free(partdir);

	return TRUE;
}

LDrawLibrary *ldraw_library_init(void)
{
	LDrawLibrary *lib;
	const gchar *lddir;

	lib = g_new0(LDrawLibrary, 1);
	lib->partdb = g_hash_table_new(g_str_hash, g_str_equal);

	ldraw_color_init(lib);

	lddir = g_getenv("LDRAWDIR");
	if(lddir == NULL) /* warning is issued when trying to load a model */
		return lib;

	lib->libdir = g_strdup(lddir);

	ldraw_library_add_dir(lib, "PARTS");
	ldraw_library_add_dir(lib, "PARTS" G_DIR_SEPARATOR_S "S");
	ldraw_library_add_dir(lib, "P");
	ldraw_library_add_dir(lib, "P" G_DIR_SEPARATOR_S "48");

	return lib;
}

void ldraw_library_cleanup(LDrawLibrary *lib)
{
	GSList *item;
	LDrawPart *part;

	item = lib->partlist;
	while(item != NULL) {
		part = item->data;
		item = g_slist_remove(item, part);
		ldraw_part_free(part);
	}
	g_hash_table_destroy(lib->partdb);
	g_free(lib);
}

void ldraw_library_insert(LDrawLibrary *lib, gchar *name, gpointer data)
{
	g_hash_table_insert(lib->partdb, name, data);
}

G3DObject *ldraw_library_lookup(LDrawLibrary *lib, const gchar *name)
{
	LDrawPart *part;
	gchar *strc;

	part = g_hash_table_lookup(lib->partdb, name);
	if(part == NULL) {
		strc = g_ascii_strup(name, -1);
		g_strdelimit(strc, "/\\", G_DIR_SEPARATOR);
		part = g_hash_table_lookup(lib->partdb, strc);
		g_free(strc);
	}
#if 0
	if(part == NULL) {
		strc = g_ascii_strdown(name, -1);
		g_strdelimit(strc, "/\\", G_DIR_SEPARATOR);
		part = g_hash_table_lookup(lib->partdb, strc);
		g_free(strc);
	}
#endif
	if(part == NULL) {
		g_warning("LDraw: failed to find '%s' in library", name);
		return NULL;
	}
	if(part->object == NULL) {
		if(part->stream) {
			/* MPD loader has a custom stream */
			part->object = ldraw_part_get_object(part, lib);
		} else if(part->filename) {
			/* try to load part from standard library */
			part->stream = g3d_stream_open_file(part->filename, "r");
			if(part->stream) {
				part->object = ldraw_part_get_object(part, lib);
				g3d_stream_close(part->stream);
				part->stream = NULL;
			} else {
				g_warning("LDraw: failed to open stream for '%s'",
					part->filename);
			}
		} else {
			g_warning("LDraw: don't know how to open part '%s'", part->name);
		}

		if(part->object == NULL) {
			g_warning("LDraw: failed to load part '%s'", part->name);
			return NULL;
		}
	}
	return g3d_object_duplicate(part->object);
}

