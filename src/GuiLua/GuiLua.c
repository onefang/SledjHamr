/*  GuiLua - a GUI library that implements matrix-RAD style stuff.

Provides the skang and widget Lua packages.

In the initial intended use case, several applications will be using
this all at once, with one central app hosting all the GUIs.

Basically this should deal with "windows" and their contents.  A
"window" in this case is hosted in the central app as some sort of
internal window, but the user can "tear off" those windows, then they
get their own OS hosted window.  This could be done by the hosting app
sending the current window contents to the original app as a skang file.

Between the actual GUI and the app might be a socket, or a stdin/out
pipe.  Just like matrix-RAD, this should be transparent to the app. 
Also just like matrix-RAD, widgets can be connected to variable /
functions (C or Lua), and any twiddlings with those widgets runs the
function / changes the variable, again transparent to the app, except
for any registered get/set methods.

This interface between the GUI and the app is "skang" files, which are
basically Lua scripts.  The GUI and the app can send skang files back
and forth, usually the app sends actual GUI stuff, and usually the GUI
sends variable twiddles or action calls.  Usually.

To start with, this will be used to support multiple apps hosting their
windows in extantz, allowing the big viewer blob to be split up into
modules.  At some point converting LL XML based UI shit into skang could
be done.  Also, this should be an exntension to LuaSL, so in-world
scripts can have a poper GUI for a change.


NOTES and TODOs -

Lua scripts do -
  require 'widget'  -> loads widget.c
  Widget.c is a library like test_c.
  It starts up GuiLua.c app, with stdin/stdout pipe.
  Widget.c then acts as a proxy for all the widget stuff.
  So Lua modules via C libraries can work with Elm code that has a special main and has to be an app.
  Seems simplest.

  Also -
    Some of this gets shared with LuaSL, since it came from there anyway.

  Finally -
    Add a --gui command line option, that runs foo.skang.
    Than change the hash bang to use it.
    And if there's a matching module, load the module first, call gimmeSkin() on it.
    So that any with an internal default skin get that instead.
    Same if there's a module, but no skang file.

Making these packages all a sub package of skang seems like a great
idea.  On the other hand, looks like most things are just getting folded
into skang anyway.  See
http://www.inf.puc-rio.br/~roberto/pil2/chapter15.pdf part 15.5 for
package details.

See if I can use LuaJIT FFI here.  Since this will be a library, and
skang apps could be written in C or Lua, perhaps writing this library to
be FFI friendly instead of the usual Lua C binding might be the way to
go?  LuaJIT is not ready yet, since it needs include files copied into
Lua files, and does not support macros, which EFL uses a lot of.

For the "GUI hosted in another app" case, we will need some sort of
internal window manager running in that other app.

This might end up running dozens of Lua scripts, and could use the LuaSL
Lua script running system.  Moving that into this library might be a
sane idea I think?  Or prehaps a separate library that both LuaSL and
GuiLua use?

Raster wants a method of sending Lua tables around as edje messages. 
Between C, Edje, Edje Lua, and Lua.  Sending between threads, and across
sockets.  Using a new edje message type, or eet for sockets, was
suggested, but perhaps Lua skang is a better choice?

Somehow access to the edje_lua2.c bindings should be provided.  And
bindings to the rest of EFL when they are done.  Assuming the other EFL
developers do proper introspection stuff, or let me do it.

The generic Lua binding helper functions I wrote for edje_lua2.c could
be used here as well, and expanded as discussed on the E devs mailing
list.  This would include the thread safe Lua function stuff copied
into the README.

There will eventually be a built in editor, like the zen editor from
matrix-RAD.  It might be a separate app.

NAWS should probably live in here to.  If I ever get around to writing
it.  lol

The pre tokenized widget structure thingy I had planned in the
matrix-RAD TODO just wont work, as it uses symbols.  On the other hand,
we will be using Lua tables anyway.  B-)

The last half of http://passingcuriosity.com/2009/extending-lua-in-c/
might be of use.

*/


/* thing package

Currently this is in skang.lua, but should bring this in here later.

*/


/* skang package

Currently this is in skang.lua, but should bring this in here later.

*/


/* stuff & squeal packages

Currently Stuff is in skang.lua, but should bring this in here later.

*/


/* widget package

Currently widget design is in skang.lua, but should bring this in here later.

*/


/* introspection

As detailed in README, EFL introspection doesn't seem to really be on
the radar, but I might get lucky, or I might have to write it myself. 
For quick and dirty early testing, I'll probably write a widget package
that has hard coded mappings between some basic "label", "button", etc.
and ordinary elementary widgets.  Proper introspection can come later.

*/



