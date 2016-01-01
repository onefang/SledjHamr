// scenri.c, deals with the in world scenery.

/* TODO - I can see this growing to be very big, so should start to split it up at some point.
  Base stuff -
    scenriAdd/Del()
    mouse hover showing tooltips
    mouse click on objects
    world animator that calls animators for stuffs
    addStuffs/Materials() setupStuffs()

  Scripting hooks
    Lua hooks
    LSL wrappers around Lua hooks
    Lua file loader, maybe have this in libg3d?

  Basic mesh stuff (likely just libg3d) -
    create cube, sphere, etc
    load meshes

  In world editing

*/

#include "extantz.h"


#define SKANG		"skang"
#define THINGASM	"thingasm"


static Ecore_Idle_Enterer	*idler;


static void _animateCube(ExtantzStuffs *stuffs)
{
  static float angle = 0.0f;
  static int   frame = 0;
  static int   inc   = 1;
  Eo *m;

  eina_accessor_data_get(stuffs->aMesh, 0, (void **) &m);

  angle += 0.5;
  if (angle > 360.0)		angle -= 360.0f;

  frame += inc;
  if (frame >= 20) inc = -1;
  else if (frame <= 0) inc = 1;

  eo_do(stuffs->mesh_node,
    evas_canvas3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, 1.0),
    evas_canvas3d_node_mesh_frame_set(m, frame)
  );
}

static void _animateSphere(ExtantzStuffs *stuffs)
{
  static float earthAngle = 0.0f;

  earthAngle += 0.3;
  if (earthAngle > 360.0)	earthAngle -= 360.0f;
  eo_do(stuffs->mesh_node,
    evas_canvas3d_node_orientation_angle_axis_set(earthAngle, 0.0, 1.0, 0.0)
  );
}

static void _animateSonic(ExtantzStuffs *stuffs)
{
  static int   sonicFrame = 0;
  Eo *m;

  eina_accessor_data_get(stuffs->aMesh, 0, (void **) &m);
  sonicFrame += 32;
  if (sonicFrame > 256 * 50)	sonicFrame = 0;
  eo_do(stuffs->mesh_node,
    evas_canvas3d_node_mesh_frame_set(m, sonicFrame)
  );
}

