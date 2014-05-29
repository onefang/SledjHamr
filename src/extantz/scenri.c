#include "extantz.h"


Eina_Bool animateScene(globals *ourGlobals)
{
  ExtantzStuffs *e;

  EINA_CLIST_FOR_EACH_ENTRY(e, &ourGlobals->stuffs, ExtantzStuffs, node)
  {
    if (e->animateStuffs)  e->animateStuffs(e);
  }

  return EINA_TRUE;
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
  {
    elm_object_tooltip_text_set(scene->image, "");
    elm_object_tooltip_hide(scene->image);
  }
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
  char *name = NULL;

  // Set the focus onto us.
  elm_object_focus_set(o, EINA_TRUE);

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
    if (scene->clickCb)  scene->clickCb(n, e, o, einfo);
  }
  else
    printf("Not picked : ");
  if (NULL == name)
    name = "";

  printf("output(%d, %d) canvas(%d, %d) object(%d, %d) scene(%f, %f) texcoord(%f, %f) node(%p) %s mesh(%p)\n",
    ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y, obj_x, obj_y, scene_x, scene_y, s, t, n, name, m);

}

Scene_Data *scenriAdd(Evas_Object *win)
{
  Scene_Data *scene;
  Evas_Object *evas, *temp;
  int w, h;

  evas = evas_object_evas_get(win);
  eo_do(win, evas_obj_size_get(&w, &h));
  scene = calloc(1, sizeof(Scene_Data));

  scene->root_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas, evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

  scene->scene = eo_add(EVAS_3D_SCENE_CLASS, evas,
    evas_3d_scene_root_node_set(scene->root_node),
    evas_3d_scene_size_set(w, h),
    evas_3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0)
  );

  // Add an image object for 3D scene rendering.
  // Any colour or texture applied to this window gets applied to the scene, including transparency.
  // Interestingly enough, the position and size of the render seems to NOT depend on the position and size of this image?
  // Note that we can't reuse the windows background image, Evas_3D needs both images.
  scene->image = eo_add(ELM_OBJ_IMAGE_CLASS, win,
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    elm_obj_image_fill_outside_set(EINA_TRUE),
    evas_obj_visibility_set(EINA_TRUE),
    temp = elm_obj_image_object_get()
  );
  elm_object_tooltip_text_set(scene->image, "");
  elm_object_tooltip_hide(scene->image);
  scene->camera_node = cameraAdd(evas, scene, scene->image);

  scene->light = eo_add(EVAS_3D_LIGHT_CLASS, evas,
    evas_3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_directional_set(EINA_TRUE)
  );
  scene->light_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas, evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT),
    evas_3d_node_light_set(scene->light),
    evas_3d_node_position_set(1000.0, 0.0, 1000.0),
    evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0)
  );
  eo_do(scene->root_node, evas_3d_node_member_add(scene->light_node));

  eo_do(temp, evas_obj_image_scene_set(scene->scene));

  // Elm can't seem to be able to tell us WHERE an image was clicked, so use raw Evas callbacks instead.
  evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, scene);
  evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, scene);

  elm_win_resize_object_add(win, scene->image);

  return scene;
}

void scenriDel(Scene_Data *scene)
{
  // TODO - I should probably free up all this Evas_3D stuff.  Oddly Eo doesn't bitch about it, only valgrind.
  //        Eo bitches if they are unref'd here.
  //        So either Eo or valgrind bitches, depending on what I do.  I'll leave them commented out, let valgrind bitch, and blame Evas_3D.
//  eo_unref(scene->light_node);
//  eo_unref(scene->light);

  // TODO - Should have a separate cameraDel() for these.
  free(scene->move);
//  eo_unref(scene->camera_node);

  eo_unref(scene->image);
  eo_unref(scene->scene);
//  eo_unref(scene->root_node);
  free(scene);
}


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


static void _sphere_init(int precision)
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


