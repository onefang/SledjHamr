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

#include <g3d/config.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <g3d/types.h>
#include <g3d/stream.h>

static gboolean gdkpixbuf_init(void);
static gboolean gdkpixbuf_postprocess(GdkPixbuf *pixbuf, G3DImage *image,
	const gchar *uri);

#define BUFSIZE 1024

EAPI
gboolean plugin_load_image_from_stream(G3DContext *context, G3DStream *stream,
	G3DImage *image, gpointer user_data)
{
	GdkPixbuf *pixbuf;
	GdkPixbufLoader *loader;
	GError *error = NULL;
	guint8 buffer[BUFSIZE];
	gsize n;
	gboolean retval;

	if(!gdkpixbuf_init())
		return FALSE;

	loader = gdk_pixbuf_loader_new();
	if (NULL == loader)
	{
		g_warning("gdkpixbuf - plugin_load_image_from_stream(): no loader");
		return FALSE;
	}
	while(!g3d_stream_eof(stream)) {
		n = g3d_stream_read(stream, buffer, BUFSIZE);
		if(0 >= n)
			break;
		if(!gdk_pixbuf_loader_write(loader, buffer, n, &error)) {
			g_warning("error loading image data from stream: %s",
				error->message);
			g_error_free(error);
			g_object_unref(loader);
			return FALSE;
		}
	}

	if(!gdk_pixbuf_loader_close(loader, &error)) {
		g_warning("error loading image data from stream: %s",
			error->message);
		g_error_free(error);
		g_object_unref(loader);
		return FALSE;
	}

	pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	gdk_pixbuf_ref(pixbuf);

	retval = gdkpixbuf_postprocess(pixbuf, image, stream->uri);

	g_object_unref(loader);
	return retval;
}

EAPI
gchar *plugin_description(G3DContext *context)
{
	return g_strdup("Read most image formats using the GdkPixbuf library.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
	gchar *extensions = g_strdup("");
	gchar **retval;
	gchar *tmp, **ext;
	GSList *fitem;
	GdkPixbufFormat *format;

	fitem = gdk_pixbuf_get_formats();
	while(fitem)
	{
		gboolean doit = TRUE;

		format = (GdkPixbufFormat *)fitem->data;
		ext = gdk_pixbuf_format_get_extensions(format);

		if (0 == g_strcmp0("bmp", *ext))
		    doit = FALSE;
		else if (0 == g_strcmp0("jpg", *ext))
		    doit = FALSE;
		else if (0 == g_strcmp0("jpeg", *ext))
		    doit = FALSE;
		else if (0 == g_strcmp0("jfif", *ext))
		    doit = FALSE;
		else if (0 == g_strcmp0("jif", *ext))
		    doit = FALSE;

		if (doit)
		{
		    tmp = g_strdup_printf("%s%s%s", extensions, strlen(extensions) ? ":" : "", g_strjoinv(":", ext));
		    g_free(extensions);
		    extensions = tmp;
		}
		fitem = fitem->next;
	}

	retval = g_strsplit(extensions, ":", 0);
	g_free(extensions);
	return retval;
}

/*****************************************************************************/

static gboolean gdkpixbuf_init(void)
{
	static gboolean init = TRUE;

	if(init) {
		/* initialize GDK */
		/* FIXME: problem if already initialized with gtk_init()? */
		gint argc = 0;
		if(!gdk_init_check(&argc, NULL))
			return FALSE;
		init = FALSE;
	}
	return TRUE;
}

static gboolean gdkpixbuf_postprocess(GdkPixbuf *pixbuf, G3DImage *image,
	const gchar *uri)
{
	guint32 x, y, nchannels;
	guchar *p;

	if(gdk_pixbuf_get_colorspace(pixbuf) != GDK_COLORSPACE_RGB) {
		g_warning("GdkPixbuf: %s: colorspace is not RGB", uri);
		gdk_pixbuf_unref(pixbuf);
		return FALSE;
	}

	nchannels = gdk_pixbuf_get_n_channels(pixbuf);
	if(nchannels < 3)
	{
		g_warning("GdkPixbuf: %s: has only %d channels", uri,
			gdk_pixbuf_get_n_channels(pixbuf));
		gdk_pixbuf_unref(pixbuf);
		return FALSE;
	}

	image->width = gdk_pixbuf_get_width(pixbuf);
	image->height = gdk_pixbuf_get_height(pixbuf);
	image->depth = 32;
	image->name = g_path_get_basename(uri);
	image->pixeldata = g_new0(guint8, image->width * image->height * 4);

	for(y = 0; y < image->height; y ++)
		for(x = 0; x < image->width; x ++)
		{
			p = gdk_pixbuf_get_pixels(pixbuf) + y *
				gdk_pixbuf_get_rowstride(pixbuf) + x * nchannels;

			image->pixeldata[(y * image->width + x) * 4 + 0] = p[0];
			image->pixeldata[(y * image->width + x) * 4 + 1] = p[1];
			image->pixeldata[(y * image->width + x) * 4 + 2] = p[2];
			if(gdk_pixbuf_get_n_channels(pixbuf) >= 4)
				image->pixeldata[(y * image->width + x) * 4 + 3] = p[3];
		}

	/* set alpha to 1.0 */
	if(gdk_pixbuf_get_n_channels(pixbuf) < 4)
		for(y = 0; y < image->height; y ++)
			for(x = 0; x < image->width; x ++)
				image->pixeldata[(y * image->width + x) * 4 + 3] = 0xFF;

	gdk_pixbuf_unref(pixbuf);

#if DEBUG > 0
	g_print("GdkPixbuf: image '%s' loaded (%dx%d)\n",
		image->name, image->width, image->height);
#endif

	return TRUE;
}

