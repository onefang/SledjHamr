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
#include <stdlib.h>

#include <g3d/g3d.h>
#include <g3d/texture.h>

int main(int argc, char *argv[])
{
	G3DContext *context;
	G3DImage *tex;
	FILE *ppm;
	gchar *outfile;
	guint32 x, y;

	context = g3d_context_new();

	if(argc > 1) {
/*		tex = g3d_texture_load(context, argv[1]); */
tex = NULL;
		if(tex != NULL) {
			outfile = g_strdup_printf("%s.ppm", argv[1]);
			ppm = fopen(outfile, "w");
			if(ppm != NULL) {
				fprintf(ppm, "P3\n%u %u\n255\n",
					tex->width, tex->height);
				for(y = 0; y < tex->height; y ++) {
					for(x = 0; x < tex->width; x ++) {
						fprintf(ppm, "%u %u %u\n",
							tex->pixeldata[(y * tex->width + x) * 4 + 0],
							tex->pixeldata[(y * tex->width + x) * 4 + 1],
							tex->pixeldata[(y * tex->width + x) * 4 + 2]);
					}
				}
				fclose(ppm);
			} else {
				fprintf(stderr, "%s: failed to write to '%s'\n",
					argv[0], outfile);
			}
			g3d_texture_free(tex);
			g_free(outfile);
		} else {
			fprintf(stderr, "%s: failed to load image '%s'\n",
				argv[0], argv[1]);
		}
	}

	g3d_context_free(context);

	return EXIT_SUCCESS;
}