void stuffsSetup(ExtantzStuffs *stuffs, globals *ourGlobals, Scene_Data *scene, int fake)
{
  char buf[PATH_MAX];
  Material *m;
  Evas_3D_Texture  *t, *t1, *ti;
  Evas_3D_Material *mi, *mj;
  Evas_3D_Mesh     *me;

// TODO - These examples just don't fit neatly into anything I can whip up quickly as a data format.
//        So just fake it for now, and expand the data format later.

  // Textures
  if (1 == fake)
  {
    t = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
      evas_3d_texture_data_set(EVAS_3D_COLOR_FORMAT_RGBA, EVAS_3D_PIXEL_FORMAT_8888, 4, 4, &pixels0[0])
      );
    eina_array_push(stuffs->textures, t);

    t1 = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
      evas_3d_texture_data_set(EVAS_3D_COLOR_FORMAT_RGBA, EVAS_3D_PIXEL_FORMAT_8888, 4, 4, &pixels1[0])
      );
    eina_array_push(stuffs->textures, t1);
  }

  EINA_INARRAY_FOREACH(stuffs->stuffs.details.mesh->materials, m)
  {
    snprintf(buf, sizeof(buf), "%s/%s", prefix_data_get(), m->texture);
    ti = eo_add(EVAS_3D_TEXTURE_CLASS, ourGlobals->evas,
      evas_3d_texture_file_set(buf, NULL),
      evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_LINEAR, EVAS_3D_TEXTURE_FILTER_LINEAR),		// Only for sphere originally.
      evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),	// Only for sonic  originally.
      evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT)
    );
    eina_array_push(stuffs->textures, ti);
  }

  // Materials.
  if (1 == fake)
  {
    eina_accessor_data_get(stuffs->aTexture, 0, (void **) &t);
    mi = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

      evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
      evas_3d_material_shininess_set(100.0),
      evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, t)
    );
    eina_array_push(stuffs->materials, mi);

    eina_accessor_data_get(stuffs->aTexture, 1, (void **) &t1);
    eina_accessor_data_get(stuffs->aTexture, 2, (void **) &ti);
    mj = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),

      evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
      evas_3d_material_shininess_set(100.0),

      evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, t1),
      evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL, ti)
    );
    eina_array_push(stuffs->materials, mj);
  }
  else
  {
    eina_accessor_data_get(stuffs->aTexture, 0, (void **) &t);
    mi = eo_add(EVAS_3D_MATERIAL_CLASS, ourGlobals->evas,
      evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, t),

      evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
      evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),		// Not for sphere originally.

      evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
      evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
      evas_3d_material_shininess_set(50.0)
    );
    eina_array_push(stuffs->materials, mi);
  }

  // Meshes
  // TODO - Write real generic cube and sphere stuff later.
  if (MT_CUBE == stuffs->stuffs.details.mesh->type)
  {
    eina_accessor_data_get(stuffs->aMaterial, 0, (void **) &mi);
    eina_accessor_data_get(stuffs->aMaterial, 1, (void **) &mj);
    me = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas,
      evas_3d_mesh_vertex_count_set(24),
      evas_3d_mesh_frame_add(0),

      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION, 12 * sizeof(float), &cube_vertices[ 0]),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,   12 * sizeof(float), &cube_vertices[ 3]),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,    12 * sizeof(float), &cube_vertices[ 6]),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD, 12 * sizeof(float), &cube_vertices[10]),

      evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, 36, &cube_indices[0]),
      evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),

      evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP),

      evas_3d_mesh_frame_material_set(0, mi),

      evas_3d_mesh_frame_add(20),
      evas_3d_mesh_frame_material_set(20, mj)
    );
    eina_array_push(stuffs->mesh, me);
  }
  else if (MT_SPHERE == stuffs->stuffs.details.mesh->type)
  {
    _sphere_init(100);

    eina_accessor_data_get(stuffs->aMaterial, 0, (void **) &mi);
    me = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas,
      evas_3d_mesh_vertex_count_set(vertex_count),
      evas_3d_mesh_frame_add(0),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION, sizeof(vertex), &sphere_vertices[0].position),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,   sizeof(vertex), &sphere_vertices[0].normal),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TANGENT,  sizeof(vertex), &sphere_vertices[0].tangent),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,    sizeof(vertex), &sphere_vertices[0].color),
      evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD, sizeof(vertex), &sphere_vertices[0].texcoord),

      evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, index_count, &sphere_indices[0]),
      evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES),
      evas_3d_mesh_frame_material_set(0, mi),

      evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE)
    );
    eina_array_push(stuffs->mesh, me);
  }
  else
  {
    eina_accessor_data_get(stuffs->aMaterial, 0, (void **) &mi);
    snprintf(buf, sizeof(buf), "%s/%s", prefix_data_get(), stuffs->stuffs.details.mesh->fileName);
    me = eo_add(EVAS_3D_MESH_CLASS, ourGlobals->evas,
      evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, buf, NULL),
      evas_3d_mesh_frame_material_set(0, mi),
      evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG)
    );
    eina_array_push(stuffs->mesh, me);
  }

  eina_accessor_data_get(stuffs->aMesh, 0, (void **) &me);
  stuffs->mesh_node = eo_add_custom(EVAS_3D_NODE_CLASS, ourGlobals->evas, evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH),
    eo_key_data_set("Name", stuffs->stuffs.name, NULL),
    evas_3d_node_position_set(stuffs->stuffs.details.mesh->pos.x, stuffs->stuffs.details.mesh->pos.y, stuffs->stuffs.details.mesh->pos.z),
    evas_3d_node_orientation_set(stuffs->stuffs.details.mesh->rot.x, stuffs->stuffs.details.mesh->rot.y, stuffs->stuffs.details.mesh->rot.z, stuffs->stuffs.details.mesh->rot.w),
    evas_3d_node_mesh_add(me)
    );

  eo_do(scene->root_node, evas_3d_node_member_add(stuffs->mesh_node));
  eina_clist_add_head(&(ourGlobals->stuffs), &(stuffs->node));
}