#include "GuiLua.h"



globals ourGlobals;
static const char *globName  = "ourGlobals";



typedef struct _Scene_Data
{
  Evas_Object      *image;		// Our Elm image.
  Evas_3D_Scene    *scene;
  Evas_3D_Node     *root_node;
  Evas_3D_Node     *camera_node;
  Evas_3D_Node     *light_node;

  Evas_3D_Camera   *camera;
  Evas_3D_Light    *light;

  Evas_3D_Mesh     *mesh;
  Evas_3D_Node     *mesh_node;
  Evas_3D_Material *material0;
  Evas_3D_Material *material1;
  Evas_3D_Texture  *texture0;
  Evas_3D_Texture  *texture1;
  Evas_3D_Texture  *texture_normal;

  Evas_3D_Mesh     *mesh2;
  Evas_3D_Node     *mesh2_node;
  Evas_3D_Material *material2;
  Evas_3D_Texture  *texture2;

  Evas_3D_Mesh     *mesh3;
  Evas_3D_Node     *mesh3_node;
  Evas_3D_Material *material3;
  Evas_3D_Texture  *texture_diffuse;

} Scene_Data;

static Scene_Data ourScene;


static const float cube_vertices[] =
{
   /* Front */
   -1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,

   /* Back */
    1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,
   -1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Left */
   -1.0,  1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,
   -1.0,  1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Right */
    1.0,  1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Top */
   -1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
   -1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Bottom */
    1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,
   -1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,
};

static const unsigned short cube_indices[] =
{
   /* Front */
   0,   1,  2,  2,  1,  3,

   /* Back */
   4,   5,  6,  6,  5,  7,

   /* Left */
   8,   9, 10, 10,  9, 11,

   /* Right */
   12, 13, 14, 14, 13, 15,

   /* Top */
   16, 17, 18, 18, 17, 19,

   /* Bottom */
   20, 21, 22, 22, 21, 23
};

static const unsigned int pixels0[] =
{
   0xff0000ff, 0xff0000ff, 0xffff0000, 0xffff0000,
   0xff0000ff, 0xff0000ff, 0xffff0000, 0xffff0000,
   0xff00ff00, 0xff00ff00, 0xff000000, 0xff000000,
   0xff00ff00, 0xff00ff00, 0xff000000, 0xff000000,
};

static const unsigned int pixels1[] =
{
   0xffff0000, 0xffff0000, 0xff00ff00, 0xff00ff00,
   0xffff0000, 0xffff0000, 0xff00ff00, 0xff00ff00,
   0xff0000ff, 0xff0000ff, 0xffffffff, 0xffffffff,
   0xff0000ff, 0xff0000ff, 0xffffffff, 0xffffffff,
};


typedef struct _vec4
{
    float   x;
    float   y;
    float   z;
    float   w;
} vec4;

typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

typedef struct _vec2
{
    float   x;
    float   y;
} vec2;

typedef struct _vertex
{
    vec3    position;
    vec3    normal;
    vec3    tangent;
    vec4    color;
    vec3    texcoord;
} vertex;

static int		vertex_count = 0;
static vertex	       *sphere_vertices = NULL;

static int		index_count = 0;
static unsigned short  *sphere_indices = NULL;

static inline vec3
_normalize(const vec3 *v)
{
    double  l = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    vec3    vec;

    vec.x = v->x / l;
    vec.y = v->y / l;
    vec.z = v->z / l;

    return vec;
}

static void
_sphere_fini()
{
    if (sphere_vertices)
	free(sphere_vertices);

    if (sphere_indices)
	free(sphere_indices);
}

