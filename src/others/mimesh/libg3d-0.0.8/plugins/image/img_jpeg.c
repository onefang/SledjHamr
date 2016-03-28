/* $Id$ */

/*
    libg3d - 3D object loading library

    Copyright (c) 2011, Dawid Seikel.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/* This is an ugly collection of styles, I'll fix it up later, just want to get it pushed up now.  lol */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <g3d/types.h>
#include <g3d/stream.h>

#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>

#define BUFSIZE 1024

typedef struct
{
    struct jpeg_source_mgr pub;	/* public fields */

    G3DStream *stream;		/* source stream */
    JOCTET *buffer;		/* start of buffer */
    boolean start_of_file;	/* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr *my_src_ptr;

/* Stuff to stop jpeglib from exit() when things fail. */
struct my_error_mgr
{
    struct jpeg_error_mgr pub;	/* "public" fields */
    jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


static void init_stream_source (j_decompress_ptr cinfo);
static boolean fill_stream_input_buffer (j_decompress_ptr cinfo);
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes);
/*static void resync_to_restart (j_decompress_ptr cinfo, int desired);*/
static void term_source (j_decompress_ptr cinfo);
static void jpeg_stream_src(struct jpeg_decompress_struct *cinfo, G3DStream *stream);


EAPI
gchar *plugin_description(G3DContext *context)
{
    return g_strdup("JPEG images.");
}

EAPI
gchar **plugin_extensions(G3DContext *context)
{
    return g_strsplit("jpg:jpeg:jpe:jfif:jif", ":", 0);
}

EAPI
gboolean plugin_load_image_from_stream(G3DContext *context, G3DStream *stream, G3DImage *image, gpointer user_data)
{
    gsize n;
    gboolean retval;
    guint32 x, y, nchannels;

  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stream_src(&cinfo, stream);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.txt for more info.
   */

  /* Step 4: set parameters for decompression */

	if (cinfo.jpeg_color_space != JCS_RGB)
	{
		g_warning("Jpeg plugin: %s: colorspace is not RGB, converting.", stream->uri);
		cinfo.out_color_space = JCS_RGB;
	}


  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

/*
//output_width            image width and height, as scaled
//output_height
//out_color_components    # of color components in out_color_space
//output_components       # of color components returned per pixel
//colormap                the selected colormap, if any
//actual_number_of_colors         number of entries in colormap
*/
	nchannels = cinfo.output_components;
	if(nchannels < 3)
	{
		g_warning("Jpeg plugin: %s: has only %d channels", stream->uri, nchannels);
		jpeg_destroy_decompress(&cinfo);
		return FALSE;
	}

	image->width = cinfo.output_width;
	image->height = cinfo.output_height;
	image->depth = 32;
	image->name = g_path_get_basename(stream->uri);
	image->pixeldata = g_new0(guint8, image->width * image->height * 4);


  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
    y = 0;
    while (cinfo.output_scanline < cinfo.output_height)
    {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
/*	for(y = 0; y < image->height; y ++) */
		for(x = 0; x < image->width; x ++)
		{
			JSAMPROW p = buffer[0] + x * nchannels;

			image->pixeldata[(y * image->width + x) * 4 + 0] = GETJSAMPLE(p[0]);
			image->pixeldata[(y * image->width + x) * 4 + 1] = GETJSAMPLE(p[1]);
			image->pixeldata[(y * image->width + x) * 4 + 2] = GETJSAMPLE(p[2]);
			if(nchannels >= 4)
				image->pixeldata[(y * image->width + x) * 4 + 3] = GETJSAMPLE(p[3]);
		}
	y++;
  }

    /* set alpha to 1.0 */
    if(nchannels < 4)
	for(y = 0; y < image->height; y ++)
	    for(x = 0; x < image->width; x ++)
		image->pixeldata[(y * image->width + x) * 4 + 3] = 0xFF;

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

    return TRUE;
}

/*
 * SOME FINE POINTS:
 *
 * In the above code, we ignored the return value of jpeg_read_scanlines,
 * which is the number of scanlines actually read.  We could get away with
 * this because we asked for only one line at a time and we weren't using
 * a suspending data source.  See libjpeg.txt for more info.
 *
 * We cheated a bit by calling alloc_sarray() after jpeg_start_decompress();
 * we should have done it beforehand to ensure that the space would be
 * counted against the JPEG max_memory setting.  In some systems the above
 * code would risk an out-of-memory error.  However, in general we don't
 * know the output image dimensions before jpeg_start_decompress(), unless we
 * call jpeg_calc_output_dimensions().  See libjpeg.txt for more about this.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must emit data bottom-to-top,
 * you can use one of the virtual arrays provided by the JPEG memory manager
 * to invert the data.  See wrbmp.c for an example.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.txt.
 */


    const JOCTET * next_input_byte; /* => next byte to read from buffer */
    size_t bytes_in_buffer;         /* # of bytes remaining in buffer */

/*
        Initialize source. This is called by jpeg_read_header() before any
        data is actually read. Unlike init_destination(), it may leave
        bytes_in_buffer set to 0 (in which case a fill_input_buffer() call
        will occur immediately).
*/

static void init_stream_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}