ExtantzStuffs *addStuffs(char *uuid, char *name, char *description, char *owner,
  char *file, MeshType type, float px, float py, float pz, float rx, float ry, float rz, float rw)
{
  ExtantzStuffs *result = calloc(1, sizeof(ExtantzStuffs));

  // TODO - using Eina arrays of any sort seems a bit heavy, might be better to just count and realloc?
  result->materials = eina_array_new(3);
  result->mesh      = eina_array_new(3);
  result->textures  = eina_array_new(3);
  result->aMaterial = eina_array_accessor_new(result->materials);
  result->aMesh     = eina_array_accessor_new(result->mesh);
  result->aTexture  = eina_array_accessor_new(result->textures);
  result->stuffs.details.mesh = calloc(1, sizeof(Mesh));
  result->stuffs.details.mesh->materials = eina_inarray_new(sizeof(Material), 1);
  result->stuffs.details.mesh->parts     = eina_inarray_new(sizeof(Mesh), 1);

  strcpy(result->stuffs.UUID, uuid);
  strcpy(result->stuffs.name, name);
  strcpy(result->stuffs.description, description);
  strcpy(result->stuffs.owner, owner);

  strcpy(result->stuffs.details.mesh->fileName, file);
  result->stuffs.details.mesh->type = type;
  result->stuffs.details.mesh->pos.x = px;  result->stuffs.details.mesh->pos.y = py;  result->stuffs.details.mesh->pos.z = pz;
  result->stuffs.details.mesh->rot.x = rx;  result->stuffs.details.mesh->rot.y = ry;  result->stuffs.details.mesh->rot.z = rz;  result->stuffs.details.mesh->rot.w = rw;

  return result;
}

void addMaterial(ExtantzStuffs *e, int face, TextureType type, char *file)
{
  Material *result = calloc(1, sizeof(Material));

  // face -1 means "all of them I think".
  result->face = face;
  result->type = type;
  strcpy(result->texture, file);
  eina_inarray_push(e->stuffs.details.mesh->materials, result);
}

void Evas_3D_Demo_fini(globals *ourGlobals)
{
  free(sphere_vertices);
  free(sphere_indices);
}