static void
_sphere_init(int precision)
{
    int		    i, j;
    unsigned short *index;

    vertex_count = (precision + 1) * (precision + 1);
    index_count = precision * precision * 6;

    /* Allocate buffer. */
    sphere_vertices = malloc(sizeof(vertex) * vertex_count);
    sphere_indices = malloc(sizeof(unsigned short) * index_count);

    for (i = 0; i <= precision; i++)
    {
	double lati = (M_PI * (double)i) / (double)precision;
	double y = cos(lati);
	double r = fabs(sin(lati));

	for (j = 0; j <= precision; j++)
	{
	    double longi = (M_PI * 2.0 * j) / precision;
	    vertex *v = &sphere_vertices[i * (precision  + 1) + j];

	    if (j == 0 || j == precision)
		v->position.x = 0.0;
	    else
		v->position.x = r * sin(longi);

	    v->position.y = y;

	    if (j == 0 || j == precision)
		v->position.z = r;
	    else
		v->position.z = r * cos(longi);

	    v->normal = v->position;

	    if (v->position.x > 0.0)
	    {
		v->tangent.x = -v->normal.y;
		v->tangent.y =  v->normal.x;
		v->tangent.z =  v->normal.z;
	    }
	    else
	    {
		v->tangent.x =  v->normal.y;
		v->tangent.y = -v->normal.x;
		v->tangent.z =  v->normal.z;
	    }

	    v->color.x = v->position.x;
	    v->color.y = v->position.y;
	    v->color.z = v->position.z;
	    v->color.w = 1.0;

	    if (j == precision)
		v->texcoord.x = 1.0;
	    else if (j == 0)
		v->texcoord.x = 0.0;
	    else
		v->texcoord.x = (double)j / (double)precision;

	    if (i == precision)
		v->texcoord.y = 1.0;
	    else if (i == 0)
		v->texcoord.y = 0.0;
	    else
		v->texcoord.y = 1.0 - (double)i / (double)precision;
	}
    }

    index = &sphere_indices[0];

    for (i = 0; i < precision; i++)
    {
	for (j = 0; j < precision; j++)
	{
	    *index++ = i * (precision + 1) + j;
	    *index++ = i * (precision + 1) + j + 1;
	    *index++ = (i + 1) * (precision + 1) + j;

	    *index++ = (i + 1) * (precision + 1) + j;
	    *index++ = i * (precision + 1) + j + 1;
	    *index++ = (i + 1) * (precision + 1) + j + 1;
	}
    }

    for (i = 0; i < index_count; i += 3)
    {
	vertex *v0 = &sphere_vertices[sphere_indices[i + 0]];
	vertex *v1 = &sphere_vertices[sphere_indices[i + 1]];
	vertex *v2 = &sphere_vertices[sphere_indices[i + 2]];

	vec3	e1, e2;
	float	du1, du2, dv1, dv2, f;
	vec3	tangent;

	e1.x = v1->position.x - v0->position.x;
	e1.y = v1->position.y - v0->position.y;
	e1.z = v1->position.z - v0->position.z;

	e2.x = v2->position.x - v0->position.x;
	e2.y = v2->position.y - v0->position.y;
	e2.z = v2->position.z - v0->position.z;

	du1 = v1->texcoord.x - v0->texcoord.x;
	dv1 = v1->texcoord.y - v0->texcoord.y;

	du2 = v2->texcoord.x - v0->texcoord.x;
	dv2 = v2->texcoord.y - v0->texcoord.y;

	f = 1.0 / (du1 * dv2 - du2 * dv1);

	tangent.x = f * (dv2 * e1.x - dv1 * e2.x);
	tangent.y = f * (dv2 * e1.y - dv1 * e2.y);
	tangent.z = f * (dv2 * e1.z - dv1 * e2.z);

	v0->tangent = tangent;
    }

    for (i = 0; i <= precision; i++)
    {
	for (j = 0; j <= precision; j++)
	{
	    if (j == precision)
	    {
		vertex *v = &sphere_vertices[i * (precision  + 1) + j];
		v->tangent = sphere_vertices[i * (precision + 1)].tangent;
	    }
	}
    }
}


static Eina_Bool
_animate_scene(void *data)
{
  static float angle = 0.0f;
  static float earthAngle = 0.0f;
  static int   frame = 0;
  static int   inc   = 1;
  static int   sonicFrame = 0;

  Scene_Data *scene = (Scene_Data *)data;

  // Animate cube.
  angle += 0.5;
  if (angle > 360.0)		angle -= 360.0f;

  frame += inc;
  if (frame >= 20) inc = -1;
  else if (frame <= 0) inc = 1;

  eo_do(scene->mesh_node,
    evas_3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, 1.0),
    evas_3d_node_mesh_frame_set(scene->mesh, frame)
    );

  // Animate sonic.
  sonicFrame += 32;
  if (sonicFrame > 256 * 50)	sonicFrame = 0;
  eo_do(scene->mesh2_node,
    evas_3d_node_mesh_frame_set(scene->mesh2, sonicFrame)
    );

  // Animate earth.
  earthAngle += 0.3;
  if (earthAngle > 360.0)	earthAngle -= 360.0f;
  eo_do(scene->mesh3_node,
    evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0)
    );

  return EINA_TRUE;
}