Eina_Bool animateScene(globals *ourGlobals)
{
  ExtantzStuffs *e;

  EINA_CLIST_FOR_EACH_ENTRY(e, &ourGlobals->scene->stuffs, ExtantzStuffs, node)
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
  Eo *n;
  Eo *m;
  Eina_Bool pick;
  char *name = NULL;

  evas_object_geometry_get(o, &x, &y, &w, &h);

  obj_x = ev->cur.canvas.x - x;
  obj_y = ev->cur.canvas.y - y;

  eo_do(scene->scene, evas_canvas3d_scene_size_get(&scene_w, &scene_h));

  scene_x = obj_x * scene_w / (Evas_Real)w;
  scene_y = obj_y * scene_h / (Evas_Real)h;

  eo_do(scene->scene, pick = evas_canvas3d_scene_pick(scene_x, scene_y, &n, &m, &s, &t));
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
  Eo *n;
  Eo *m;
  Eina_Bool pick;
  char *name = NULL;

  // Set the focus onto us.
  elm_object_focus_set(o, EINA_TRUE);

  evas_object_geometry_get(o, &x, &y, &w, &h);

  obj_x = ev->canvas.x - x;
  obj_y = ev->canvas.y - y;

  eo_do(scene->scene, evas_canvas3d_scene_size_get(&scene_w, &scene_h));

  scene_x = obj_x * scene_w / (Evas_Real)w;
  scene_y = obj_y * scene_h / (Evas_Real)h;

  eo_do(scene->scene, pick = evas_canvas3d_scene_pick(scene_x, scene_y, &n, &m, &s, &t));
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

static int _preallocateStuffs(lua_State *L)
{
  ExtantzStuffs *result = NULL;
  Scene_Data *scene = NULL;
  int count, i;

  pull_lua(L, 1, "%", &count);
  lua_getfield(L, LUA_REGISTRYINDEX, "sceneData");
  scene = (Scene_Data *) lua_touserdata(L, -1);

  result = calloc(count, sizeof(ExtantzStuffs));

  for (i = 0;  i < count;  i++)
  {
    // TODO - using Eina arrays of any sort seems a bit heavy, might be better to just count and realloc?
    result[i].materials = eina_array_new(3);
    result[i].mesh      = eina_array_new(3);
    result[i].textures  = eina_array_new(3);
    result[i].aMaterial = eina_array_accessor_new(result[i].materials);
    result[i].aMesh     = eina_array_accessor_new(result[i].mesh);
    result[i].aTexture  = eina_array_accessor_new(result[i].textures);
    result[i].stuffs.details.mesh = calloc(1, sizeof(Mesh));
    result[i].stuffs.details.mesh->materials = eina_inarray_new(sizeof(Material), 1);
    result[i].stuffs.details.mesh->parts     = eina_inarray_new(sizeof(Mesh), 1);
    result[i].scene = scene;
    result[i].stage = ES_PRE;
    eina_clist_add_head(&(scene->loading), &(result[i].node));
  }

  return 0;
}

static int _partFillStuffs(lua_State *L)
{
  ExtantzStuffs *result = NULL;
  Scene_Data *scene = NULL;
  char *name, *file;
  double px, py, pz, rx, ry, rz, rw;

  pull_lua(L, 1, "$ $ # # # # # # #", &name, &file, &px, &py, &pz, &rx, &ry, &rz, &rw);
  lua_getfield(L, LUA_REGISTRYINDEX, "sceneData");
  scene = (Scene_Data *) lua_touserdata(L, -1);

  EINA_CLIST_FOR_EACH_ENTRY(result, &(scene->loading), ExtantzStuffs, node)
  {
    if (ES_PRE == result->stage)
    {
      strcpy(result->stuffs.name, name);

      strcpy(result->stuffs.file, file);
      result->stuffs.details.mesh->pos.x = px;  result->stuffs.details.mesh->pos.y = py;  result->stuffs.details.mesh->pos.z = pz;
      result->stuffs.details.mesh->rot.x = rx;  result->stuffs.details.mesh->rot.y = ry;  result->stuffs.details.mesh->rot.z = rz;  result->stuffs.details.mesh->rot.w = rw;
      result->stage = ES_PART;
      break;
    }
  }

  return 0;
}

static int _finishStuffs(lua_State *L)
{
  ExtantzStuffs *result = NULL;
  Scene_Data *scene = NULL;
  char *uuid, *name, *file, *description, *owner, *mesh;
  int type, fake;

  pull_lua(L, 1, "$ $ $ $ $ $ % %", &uuid, &name, &file, &description, &owner, &mesh, &type, &fake);
  lua_getfield(L, LUA_REGISTRYINDEX, "sceneData");
  scene = (Scene_Data *) lua_touserdata(L, -1);

  EINA_CLIST_FOR_EACH_ENTRY(result, &(scene->loading), ExtantzStuffs, node)
  {
    if (strcmp(file, result->stuffs.name) == 0)
    {
      strcpy(result->stuffs.UUID, uuid);
      strcpy(result->stuffs.name, name);
      strcpy(result->stuffs.description, description);
      strcpy(result->stuffs.owner, owner);
      strcpy(result->stuffs.details.mesh->fileName, mesh);

      result->stuffs.details.mesh->type = type;
      result->fake = fake;
      result->stage = ES_LOADED;

      lua_pushlightuserdata(L, (void *) result);

      PI("LOADED %s", name);
      return 1;
    }
  }

  return 0;
}

static int _addStuffsL(lua_State *L)
{
  ExtantzStuffs *result = NULL;
  char *uuid, *name, *description, *owner, *file;
  int type;
  double px, py, pz, rx, ry, rz, rw;

  pull_lua(L, 1, "$ $ $ $ $ % # # # # # # #", &uuid, &name, &description, &owner, &file, &type, &px, &py, &pz, &rx, &ry, &rz, &rw);
  result = addStuffs(uuid, name, description, owner, file, type, px, py, pz, rx, ry, rz, rw);
  if (result)
  {
    lua_getfield(L, LUA_REGISTRYINDEX, "sceneData");
    result->scene = (Scene_Data *) lua_touserdata(L, -1);

    lua_pushlightuserdata(L, (void *) result);
    return 1;
  }

  return 0;
}

static int _addMaterialL(lua_State *L)
{
  ExtantzStuffs *e = NULL;
  int face, type;
  char *file;

  pull_lua(L, 1, "* % % $", &e, &face, &type, &file);
  if (e)
    addMaterial(e, face, type, file);

  return 0;
}

static int _stuffsSetupL(lua_State *L)
{
  ExtantzStuffs *e = NULL;
  int fake;

  pull_lua(L, 1, "* %", &e, &fake);
  if (e)
    stuffsSetup(e, e->scene, fake);

  return 0;
}

static Eina_Bool _stuffsLoader(void *data)
{
  ExtantzStuffs *result = NULL, *e1;
  Scene_Data *scene = data;

  // TODO - If there's lots of them, only do some.
  EINA_CLIST_FOR_EACH_ENTRY_SAFE(result, e1, &(scene->loading), ExtantzStuffs, node)
  {
    if (ES_PART == result->stage)
    {
      int scenriLua;

      lua_getglobal(scene->L, "package");
      lua_getfield(scene->L, lua_gettop(scene->L), "loaded");
      lua_remove(scene->L, -2);				// Removes "package"
      lua_getfield(scene->L, lua_gettop(scene->L), "scenriLua");
      lua_remove(scene->L, -2);				// Removes "loaded"
      scenriLua = lua_gettop(scene->L);

      push_lua(scene->L, "@ ( $ $ )", scenriLua, "finishLoadStuffs", scene->sim, result->stuffs.file, 0);
    }
    if (ES_LOADED == result->stage)
    {
      eina_clist_remove(&(result->node));
      stuffsSetup(result, result->scene, result->fake);
      result->stage = ES_RENDERED;
    }
  }

  return ECORE_CALLBACK_RENEW;
}

Scene_Data *scenriAdd(Evas_Object *win)
{
  Scene_Data *scene;
  Evas_Object *evas;
#if USE_ELM_IMG
  Evas_Object *temp;
#endif
  int w, h;

  evas = evas_object_evas_get(win);
  eo_do(win, efl_gfx_size_get(&w, &h));
  scene = calloc(1, sizeof(Scene_Data));
  scene->evas = evas;
  eina_clist_init(&(scene->stuffs));
  eina_clist_init(&(scene->loading));

  scene->root_node = eo_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_NODE));

  scene->scene = eo_add(EVAS_CANVAS3D_SCENE_CLASS, evas,
    evas_canvas3d_scene_root_node_set(scene->root_node),
    evas_canvas3d_scene_size_set(w, h),
    evas_canvas3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0)
  );

  // Add an image object for 3D scene rendering.
  // Any colour or texture applied to this window gets applied to the scene, including transparency.
  // Interestingly enough, the position and size of the render seems to NOT depend on the position and size of this image?
  // Note that we can't reuse the windows background image, Evas_3D needs both images.
