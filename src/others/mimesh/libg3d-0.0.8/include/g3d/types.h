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

#ifndef __G3D_TYPES_H__
#define __G3D_TYPES_H__

#include <GL/gl.h>

#include <glib.h>

#ifdef USE_LIBMAGIC
#include <magic.h>
#endif

#ifdef EAPI
# undef EAPI
#endif /* ifdef EAPI */

#ifdef _WIN32
# ifdef EFL_EET_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else /* ifdef DLL_EXPORT */
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else /* ifdef EFL_EET_BUILD */
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EET_BUILD */
#else /* ifdef _WIN32 */
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else /* if __GNUC__ >= 4 */
#   define EAPI
#  endif /* if __GNUC__ >= 4 */
# else /* ifdef __GNUC__ */
#  define EAPI
# endif /* ifdef __GNUC__ */
#endif /* ! _WIN32 */


#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * SECTION:types
 * @short_description: some defined types for libg3d
 * @include: g3d/g3d.h
 */
/* goffset is defined since glib 2.14 */
#ifndef G_MAXOFFSET
typedef gint64 goffset;
#endif

/* supposed to trick gtk-doc not to ignore these functions - not really... */
#define _G3D_STATIC_INLINE static inline

/*****************************************************************************
 * basic types
 *****************************************************************************/

/**
 * SECTION:types
 * @short_description: Basic types
 * @include: g3d/types.h
 *
 * Some basic type abstractions used in libg3d.
 */

/**
 * G3DFloat:
 *
 * Default floating point type. If used consistently in the library (not the
 * case at the moment) this type can be switched between single and double
 * precision at compile-time.
 */
/**
 * G3D_FLOAT_IS_DOUBLE
 *
 * A flag indicating type of #G3DFloat. It is TRUE if #G3DFloat is double and
 * FALSE if #G3DFloat is single precision.
 */
#if 1
#define G3D_FLOAT_IS_DOUBLE TRUE
#define G3D_SCANF_FLOAT "%lf"
typedef gdouble G3DFloat;
#else
#define G3D_FLOAT_IS_DOUBLE FALSE
#define G3D_SCANF_FLOAT "%f"
typedef gfloat G3DFloat;
#endif
/**
 * G3DSingle:
 *
 * Single-precision floating point number.
 */
typedef gfloat G3DSingle;
/**
 * G3DDouble:
 *
 * Double-precision floating point number.
 */
typedef gdouble G3DDouble;
/**
 * G3DVector
 *
 * Vector element type.
 */
typedef G3DFloat G3DVector;
/**
 * G3DMatrix:
 *
 * Matrix element type.
 */
typedef G3DFloat G3DMatrix;
/**
 * G3DQuat
 *
 * Quaternion element type.
 */
typedef G3DFloat G3DQuat;

/*****************************************************************************
 * G3DImage
 *****************************************************************************/

/**
 * G3D_FLAG_IMG_GREYSCALE:
 *
 * The image just uses the red channel for grey value
 */
#define G3D_FLAG_IMG_GREYSCALE       (1L << 1)

/**
 * G3DTexEnv:
 * @G3D_TEXENV_UNSPECIFIED: unspecified, application decides
 * @G3D_TEXENV_BLEND: use blending
 * @G3D_TEXENV_DECAL: use as decal
 * @G3D_TEXENV_MODULATE: use modulate
 * @G3D_TEXENV_REPLACE: replace color
 *
 * Specify how the texture should interact with other material properties.
 */
typedef enum {
	G3D_TEXENV_UNSPECIFIED = 0,
	G3D_TEXENV_BLEND,
	G3D_TEXENV_DECAL,
	G3D_TEXENV_MODULATE,
	G3D_TEXENV_REPLACE
} G3DTexEnv;