static void
_camera_setup(globals *ourGlobals, Scene_Data *scene)
{
  scene->camera = eo_add(EVAS_3D_CAMERA_CLASS, ourGlobals->evas);
  eo_do(scene->camera,
    evas_3d_camera_projection_perspective_set(60.0, 1.0, 1.0, 500.0)
    );

  scene->camera_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_CAMERA);
  eo_do(scene->camera_node,
    evas_3d_node_camera_set(scene->camera)
    );
  eo_do(scene->root_node, evas_3d_node_member_add(scene->camera_node));
  eo_do(scene->camera_node,
    evas_3d_node_position_set(50.0, 0.0, 20.0),
    evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 20.0, EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0)
    );
}

static void
_light_setup(globals *ourGlobals, Scene_Data *scene)
{
  scene->light = eo_add(EVAS_3D_LIGHT_CLASS, ourGlobals->evas);
  eo_do(scene->light,
    evas_3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_directional_set(EINA_TRUE)
    );

  scene->light_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_LIGHT);
  eo_do(scene->light_node,
    evas_3d_node_light_set(scene->light)
    );
  eo_do(scene->root_node,
    evas_3d_node_member_add(scene->light_node)
    );
  eo_do(scene->light_node,
    evas_3d_node_position_set(1000.0, 0.0, 1000.0),
    evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0)
    );
}

static void _cube_setup(globals *ourGlobals, Scene_Data *scene)
{
  // Setup cube materials.
  scene->material0 = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas);
  scene->material1 = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas);

  eo_do(scene->material0,
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

    evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_shininess_set(100.0)
    );

  eo_do(scene->material1,
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

    evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_shininess_set(100.0)
    );

  scene->texture0 = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas);
  scene->texture1 = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas);
  scene->texture_normal = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas);

  eo_do(scene->texture0,
    evas_3d_texture_data_set(EVAS_3D_COLOR_FORMAT_RGBA, EVAS_3D_PIXEL_FORMAT_8888, 4, 4, &pixels0[0])
    );
  eo_do(scene->texture1,
    evas_3d_texture_data_set(EVAS_3D_COLOR_FORMAT_RGBA, EVAS_3D_PIXEL_FORMAT_8888, 4, 4, &pixels1[0])
    );
  eo_do(scene->texture_normal, evas_3d_texture_file_set("../../media/normal_lego.png", NULL));

  eo_do(scene->material0, evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, scene->texture0));
  eo_do(scene->material1, evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, scene->texture1));
  eo_do(scene->material1, evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL, scene->texture_normal));

  // Setup CUBE mesh.
  scene->mesh = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas);
  eo_do(scene->mesh,
    evas_3d_mesh_vertex_count_set(24),
    evas_3d_mesh_frame_add(0),

    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION, 12 * sizeof(float), &cube_vertices[ 0]),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,   12 * sizeof(float), &cube_vertices[ 3]),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,    12 * sizeof(float), &cube_vertices[ 6]),
    evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD, 12 * sizeof(float), &cube_vertices[10]),

    evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]),
    evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),

    evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP),

    evas_3d_mesh_frame_material_set(0, scene->material0),

    evas_3d_mesh_frame_add(20),
    evas_3d_mesh_frame_material_set(20, scene->material1)
    );
  scene->mesh_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_MESH);
  eo_do(scene->root_node, evas_3d_node_member_add(scene->mesh_node));
  eo_do(scene->mesh_node,
    eo_key_data_set("Name", "cube", NULL),
    evas_3d_node_position_set(40.0, 3.5, 23.0),
    evas_3d_node_mesh_add(scene->mesh)
    );
}

static void _sonic_setup(globals *ourGlobals, Scene_Data *scene)
{
  // Setup an MD2 mesh.
  scene->mesh2 = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas);
  eo_do(scene->mesh2,
    evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, "../../media/sonic.md2", NULL)
    );

  scene->material2 = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas);
  eo_do(scene->mesh2,
    evas_3d_mesh_frame_material_set(0, scene->material2)
    );

  scene->texture2 = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas);
  eo_do(scene->texture2,
    evas_3d_texture_file_set("../../media/sonic.png", NULL),
    evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),
    evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT)
    );

  eo_do(scene->material2,
    evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, scene->texture2),

    evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
    evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

    evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
    evas_3d_material_shininess_set(50.0)
  );

  scene->mesh2_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_MESH);
  eo_do(scene->root_node,
    evas_3d_node_member_add(scene->mesh2_node)
    );
  eo_do(scene->mesh2_node,
    eo_key_data_set("Name", "sonic", NULL),
    evas_3d_node_mesh_add(scene->mesh2)
    );

  eo_do(scene->mesh2,
    evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG)
    );
}