#if USE_ELM_IMG
  scene->image = eo_add(ELM_IMAGE_CLASS, win,
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    elm_obj_image_fill_outside_set(EINA_TRUE),
    efl_gfx_visible_set(EINA_TRUE),
    temp = elm_obj_image_object_get()
  );
#else
  scene->image = evas_object_image_filled_add(evas);
  evas_object_size_hint_weight_set(scene->image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
  evas_object_resize(scene->image, WIDTH, HEIGHT);
  evas_object_show(scene->image);
#endif
  elm_object_tooltip_text_set(scene->image, "");
  elm_object_tooltip_hide(scene->image);
  scene->camera_node = cameraAdd(evas, scene, scene->image);

  scene->light = eo_add(EVAS_CANVAS3D_LIGHT_CLASS, evas,
    evas_canvas3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
    evas_canvas3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
    evas_canvas3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
    evas_canvas3d_light_directional_set(EINA_TRUE)
  );
  scene->light_node = eo_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_LIGHT),
    evas_canvas3d_node_light_set(scene->light),
    evas_canvas3d_node_position_set(1000.0, 0.0, 1000.0),
    evas_canvas3d_node_look_at_set(EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0)
  );
  eo_do(scene->root_node, evas_canvas3d_node_member_add(scene->light_node));