/**
 * G3DImage:
 * @name: name of image
 * @width: width of image in pixels
 * @height: height of image in pixels
 * @depth: depth of image in bits
 * @flags: flags
 * @pixeldata: the binary image data
 * @tex_id: the OpenGL texture id, should be unique model-wide
 * @tex_env: texture environment flags
 * @tex_scale_u: factor scaling texture width, should be 1.0 for most cases
 * @tex_scale_v: factor scaling texture height, should be 1.0 for most cases
 *
 * Object containing a two-dimensional pixel image.
 */
typedef struct {
	gchar *name;
	guint32 width;
	guint32 height;
	guint8 depth;
	guint32 flags;
	guint8 *pixeldata;

	guint32 tex_id;
	G3DTexEnv tex_env;
	G3DFloat tex_scale_u;
	G3DFloat tex_scale_v;
} G3DImage;

/*****************************************************************************
 * G3DMaterial
 *****************************************************************************/

/**
 * G3D_FLAG_MAT_TWOSIDE:
 *
 * Faces using this material should be rendered two-sided as the direction
 * is unknown.
 */
#define G3D_FLAG_MAT_TWOSIDE    (1L << 0)

/**
 * G3DMaterial:
 * @name: name of material
 * @r: red component of color
 * @g: green component of color
 * @b: blue component of color
 * @a: alpha component of color
 * @shininess: shiny color
 * @specular: specular color
 * @flags: flags
 * @tex_image: texture image (optional, may be NULL)
 *
 * A material object.
 */

typedef struct {
	gchar *name;
	G3DFloat r, g, b, a;
	GLfloat shininess;
	GLfloat specular[4];
	guint32 flags;

	G3DImage *tex_image;
} G3DMaterial;

/*****************************************************************************
 * G3DFace
 *****************************************************************************/

/**
 * G3D_FLAG_FAC_NORMALS:
 *
 * The face has custom normals.
 */
#define G3D_FLAG_FAC_NORMALS    (1L << 0)
/**
 * G3D_FLAG_FAC_TEXMAP:
 *
 * The face has a texture map and texture coordinates.
 */
#define G3D_FLAG_FAC_TEXMAP     (1L << 1)

/**
 * G3DFace:
 * @vertex_count: number of vertices
 * @vertex_indices: indices of vertices in #G3DObject
 * @material: material to use for surface
 * @flags: flags
 * @normals: optional normal array (one vector - 3 #G3DVector values - for each
 *   vertex)
 * @tex_image: optional texture image
 * @tex_vertex_count: number of texture vertices, should be 0 or match
 *   vertex_count
 * @tex_vertex_data: array of texture vertices
 *
 * An object representing a surface.
 */
typedef struct {
	guint32 vertex_count;
	guint32 *vertex_indices;

	G3DMaterial *material;

	guint32 flags;

	G3DVector *normals;

	G3DImage *tex_image;
	guint32 tex_vertex_count;
	G3DVector *tex_vertex_data;
} G3DFace;

/*****************************************************************************
 * G3DTransformation
 *****************************************************************************/

/**
 * G3DTransformation:
 * @matrix: the transformation matrix
 * @flags: flags
 *
 * A three-dimensional matrix transformation object.
 */
typedef struct {
	G3DMatrix matrix[16];
	guint32 flags;
} G3DTransformation;


/*****************************************************************************
 * G3DObject
 *****************************************************************************/

/**
 * G3DObject:
 * @name: name of object
 * @materials: list of materials
 * @faces: list of faces
 * @objects: list of sub-objects
 * @transformation: optional transformation
 * @hide: flag to disable object rendering
 * @vertex_count: number of vertices
 * @vertex_data: vertex vector data
 *
 * A three-dimensional object.
 */