static void _earth_setup(globals *ourGlobals, Scene_Data *scene)
{
  // Setup earth material.
   scene->material3 = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas);

   scene->texture_diffuse = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas);
   eo_do(scene->texture_diffuse,
         evas_3d_texture_file_set("../../media/EarthDiffuse.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_LINEAR, EVAS_3D_TEXTURE_FILTER_LINEAR));
   eo_do(scene->material3,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, scene->texture_diffuse),

         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

  // Setup earth mesh.
   _sphere_init(100);

   scene->mesh3 = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas);
   eo_do(scene->mesh3,
         evas_3d_mesh_vertex_count_set(vertex_count),
         evas_3d_mesh_frame_add(0),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION, sizeof(vertex), &sphere_vertices[0].position),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,   sizeof(vertex), &sphere_vertices[0].normal),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TANGENT,  sizeof(vertex), &sphere_vertices[0].tangent),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,    sizeof(vertex), &sphere_vertices[0].color),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD, sizeof(vertex), &sphere_vertices[0].texcoord),

         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &sphere_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
         evas_3d_mesh_frame_material_set(0, scene->material3)
         );

  scene->mesh3_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_MESH);
  eo_do(scene->root_node,
    evas_3d_node_member_add(scene->mesh3_node)
    );
  eo_do(scene->mesh3_node,
    eo_key_data_set("Name", "earth", NULL),
    evas_3d_node_position_set(40.0, -3.5, 23.0),
    evas_3d_node_mesh_add(scene->mesh3)
    );

  eo_do(scene->mesh3,
    evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE)
    );
}


static void
_scene_setup(globals *ourGlobals, Scene_Data *scene)
{
  scene->scene = eo_add(EVAS_3D_SCENE_CLASS, ourGlobals->evas);
  eo_do(scene->scene,
    evas_3d_scene_size_set(512, 512),
    evas_3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0)
    );

  // TODO - I have no idea how this should work.
  // It seems the people that wrote the examples don't know either.  lol
//  scene->root_node = eo_add(EVAS_3D_NODE_CLASS, ourGlobals->evas, EVAS_3D_NODE_TYPE_NODE);
  scene->root_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_NODE);

  _camera_setup(ourGlobals, scene);
  _light_setup(ourGlobals, scene);
  _cube_setup(ourGlobals, scene);
  _sonic_setup(ourGlobals, scene);
  _earth_setup(ourGlobals, scene);

  eo_do(scene->scene,
    evas_3d_scene_root_node_set(scene->root_node),
    evas_3d_scene_camera_node_set(scene->camera_node)
);
}

/* Sooo, how to do this -
widget has to be a light userdata
The rest can be Lua sub things?  Each with a C function to update the widget.

win.quitter:colour(1,2,3,4)  -> win.quitter.colour(win.quitter, 1,2,3,4)  ->  __call(win.quitter.colour, win.quitter, 1,2,3,4)  ->  skang.colour(win.quitter.colour, win.quitter, 1,2,3,4)
win.quitter.colour.r = 5     -> direct access to the table, well "direct" via Thing and Mum.  We eventually want to call skang.colour() though.
*/

struct _Widget
{
  char		magic[8];
  Evas_Object	*obj;
  Eina_Clist	node;
  char		*label, *look, *action, *help;
  // foreground / background colour
  // thing
  // types {}
  // skangCoord x, y, w, h
};


// TODO - These functions should be able to deal with multiple windows.
// TODO - Should be able to open external and internal windows, and even switch between them on the fly.
static void _on_click(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
  globals *ourGlobals;
  lua_State *L = data;
  struct _Widget *wid;

  lua_getfield(L, LUA_REGISTRYINDEX, globName);
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  wid = evas_object_data_get(obj, "Widget");
  if (wid)
  {
    PD("Doing action %s", wid->action);
    if (0 != luaL_dostring(L, wid->action))
      PE("Error running - %s", wid->action);
  }
}

