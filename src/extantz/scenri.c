#include "extantz.h"
#include "scenri.h"

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
  }
  else
    printf("Not picked : ");
  if (NULL == name)
    name = "";

  printf("output(%d, %d) canvas(%d, %d) object(%d, %d) scene(%f, %f) texcoord(%f, %f) node(%p) %s mesh(%p)\n",
    ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y, obj_x, obj_y, scene_x, scene_y, s, t, n, name, m);
}

Scene_Data *scenriAdd(globals *ourGlobals)
{
  Scene_Data *scene;
  Evas_Object *obj, *temp;

  scene = calloc(1, sizeof(Scene_Data));

  // TODO - I have no idea how this should work.
  // It seems the people that wrote the examples don't know either.  lol
//  scene->root_node = eo_add(EVAS_3D_NODE_CLASS, ourGlobals->evas, EVAS_3D_NODE_TYPE_NODE);
  scene->root_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_NODE);

  scene->scene = eo_add(EVAS_3D_SCENE_CLASS, ourGlobals->evas,
    evas_3d_scene_root_node_set(scene->root_node),
    evas_3d_scene_size_set(512, 512),
    evas_3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0)
  );

  // Add an image object for 3D scene rendering.
  obj = eo_add(ELM_OBJ_IMAGE_CLASS, ourGlobals->win,
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    elm_obj_image_fill_outside_set(EINA_TRUE),
    evas_obj_visibility_set(EINA_TRUE),
    temp = elm_obj_image_object_get()
  );
  elm_object_tooltip_text_set(obj, "");
  elm_object_tooltip_hide(obj);
  scene->image = obj;
  scene->camera_node = cameraAdd(ourGlobals, scene, obj);

  scene->light = eo_add(EVAS_3D_LIGHT_CLASS, ourGlobals->evas,
    evas_3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
    evas_3d_light_directional_set(EINA_TRUE)
  );

  scene->light_node = evas_3d_node_add(ourGlobals->evas, EVAS_3D_NODE_TYPE_LIGHT);
  eo_do(scene->light_node,
    evas_3d_node_light_set(scene->light),
    evas_3d_node_position_set(1000.0, 0.0, 1000.0),
    evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0)
  );

  eo_do(scene->root_node, evas_3d_node_member_add(scene->light_node));

  eo_do(temp, evas_obj_image_scene_set(scene->scene));
  // Elm can't seem to be able to tell us WHERE an image was clicked, so use raw Evas calbacks instead.
  evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, scene);
  evas_object_event_callback_add(temp, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, scene);

  elm_win_resize_object_add(ourGlobals->win, obj);

  return scene;
}

void scenriDel(globals *ourGlobals)
{
  eo_unref(ourGlobals->scene->image);
}