typedef struct {
	gchar *name;

	GSList *materials;
	GSList *faces;
	GSList *objects;

	/* transformation, may be NULL */
	G3DTransformation *transformation;

	/* don't render this object */
	gboolean hide;

	/* vertices */
	guint32 vertex_count;
	G3DVector *vertex_data;

	/*< private >*/
	/* FIXME: texture stuff: temporary storage, should be in G3DFace items */
	guint32 tex_vertex_count;
	G3DVector *tex_vertex_data;
	G3DImage *tex_image;

	/*< private >*/
	/* some fields to speed up rendering, should not be used by plugins */
	/* FIXME: remove from API (replace with user_data pointer?) */
	G3DVector *_normals;
	G3DMaterial **_materials;
	guint32  _num_faces;
	guint32 *_indices;
	guint32 *_flags;
	guint32 *_tex_images;
	G3DVector *_tex_coords;
} G3DObject;

/*****************************************************************************
 * G3DContext
 *****************************************************************************/

/**
 * G3DSetBgColorFunc:
 * @r: red component
 * @g: green component
 * @b: blue component
 * @a: alpha component
 * @user_data: opaque data as given to g3d_context_set_set_bgcolor_func()
 *
 * Background color setting callback.
 *
 * Returns: TRUE on success, FALSE else.
 */
typedef gboolean (* G3DSetBgColorFunc)(G3DFloat r, G3DFloat g, G3DFloat b,
	G3DFloat a,	gpointer user_data);

/**
 * G3DUpdateInterfaceFunc:
 * @user_data: opaque data as given to g3d_context_set_update_interface_func()
 *
 * Interface updating callback.
 *
 * Returns: TRUE on success, FALSE else.
 */
typedef gboolean (* G3DUpdateInterfaceFunc)(gpointer user_data);

/**
 * G3DUpdateProgressBarFunc:
 * @percentage: progress of plugin operation
 * @show: #TRUE if the progress bar should be visible, #FALSE else
 * @user_data: opaque data as given to
 * g3d_context_set_update_progress_bar_func()
 *
 * Progress updating callback.
 *
 * Returns: TRUE on success, FALSE else.
 */
typedef gboolean (* G3DUpdateProgressBarFunc)(G3DFloat percentage,
	gboolean show, gpointer user_data);

/*< private >*/
#ifdef USE_LIBMAGIC
#define MAGIC_PTR_TYPE magic_t
#else
#define MAGIC_PTR_TYPE void *
#endif
/*< public >*/

/**
 * G3DContext:
 *
 * Internal stuff for libg3d.
 */
typedef struct {
	/*< private >*/
	GSList *plugins;
	MAGIC_PTR_TYPE magic_cookie;

	GHashTable *exts_import;
	GHashTable *exts_image;

	G3DSetBgColorFunc set_bgcolor_func;
	gpointer set_bgcolor_data;
	G3DUpdateInterfaceFunc update_interface_func;
	gpointer update_interface_data;
	G3DUpdateProgressBarFunc update_progress_bar_func;
	gpointer update_progress_bar_data;

	GHashTable *modelCache;
} G3DContext;

/*****************************************************************************
 * G3DPlugin: real definition in g3d/plugins.h
 *****************************************************************************/

typedef struct _G3DPlugin G3DPlugin;

/*****************************************************************************
 * G3DStream
 *****************************************************************************/

/* declared in stream.h */
typedef struct _G3DStream G3DStream;

/*****************************************************************************
 * G3DModel
 *****************************************************************************/

/**
 * G3DModel:
 * @filename: file name or URI of loaded model, may be set by application
 * @materials: list of materials (#G3DMaterial)
 * @objects: list of objects (#G3DObject)
 *
 * A 3D model.
 */
typedef struct {
	gchar *filename;
	/*< private >*/
	G3DContext *context;

	/*< public >*/
	GSList *materials;
	GSList *objects;

	/*< private >*/
	GHashTable *tex_images;

	/* the plugin used to load the model, may be NULL */
	G3DPlugin *plugin;
	/* The stream used to load the model, may be NULL */
	G3DStream *stream;

	int refCount;
} G3DModel;

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* __G3D_TYPES_H__ */