static void _on_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *o, void *einfo)
{
   Scene_Data *scene = data;
   Evas_Event_Mouse_Move *ev = einfo;
   Evas_Coord x, y, w, h;
   Evas_Coord obj_x, obj_y;
   int scene_w, scene_h;
   Evas_Real scene_x, scene_y;
   Evas_Real s, t;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Eina_Bool pick;
   char *name = NULL;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   obj_x = ev->cur.canvas.x - x;
   obj_y = ev->cur.canvas.y - y;

   eo_do(scene->scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (Evas_Real)w;
   scene_y = obj_y * scene_h / (Evas_Real)h;

   eo_do(scene->scene, pick = evas_3d_scene_pick(scene_x, scene_y, &n, &m, &s, &t));
   if (pick)
     name = evas_object_data_get(n, "Name");
   // This is a raw Evas callback, on the Elm image internal Evas_Object.
   // So we need to get the Elm Image back from the raw Evas_Object.
   // Which is why we stuffed it in the scene structure.
   if (name)
   {
      elm_object_tooltip_text_set(scene->image, name);
      elm_object_tooltip_show(scene->image);
   }
   else
      elm_object_tooltip_hide(scene->image);
}

static void _on_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *o, void *einfo)
{
   Scene_Data *scene = data;
   Evas_Event_Mouse_Down *ev = einfo;
   Evas_Coord x, y, w, h;
   Evas_Coord obj_x, obj_y;
   int scene_w, scene_h;
   Evas_Real scene_x, scene_y;
   Evas_Real s, t;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Eina_Bool pick;
   char *name;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   obj_x = ev->canvas.x - x;
   obj_y = ev->canvas.y - y;

   eo_do(scene->scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (Evas_Real)w;
   scene_y = obj_y * scene_h / (Evas_Real)h;

   eo_do(scene->scene, pick = evas_3d_scene_pick(scene_x, scene_y, &n, &m, &s, &t));
   if (pick)
   {
     name = evas_object_data_get(n, "Name");
     printf("Picked     : ");
   }
   else
     printf("Not picked : ");
   if (NULL == name)
     name = "";

    printf("output(%d, %d) canvas(%d, %d) object(%d, %d) scene(%f, %f) texcoord(%f, %f) "
           "node(%p) %s mesh(%p)\n",
           ev->output.x, ev->output.y,
           ev->canvas.x, ev->canvas.y,
           obj_x, obj_y,
           scene_x, scene_y,
           s, t, n, name, m);
}

static void _on_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
//  globals *ourGlobals = data;

  // Tell the main loop to stop, which it will, eventually.
  elm_exit();
}

static int widget(lua_State *L)
{
  globals *ourGlobals;
  char *type = "label";
  char *title = ":";
  int x = 1, y = 1, w = WIDTH/3, h = HEIGHT/3;

  lua_getfield(L, LUA_REGISTRYINDEX, globName);
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  pull_lua(L, 1, "$type $title %x %y %w %h", &type, &title, &x, &y, &w, &h);

  // Poor mans introspection, until I write real introspection into EFL.
  // TODO - The alternative is to just lookup the ELM_*_CLASS in a hash table?
  if (strcmp(type, "button") == 0)
  {
    struct _Widget *wid;

    wid = calloc(1, sizeof(struct _Widget));
    strcpy(wid->magic, "Widget");
    eina_clist_add_head(&ourGlobals->widgets, &wid->node);
    wid->label = strdup(title);

    // These two lines are likely the only ones that will be different for the different sorts of widgets.
    wid->obj = eo_add(ELM_OBJ_BUTTON_CLASS, ourGlobals->win);
    evas_object_smart_callback_add(wid->obj, "clicked", _on_click, L);

    elm_object_part_text_set(wid->obj, NULL, wid->label);
    eo_do(wid->obj,
	evas_obj_size_set(w, h),
	evas_obj_position_set(x, y),
	evas_obj_visibility_set(EINA_TRUE),
	eo_key_data_set("Widget", wid, NULL)
	);

    /* Evas_Object *bt isn't a real pointer it seems.  At least Lua bitches about it -
         PANIC: unprotected error in call to Lua API (bad light userdata pointer)
       So we wrap the _Widget instead of the Evas_Object.
       TODO - Might as well make _Widget a full userdata.
    */
    lua_pushlightuserdata(L, (void *) wid);
    return 1;
  }

  return 0;
}

static int action(lua_State *L)
{
  globals *ourGlobals;
  struct _Widget *wid = lua_touserdata(L, 1);
  char *action = "nada";

  lua_getfield(L, LUA_REGISTRYINDEX, globName);
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  pull_lua(L, 2, "$", &action);
  if (wid && strcmp(wid->magic, "Widget") == 0)
  {
    PD("Setting action %s", action);
    wid->action = strdup(action);
  }
  return 0;
}

