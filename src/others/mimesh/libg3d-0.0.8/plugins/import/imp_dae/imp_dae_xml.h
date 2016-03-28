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

#ifndef _IMP_DAE_XML_H
#define _IMP_DAE_XML_H

#include <glib.h>
#include <libxml/tree.h>

#include "imp_dae_library.h"
#include "imp_dae_cb.h"

gchar *dae_xml_get_attr(xmlNodePtr node, const gchar *attrname);
xmlNodePtr dae_xml_get_child_by_tagname(xmlNodePtr parent, const gchar *tag);
xmlNodePtr dae_xml_next_child_by_tagname(xmlNodePtr parent, xmlNodePtr *node,
	gchar *nodename);
xmlNodePtr dae_xml_next_child(DaeLibrary *lib, xmlNodePtr parent,
	xmlNodePtr *node, xmlNodePtr *instance, gchar **nodename);
gboolean dae_xml_parse(DaeGlobalData *global, xmlNodePtr parent,
	DaeChunkDesc *chunks, guint32 level, gpointer user_data);
gboolean dae_xml_next_int(xmlNodePtr node, gchar **nextp, gint *i);
gboolean dae_xml_next_double(xmlNodePtr node, gchar **nextp, GLdouble *d);
gboolean dae_xml_next_float(xmlNodePtr node, gchar **nextp, GLfloat *f);

#endif /* _IMP_DAE_XML_H */