/*
        This is called whenever bytes_in_buffer has reached zero and more
        data is wanted. In typical applications, it should read fresh data
        into the buffer (ignoring the current state of next_input_byte and
        bytes_in_buffer), reset the pointer & count to the start of the
        buffer, and return TRUE indicating that the buffer has been reloaded.
        It is not necessary to fill the buffer entirely, only to obtain at
        least one more byte. bytes_in_buffer MUST be set to a positive value
        if TRUE is returned. A FALSE return should only be used when I/O
        suspension is desired (this mode is discussed in the next section).
*/
static boolean fill_stream_input_buffer (j_decompress_ptr cinfo)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;
    size_t nbytes = 0;

    if (!g3d_stream_eof(src->stream))
	nbytes = g3d_stream_read(src->stream, src->buffer, BUFSIZE);

    if (nbytes <= 0)
    {
	if (src->start_of_file)	/* Treat empty input file as fatal error */
	    ERREXIT(cinfo, JERR_INPUT_EMPTY);
	WARNMS(cinfo, JWRN_JPEG_EOF);
	/* Insert a fake EOI marker */
	src->buffer[0] = (JOCTET) 0xFF;
	src->buffer[1] = (JOCTET) JPEG_EOI;
	nbytes = 2;
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file = FALSE;

    return TRUE;
}

/*
        Skip num_bytes worth of data. The buffer pointer and count should
        be advanced over num_bytes input bytes, refilling the buffer as
        needed. This is used to skip over a potentially large amount of
        uninteresting data (such as an APPn marker). In some applications
        it may be possible to optimize away the reading of the skipped data,
        but it's not clear that being smart is worth much trouble; large
        skips are uncommon. bytes_in_buffer may be zero on return.
        A zero or negative skip count should be treated as a no-op.
*/
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  struct jpeg_source_mgr * src = cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0)
  {
    while (num_bytes > (long) src->bytes_in_buffer)
    {
      num_bytes -= (long) src->bytes_in_buffer;
      (void) (*src->fill_input_buffer) (cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->next_input_byte += (size_t) num_bytes;
    src->bytes_in_buffer -= (size_t) num_bytes;
  }
}

/*
        This routine is called only when the decompressor has failed to find
        a restart (RSTn) marker where one is expected. Its mission is to
        find a suitable point for resuming decompression. For most
        applications, we recommend that you just use the default resync
        procedure, jpeg_resync_to_restart(). However, if you are able to back
        up in the input data stream, or if you have a-priori knowledge about
        the likely location of restart markers, you may be able to do better.
        Read the read_restart_marker() and jpeg_resync_to_restart() routines
        in jdmarker.c if you think you'd like to implement your own resync
        procedure.
*/
/*
//static void resync_to_restart (j_decompress_ptr cinfo, int desired)
//{
//}
*/

/*
        Terminate source --- called by jpeg_finish_decompress() after all
        data has been read. Often a no-op.
*/
static void term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}
/*
For both fill_input_buffer() and skip_input_data(), there is no such thing as an EOF return. If the end of the file has been reached, the routine has a choice of exiting via ERREXIT() or inserting fake data into the buffer. In most cases, generating a warning message and inserting a fake EOI marker is the best course of action --- this will allow the decompressor to output however much of the image is there. In pathological cases, the decompressor

may swallow the EOI and again demand data ... just keep feeding it fake EOIs.

jdatasrc.c illustrates the recommended error recovery behavior.

term_source() is NOT called by jpeg_abort() or jpeg_destroy(). If you want the source manager to be cleaned up during an abort, you must do it yourself.

You will also need code to create a jpeg_source_mgr struct, fill in its method pointers, and insert a pointer to the struct into the "src" field of the JPEG decompression object. This can be done in-line in your setup code if you like, but it's probably cleaner to provide a separate routine similar to the jpeg_stdio_src() routine of the supplied source manager.

For more information, consult the stdio source and destination managers in jdatasrc.c and jdatadst.c. 
*/

static void jpeg_stream_src(struct jpeg_decompress_struct *cinfo, G3DStream *stream)
{
    my_source_mgr *src;

    if (stream == NULL)
	ERREXIT(cinfo, JERR_INPUT_EMPTY);

  /* The source object is made permanent so that a series of JPEG images
   * can be read from the same buffer by calling jpeg_mem_src only before
   * the first one.
   */
  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(my_source_mgr));

    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, BUFSIZE * sizeof(JOCTET));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_stream_source;
  src->pub.fill_input_buffer = fill_stream_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->stream = stream;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}