static int colour(lua_State *L)
{
// TODO - This is just a stub for now.

  return 0;
}

static int window(lua_State *L)
{
  globals *ourGlobals;
  char *name = "GuiLua";
  char *title = "GuiLua test harness";
  Evas_Object *obj, *temp;
  int result = 0;
  int w = WIDTH, h = HEIGHT;

  lua_getfield(L, LUA_REGISTRYINDEX, globName);
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  pull_lua(L, 1, "%w %h $title $name", &w, &h, &title, &name);

  // Set the engine to opengl_x11, then open the window.
  elm_config_preferred_engine_set("opengl_x11");
  if ((ourGlobals->win = elm_win_util_standard_add(name, title)))
  {
    eina_clist_init(&ourGlobals->widgets);

    evas_object_smart_callback_add(ourGlobals->win, "delete,request", _on_done, ourGlobals);
    evas_object_resize(ourGlobals->win, w, h);
    evas_object_move(ourGlobals->win, 0, 0);
    evas_object_show(ourGlobals->win);

    // Get the Evas / canvas from the elm window (that the Evas_Object "lives on"), which is itself an Evas_Object created by Elm, so not sure if it was created internally with Ecore_Evas.
    ourGlobals->evas = evas_object_evas_get(ourGlobals->win);

    _scene_setup(ourGlobals, &ourScene);

    // Add a background image object.
    obj = eo_add(ELM_OBJ_IMAGE_CLASS, ourGlobals->win);
    eo_do(obj,
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
	elm_obj_image_fill_outside_set(EINA_TRUE),
	elm_obj_image_file_set("../../media/sky_01.jpg", NULL),
	evas_obj_visibility_set(EINA_TRUE)
	);
    elm_win_resize_object_add(ourGlobals->win, obj);
    eo_unref(obj);

    // Add an image object for 3D scene rendering.
    obj = eo_add(ELM_OBJ_IMAGE_CLASS, ourGlobals->win);
    ourScene.image = obj;
    eo_do(obj,
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
	elm_obj_image_fill_outside_set(EINA_TRUE),
	evas_obj_visibility_set(EINA_TRUE),
	temp = elm_obj_image_object_get()
	);
    eo_do(temp,
	evas_obj_image_scene_set(ourScene.scene)
	);
    elm_object_tooltip_text_set(obj, "");
    elm_object_tooltip_hide(obj);
    // Elm can't seem to be able to tell us WHERE an image was clicked, so use raw Evas calbacks instead.
    evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, &ourScene);
    evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, &ourScene);
    elm_win_resize_object_add(ourGlobals->win, obj);

    // Add animation timer callback.
    ecore_timer_add(0.016, _animate_scene, &ourScene);

    lua_pushlightuserdata(L, &ourGlobals->win);
    result = 1;
  }

  // Set preferred engine back to default from config
  elm_config_preferred_engine_set(NULL);

  return result;
}

static int clear(lua_State *L)
{
// TODO - This is just a stub for now.

  return 0;
}

static int loopWindow(lua_State *L)
{
  globals *ourGlobals;

  lua_getfield(L, LUA_REGISTRYINDEX, globName);
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (ourGlobals->win)
    elm_run();

  return 0;
}

static int quit(lua_State *L)
{
  globals *ourGlobals;

  lua_getfield(L, LUA_REGISTRYINDEX, globName);
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  _on_done(ourGlobals, NULL, NULL);

  return 0;
}

static int closeWindow(lua_State *L)
{
  globals *ourGlobals;

  lua_getfield(L, LUA_REGISTRYINDEX, globName);
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (ourGlobals->win)
  {
    struct _Widget *wid;

    // Elm will delete our widgets, but if we are using eo, we need to unref them.
    EINA_CLIST_FOR_EACH_ENTRY(wid, &ourGlobals->widgets, struct _Widget, node)
    {
      eo_unref(wid->obj);
    }
    eo_unref(ourScene.image);
    evas_object_del(ourGlobals->win);
    _sphere_fini();
  }

  if (ourGlobals->logDom >= 0)
  {
    eina_log_domain_unregister(ourGlobals->logDom);
    ourGlobals->logDom = -1;
  }

  // This shuts down Elementary, but keeps the main loop running until all ecore_evas are freed.
  elm_shutdown();

  return 0;
}