#if USE_ELM_IMG
  eo_do(temp, evas_obj_image_scene_set(scene->scene));

  // Elm can't seem to be able to tell us WHERE an image was clicked, so use raw Evas callbacks instead.
  evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, scene);
  evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, scene);
#else
  eo_do(scene->image, evas_obj_image_scene_set(scene->scene));

  // Elm can't seem to be able to tell us WHERE an image was clicked, so use raw Evas callbacks instead.
  evas_object_event_callback_add(scene->image, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, scene);
  evas_object_event_callback_add(scene->image, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, scene);
#endif


  elm_win_resize_object_add(win, scene->image);

  scene->L = luaL_newstate();
  if (scene->L)
  {
    char buf[PATH_MAX];
    int skang, scenriLua;

    luaL_openlibs(scene->L);
    lua_getglobal(scene->L, "require");
    lua_pushstring(scene->L, "scenriLua");
    lua_call(scene->L, 1, 1);

    // Shove our structure into the registry.
    lua_pushlightuserdata(scene->L, scene);
    lua_setfield(scene->L, LUA_REGISTRYINDEX, "sceneData");

    // The skang module should have been loaded by now, so we can just grab it out of package.loaded[].
    lua_getglobal(scene->L, "package");
    lua_getfield(scene->L, lua_gettop(scene->L), "loaded");
    lua_remove(scene->L, -2);				// Removes "package"
    lua_getfield(scene->L, lua_gettop(scene->L), SKANG);
    lua_remove(scene->L, -2);				// Removes "loaded"
    lua_setfield(scene->L, LUA_REGISTRYINDEX, SKANG);
    lua_getfield(scene->L, LUA_REGISTRYINDEX, SKANG);	// Puts the skang table back on the stack.
    skang = lua_gettop(scene->L);

    // Same for the scenriLua module, we just loaded it.
    lua_getglobal(scene->L, "package");
    lua_getfield(scene->L, lua_gettop(scene->L), "loaded");
    lua_remove(scene->L, -2);				// Removes "package"
    lua_getfield(scene->L, lua_gettop(scene->L), "scenriLua");
    lua_remove(scene->L, -2);				// Removes "loaded"
    scenriLua = lua_gettop(scene->L);

    // Define our functions.
    push_lua(scene->L, "@ ( = $ $ & $ )", skang, THINGASM, scenriLua, "preallocateStuffs", "preallocate a bunch of stuffs.", _preallocateStuffs,
      "number", 0);
    push_lua(scene->L, "@ ( = $ $ & $ )", skang, THINGASM, scenriLua, "partFillStuffs", "Put some info into a stuffs.", _partFillStuffs,
      "string,string,number,number,number,number,number,number,number", 0);
    push_lua(scene->L, "@ ( = $ $ & $ )", skang, THINGASM, scenriLua, "finishStuffs", "Put rest of info into a stuffs.", _finishStuffs,
      "string,string,string,string,string,number,number,number,number,number,number,number,number", 0);
    push_lua(scene->L, "@ ( = $ $ & $ )", skang, THINGASM, scenriLua, "addStuffs", "Add an in world stuffs.", _addStuffsL,
      "string,string,string,string,string,number,number,number,number,number,number,number,number", 0);
    push_lua(scene->L, "@ ( = $ $ & $ )", skang, THINGASM, scenriLua, "addMaterial", "Add a material to an in world stuffs.", _addMaterialL,
      "userdata,number,number,string", 0);
    push_lua(scene->L, "@ ( = $ $ & $ )", skang, THINGASM, scenriLua, "stuffsSetup", "Render the stuffs.", _stuffsSetupL,
      "userdata,number", 0);

    // Pass the enums to scenriLua.
    sprintf(buf, "MeshType = {cube = %d, mesh = %d, sphere = %d}", MT_CUBE, MT_MESH, MT_SPHERE);
    doLuaString(scene->L, buf, "scenriLua");
    sprintf(buf, "TextureType = {face = %d, normal = %d}", TT_FACE, TT_NORMAL);
    doLuaString(scene->L, buf, "scenriLua");
  }

  idler = ecore_idler_add(_stuffsLoader, scene);

  return scene;
}

