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

#include <stdio.h>
#include <string.h>
#include <g3d/config.h>
#include <g3d/types.h>
#include <g3d/plugins.h>
#include <g3d/stream.h>
#include <g3d/texture.h>

#ifdef G3D_DEBUG_DUMP_IMAGE
static gboolean dump_ppm(G3DImage *image, const gchar *filename);
#endif


EAPI
G3DImage *g3d_texture_load_from_stream(G3DContext *context, G3DModel *model,
	G3DStream *stream)
{
	G3DImage *image;

	if(model != NULL) {
		if(model->tex_images == NULL)
			model->tex_images = g_hash_table_new(g_str_hash, g_str_equal);

		image = g_hash_table_lookup(model->tex_images, stream->uri);
		if(image != NULL)
			return image;
	}

	image = g_new0(G3DImage, 1);
	image->tex_scale_u = 1.0;
	image->tex_scale_v = 1.0;

	if(g3d_plugins_load_image_from_stream(context, stream, image)) {
		image->tex_id = g_str_hash(stream->uri);
		if(model != NULL)
			g_hash_table_insert(model->tex_images, g_strdup(stream->uri),
				image);
		return image;
	}
	g_free(image);
	return NULL;
}

EAPI
G3DImage *g3d_texture_load_cached(G3DContext *context, G3DModel *model,
	const gchar *filename)
{
	G3DImage *image = NULL;
	G3DStream *imgstream;
#ifdef G3D_DEBUG_DUMP_IMAGE
	gchar *basename, *ppmname;
#endif

	/* create hash table if it does not exist yet */
	if(model->tex_images == NULL)
		model->tex_images = g_hash_table_new(g_str_hash, g_str_equal);

	/* if already loaded, return cached image */
	image = g_hash_table_lookup(model->tex_images, filename);
	if(image != NULL)
		return image;

	if (model->stream)
	{
	    g_debug("texture: loading '%s' from: '%s'", filename, model->stream->uri);
	    imgstream = g3d_stream_open_zip_from_stream(model->stream->zip_container, filename);
	    image = g3d_texture_load_from_stream(context, model, imgstream);
	}
	if(image != NULL)
	{
		image->tex_id = g_str_hash(filename);
#if 0
		g3d_texture_prepare(image);
#endif
		g_hash_table_insert(model->tex_images, (gpointer)g_strdup(filename),
			image);
	}

#ifdef G3D_DEBUG_DUMP_IMAGE
	if(image)
	{
		basename = g_path_get_basename(filename);
		ppmname = g_strdup_printf("/tmp/%s.ppm", basename);
		dump_ppm(image, ppmname);
		g_free(ppmname);
		g_free(basename);
	}
#endif

	return image;
}

EAPI
void g3d_texture_free(G3DImage *texture)
{
	if(texture->name) g_free(texture->name);
	if(texture->pixeldata) g_free(texture->pixeldata);
	g_free(texture);
}

EAPI
gboolean g3d_texture_prepare(G3DImage *texture)
{
	guint32 nw = 1, nh = 1, y;
	guint8 *np;

	while(nw < texture->width) nw *= 2;
	while(nh < texture->height) nh *= 2;

	if((nw != texture->width) || (nh != texture->height))
	{
		/* blow up texture image to dimensions with a power of two */
		np = g_malloc(nw * nh * 4);
		memset(np, 0xFF, nw * nh * 4);

		/* copy image data */
		for(y = 0; y < nh; y ++)
			memcpy(np + ((nh - y - 1) * nw * 4),
				texture->pixeldata +
					(((texture->height - y - 1) % texture->height) *
					texture->width * 4),
				texture->width * 4);

		/* calculate scaling factor */
		texture->tex_scale_u = ((G3DFloat)texture->width / (G3DFloat)nw);
		texture->tex_scale_v = ((G3DFloat)texture->height / (G3DFloat)nh);

#if DEBUG > 0
		g_debug("texture scaling factor for '%s' set to %.2f,%.2f",
			texture->name, texture->tex_scale_u, texture->tex_scale_v);
#endif

		/* update image */
		g_free(texture->pixeldata);
		texture->pixeldata = np;
		texture->width = nw;
		texture->height = nh;

		return TRUE;
	}
	return FALSE;
}

EAPI
gboolean g3d_texture_flip_y(G3DImage *texture)
{
	guint8 *newpixel;
	gint32 y;

	g_return_val_if_fail(texture != NULL, FALSE);

	newpixel = g_new0(guint8, texture->width * texture->height * 4);

	for(y = 0; y < texture->height; y ++)
	{
		memcpy(
			newpixel + (y * texture->width * 4),
			texture->pixeldata + (
				(texture->height - y - 1) * texture->width * 4),
			texture->width * 4);
	}

	g_free(texture->pixeldata);
	texture->pixeldata = newpixel;

	return TRUE;
}

#ifdef G3D_DEBUG_DUMP_IMAGE
static gboolean dump_ppm(G3DImage *image, const gchar *filename)
{
	FILE *f;
	guint32 x, y;

	f = fopen(filename, "w");
	if(f == NULL)
	{
		g_warning("image: failed to write to '%s'", filename);
		return FALSE;
	}

	fprintf(f, "P3\n# CREATOR: g3dviewer\n%d %d\n%d\n",
		image->width, image->height, 255);

	for(y = 0; y < image->height; y ++)
		for(x = 0; x < image->width; x ++)
			fprintf(f, "%d\n%d\n%d\n",
				image->pixeldata[(y * image->width + x) * 4 + 0],
				image->pixeldata[(y * image->width + x) * 4 + 1],
				image->pixeldata[(y * image->width + x) * 4 + 2]);

	fclose(f);
	return TRUE;
}
#endif

EAPI
G3DImage *g3d_texture_merge_alpha(G3DImage *image, G3DImage *aimage)
{
	G3DImage *texture;
	gint32 x, y;
	gboolean negative;

	g_return_val_if_fail(aimage != NULL, NULL);

	if(image && (
			(image->width != aimage->width) ||
			(image->height != aimage->height)))
	{
		/* size doesn't match, don't do something */
		return image;
	}

	if(image)
	{
		texture = image;
	}
	else
	{
		texture = g_new0(G3DImage, 1);
		texture->tex_scale_u = 1.0;
		texture->tex_scale_v = 1.0;
		texture->width = aimage->width;
		texture->height = aimage->height;
		texture->depth = 4;
		texture->pixeldata = g_malloc(texture->width * texture->height * 4);
	}

	/* negative map? */
	/* FIXME: better solution? */
	if(aimage->pixeldata[0] == 0)
		negative = TRUE;
	else
		negative = FALSE;

	for(y = 0; y < texture->height; y ++)
	{
		for(x = 0; x < texture->width; x ++)
		{
			texture->pixeldata[(y * image->width + x) * 4 + 3] = (negative ?
				255 - aimage->pixeldata[(y * image->width + x) * 4 + 0] :
				aimage->pixeldata[(y * image->width + x) * 4 + 0]);
		}
	}

	return texture;
}