/* local widget = require 'libGuiLua'

Lua's require() function will strip any stuff from the front of the name
separated by a hyphen, so 'ClientHamr-GuiLua-libGuiLua' -> 'libGuiLua'.  Then
it will search through a path, and eventually find this libGuiLua.so (or
libGuiLua.dll or whatever), then call luaopen_libGuiLua(), which should return
a table.  The argument (only thing on the stack) for this function will
be 'libGuiLua'.

Normally luaL_register() creates a table of functions, that is the table
returned, but we want to do something different with skang.
*/
int luaopen_GuiLua(lua_State *L)
{
  int skang;

  // In theory this function only ever gets called once.
  memset(&ourGlobals, 0, sizeof(globals));
  ourGlobals.logDom = loggingStartup("GuiLua", ourGlobals.logDom);

  elm_policy_set(ELM_POLICY_EXIT,	ELM_POLICY_EXIT_NONE);
  elm_policy_set(ELM_POLICY_QUIT,	ELM_POLICY_QUIT_NONE);
  elm_policy_set(ELM_POLICY_THROTTLE,	ELM_POLICY_THROTTLE_HIDDEN_ALWAYS);

  // These are set via the elementary_config tool, which is hard to find.
  elm_config_finger_size_set(0);
  elm_config_scale_set(1.0);

// pseudo-indices, special tables that can be accessed like the stack -
//    LUA_GLOBALSINDEX - thread environment, where globals are
//    LUA_ENVIRONINDEX - C function environment, in this case luaopen_widget() is the C function
//    LUA_REGISTRYINDEX - C registry, global, for unique keys use the module name as a string, or a lightuserdata address to a C object in our module.
//    lua_upvalueindex(n) - C function upvalues

  // Shove ourGlobals into the registry.
  lua_pushlightuserdata(L, &ourGlobals);
  lua_setfield(L, LUA_REGISTRYINDEX, globName);

  // The skang module should have been loaded by now, so we can just grab it out of package.loaded[].
  lua_getglobal(L, "package");
  lua_getfield(L, lua_gettop(L), "loaded");
  lua_remove(L, -2);					// Removes "package"
  lua_getfield(L, lua_gettop(L), SKANG);
  lua_remove(L, -2);					// Removes "loaded"
  lua_setfield(L, LUA_REGISTRYINDEX, SKANG);
  lua_getfield(L, LUA_REGISTRYINDEX, SKANG);	// Puts the skang table back on the stack.
  skang = lua_gettop(L);

  // Define our functions.
//thingasm{'window', 'The size and title of the application Frame.', window, 'x,y,name', acl='GGG'}
  push_lua(L, "@ ( { = $ $ & $ $acl } )",	skang, THINGASM, skang, "Cwindow",	"Opens our window.",				window, "number,number,string", "GGG", 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "clear",	"The current skin is cleared of all widgets.",	clear, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "widget",	"Create a widget.",				widget, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "action",	"Add an action to a widget.",			action, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "Colour",	"Change widget colours.",			colour, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "loopWindow",	"Run our windows main loop.",			loopWindow, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "quit",		"Quit, exit, remove thyself.",			quit, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "closeWindow",	"Closes our window.",				closeWindow, 0);

  // A test of the array building stuff.
  push_lua(L, "@ ( { = $ $ % $widget !required } )", skang, THINGASM, skang, "wibble", "It's wibbly!", 1, "'edit', 'The wibblinator:', 1, 1, 10, 50", 1, 0);

  // Makes no difference what we return, but it's expecting something.
  return 1;
}


void GuiLuaDo(int argc, char **argv)
{
  lua_State  *L;
  lua_Number  i;

  L = luaL_newstate();
  if (L)
  {
    luaL_openlibs(L);

    // Pass all our command line arguments to Lua.
    i = 1;
    lua_newtable(L);
    while (--argc > 0 && *++argv != '\0')
    {
      lua_pushnumber(L, i++);
      lua_pushstring(L, *argv);
      lua_settable(L, -3);
    }
    lua_setfield(L, LUA_GLOBALSINDEX, "arg");


    // When we do this, skang will process all the arguments passed to GuiLuaDo().
    // This likely includes a module load, which likely opens a window.
    lua_getglobal(L, "require");
    lua_pushstring(L, SKANG);
    lua_call(L, 1, 1);
    lua_setfield(L, LUA_GLOBALSINDEX, SKANG);


    // Run the main loop via a Lua call.
    // This does nothing if no module opened a window.
    if (0 != luaL_dostring(L, "skang.loopWindow()"))
      PEm("Error running - skang.loopWindow()");
    lua_pop(L, closeWindow(L));
    lua_close(L);
  }
  else
    fprintf(stderr, "Failed to start Lua!\n");
}