void scenriDel(Scene_Data *scene)
{
  lua_close(scene->L);

  // TODO - I should probably free up all this Evas_3D stuff.  Oddly Eo doesn't bitch about it, only valgrind.
  //        Eo bitches if they are unref'd here.
  //        So either Eo or valgrind bitches, depending on what I do.  I'll leave them commented out, let valgrind bitch, and blame Evas_3D.
//  eo_unref(scene->light_node);
//  eo_unref(scene->light);

  // TODO - Should have a separate cameraDel() for these.
  free(scene->move);
//  eo_unref(scene->camera_node);

//  eo_unref(scene->image);
//  eo_unref(scene->scene);
//  eo_unref(scene->root_node);
  free(scene);
}


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


void stuffsSetup(ExtantzStuffs *stuffs, Scene_Data *scene, int fake)
{
  char buf[PATH_MAX];
  Material *m;
  Eo  *t, *t1, *ti;
  Eo *mi, *mj;
  Eo     *me;

  PI("REZZING %s", stuffs->stuffs.name);
// TODO - These examples just don't fit neatly into anything I can whip up quickly as a data format.
//        So just fake it for now, and expand the data format later.

  // Textures
  if (1 == fake)
  {
    t = eo_add(EVAS_CANVAS3D_TEXTURE_CLASS, scene->evas,
      evas_canvas3d_texture_data_set(EVAS_COLORSPACE_ARGB8888, 4, 4, &pixels0[0])
      );
    eina_array_push(stuffs->textures, t);

    t1 = eo_add(EVAS_CANVAS3D_TEXTURE_CLASS, scene->evas,
      evas_canvas3d_texture_data_set(EVAS_COLORSPACE_ARGB8888, 4, 4, &pixels1[0])
      );
    eina_array_push(stuffs->textures, t1);
  }

  EINA_INARRAY_FOREACH(stuffs->stuffs.details.mesh->materials, m)
  {
    snprintf(buf, sizeof(buf), "%s/%s", prefix_data_get(), m->texture);
    ti = eo_add(EVAS_CANVAS3D_TEXTURE_CLASS, scene->evas,
      evas_canvas3d_texture_file_set(buf, NULL),
      evas_canvas3d_texture_filter_set(EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR, EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR),		// Only for sphere originally.
      evas_canvas3d_texture_filter_set(EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST),	// Only for sonic  originally.
      evas_canvas3d_texture_wrap_set(EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT)
    );
    eina_array_push(stuffs->textures, ti);
  }

  // Materials.
  if (1 == fake)
  {
    eina_accessor_data_get(stuffs->aTexture, 0, (void **) &t);
    mi = eo_add(EVAS_CANVAS3D_MATERIAL_CLASS, scene->evas,
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE),

      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0),
      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0),
      evas_canvas3d_material_shininess_set(100.0),
      evas_canvas3d_material_texture_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, t)
    );
    eina_array_push(stuffs->materials, mi);

    eina_accessor_data_get(stuffs->aTexture, 1, (void **) &t1);
    eina_accessor_data_get(stuffs->aTexture, 2, (void **) &ti);
    mj = eo_add(EVAS_CANVAS3D_MATERIAL_CLASS, scene->evas,
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE),

      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0),
      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0),
      evas_canvas3d_material_shininess_set(100.0),

      evas_canvas3d_material_texture_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, t1),
      evas_canvas3d_material_texture_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, ti)
    );
    eina_array_push(stuffs->materials, mj);
  }
  else
  {
    eina_accessor_data_get(stuffs->aTexture, 0, (void **) &t);
    mi = eo_add(EVAS_CANVAS3D_MATERIAL_CLASS, scene->evas,
      evas_canvas3d_material_texture_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, t),

      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE),
      evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE),		// Not for sphere originally.

      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.01, 0.01, 0.01, 1.0),
      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
      evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0),
      evas_canvas3d_material_shininess_set(50.0)
    );
    eina_array_push(stuffs->materials, mi);
  }

  // Meshes
  // TODO - Write real generic cube and sphere stuff later.
  if (MT_CUBE == stuffs->stuffs.details.mesh->type)
  {
    Eo *cube;

    eina_accessor_data_get(stuffs->aMaterial, 0, (void **) &mi);
    eina_accessor_data_get(stuffs->aMaterial, 1, (void **) &mj);

    cube = eo_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, scene->evas);
    eo_do(cube, evas_canvas3d_primitive_form_set(EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE));
    me = eo_add(EVAS_CANVAS3D_MESH_CLASS, scene->evas,
      evas_canvas3d_mesh_from_primitive_set(0, cube),

      evas_canvas3d_mesh_shade_mode_set(EVAS_CANVAS3D_SHADE_MODE_NORMAL_MAP),

      evas_canvas3d_mesh_frame_material_set(0, mi),

      evas_canvas3d_mesh_frame_add(20),
      evas_canvas3d_mesh_frame_material_set(20, mj)
    );
    eina_array_push(stuffs->mesh, me);
  }
  else if (MT_SPHERE == stuffs->stuffs.details.mesh->type)
  {
    Eo *sphere;

    eina_accessor_data_get(stuffs->aMaterial, 0, (void **) &mi);

    sphere = eo_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, scene->evas);
    eo_do(sphere, evas_canvas3d_primitive_form_set(EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE));

    me = eo_add(EVAS_CANVAS3D_MESH_CLASS, scene->evas,
         evas_canvas3d_mesh_from_primitive_set(0, sphere),
      evas_canvas3d_mesh_frame_material_set(0, mi),

      evas_canvas3d_mesh_shade_mode_set(EVAS_CANVAS3D_SHADE_MODE_DIFFUSE)
    );
    eina_array_push(stuffs->mesh, me);
  }
  else
  {
    eina_accessor_data_get(stuffs->aMaterial, 0, (void **) &mi);
    snprintf(buf, sizeof(buf), "%s/%s", prefix_data_get(), stuffs->stuffs.details.mesh->fileName);
    me = eo_add(EVAS_CANVAS3D_MESH_CLASS, scene->evas,
      efl_file_set(buf, NULL),
      evas_canvas3d_mesh_frame_material_set(0, mi),
      evas_canvas3d_mesh_shade_mode_set(EVAS_CANVAS3D_SHADE_MODE_PHONG)
    );
    eina_array_push(stuffs->mesh, me);
  }

  eina_accessor_data_get(stuffs->aMesh, 0, (void **) &me);
  stuffs->mesh_node = eo_add(EVAS_CANVAS3D_NODE_CLASS, scene->evas, evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_MESH),
    eo_key_data_set("Name", stuffs->stuffs.name),
    evas_canvas3d_node_position_set(stuffs->stuffs.details.mesh->pos.x, stuffs->stuffs.details.mesh->pos.y, stuffs->stuffs.details.mesh->pos.z),
    evas_canvas3d_node_orientation_set(stuffs->stuffs.details.mesh->rot.x, stuffs->stuffs.details.mesh->rot.y, stuffs->stuffs.details.mesh->rot.z, stuffs->stuffs.details.mesh->rot.w),
    evas_canvas3d_node_mesh_add(me)
    );

  eo_do(scene->root_node, evas_canvas3d_node_member_add(stuffs->mesh_node));
  eina_clist_add_head(&(scene->stuffs), &(stuffs->node));

  if (1 == fake)
    stuffs->animateStuffs = (aniStuffs) _animateCube;
  else if (2 == fake)
    stuffs->animateStuffs = (aniStuffs) _animateSphere;
  else if (3 == fake)
    stuffs->animateStuffs = (aniStuffs) _animateSonic;

}

ExtantzStuffs *addStuffs(char *uuid, char *name, char *description, char *owner,
  char *file, MeshType type, double px, double py, double pz, double rx, double ry, double rz, double rw)
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
  result->stage = ES_NORMAL;

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
