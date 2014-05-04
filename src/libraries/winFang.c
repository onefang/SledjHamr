#include "winFang.h"


// Elm inlined image windows needs this to change focus on mouse click.
// Evas style event callback.
static void _cb_mouse_down_elm(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Down *ev = event_info;

    if (1 == ev->button)
	elm_object_focus_set(obj, EINA_TRUE);
}

static void cb_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *orig = data;
   Evas_Coord x, y;
   Evas_Map *p;
   int i, w, h;

   if (!ev->buttons) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(obj,
                    x + (ev->cur.canvas.x - ev->prev.output.x),
                    y + (ev->cur.canvas.y - ev->prev.output.y));
   evas_object_image_size_get(orig, &w, &h);
   p = evas_map_new(4);
   evas_object_map_enable_set(orig, EINA_TRUE);
//   evas_object_raise(orig);
   for (i = 0; i < 4; i++)
     {
        Evas_Object *hand;
        char key[32];

        snprintf(key, sizeof(key), "h-%i\n", i);
        hand = evas_object_data_get(orig, key);
        evas_object_raise(hand);
        evas_object_geometry_get(hand, &x, &y, NULL, NULL);
        x += 15;
        y += 15;
        evas_map_point_coord_set(p, i, x, y, 0);
        if (i == 0) evas_map_point_image_uv_set(p, i, 0, 0);
        else if (i == 1) evas_map_point_image_uv_set(p, i, w, 0);
        else if (i == 2) evas_map_point_image_uv_set(p, i, w, h);
        else if (i == 3) evas_map_point_image_uv_set(p, i, 0, h);
     }
   evas_object_map_set(orig, p);
   evas_map_free(p);
}

static void _on_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  elm_exit();
}

void winFangHide(winFang *win)
{
  int i;

  evas_object_hide(win->win);
  for (i = 0; i < 4; i++)
    evas_object_hide(win->hand[i]);
}

void winFangShow(winFang *win)
{
  int i;

  evas_object_show(win->win);
  for (i = 0; i < 4; i++)
    evas_object_show(win->hand[i]);
}

winFang *winFangAdd(Evas_Object *parent, int x, int y, int w, int h, char *title, char *name)
{
  winFang *result;
  Evas_Object *obj, *obj2, *bg;
  char buf[PATH_MAX];
  int i;

  result = calloc(1, sizeof(winFang));
  eina_clist_init(&result->widgets);

  if (parent)	result->internal = EINA_TRUE;

  result->x = x;
  result->y = y;
  result->w = w;
  result->h = h;

  // In theory this should create an EWS window, in practice, I'm not seeing any difference.
  // Guess I'll have to implement my own internal window manager.  I don't think a basic one will be that hard.  Famous last words.
//  elm_config_engine_set("ews");
  if (result->internal)
  {
    result->win = elm_win_add(parent, name, ELM_WIN_INLINED_IMAGE);
    obj  = elm_win_inlined_image_object_get(result->win);
    // On mouse down we try to shift focus to the backing image, this seems to be the correct thing to force focus onto it's widgets.
    // According to the Elm inlined image window example, this is what's needed to.
    evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_elm, NULL);
    elm_win_alpha_set(result->win, EINA_TRUE);

    // image object for win is unlinked to its pos/size - so manual control
    // this allows also for using map and other things with it.
    evas_object_move(obj, result->x, result->y);
    // Odd, it needs to be resized twice.  WTF?
    evas_object_resize(obj, result->w, result->h);

    obj2 = evas_object_evas_get(obj);
    // Create corner handles.
    snprintf(buf, sizeof(buf), "%s/pt.png", elm_app_data_dir_get());
    for (i = 0; i < 4; i++)
    {
      char key[32];
      int cx = result->x, cy = result->y;

           if (i == 1)   cx += result->w;
      else if (i == 2)  {cx += result->w;  cy += result->h;}
      else if (i == 3)   cy += result->h;
      snprintf(key, sizeof(key), "h-%i\n", i);
#if 1
      result->hand[i] = evas_object_image_filled_add(obj2);
      evas_object_image_file_set(result->hand[i], buf, NULL);
      evas_object_resize(result->hand[i], 31, 31);
      evas_object_move(result->hand[i], cx - 15, cy - 15);
      evas_object_data_set(obj, key, result->hand[i]);
      evas_object_show(result->hand[i]);
      evas_object_event_callback_add(result->hand[i], EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move, obj);
#else
// TODO - No idea why, but using this version makes the window vanish when you click on a handle.
      result->hand[i] = eo_add(EVAS_OBJ_IMAGE_CLASS, obj2,
	evas_obj_image_filled_set(EINA_TRUE),
	evas_obj_image_file_set(buf, NULL),
	evas_obj_size_set(31, 31),
	evas_obj_position_set(cx - 15, cy - 15),
	eo_key_data_set(key, result->hand[i], NULL),
	evas_obj_visibility_set(EINA_TRUE)
      );
      evas_object_event_callback_add(result->hand[i], EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move, obj);
      eo_unref(result->hand[i]);
#endif
    }
  }
  else
  {
    result->win = elm_win_add(parent, name, ELM_WIN_BASIC);
    evas_object_move(result->win, result->x, result->y);
    evas_object_smart_callback_add(result->win, "delete,request", _on_done, NULL);
  }

  elm_win_title_set(result->win, title);
  // Apparently transparent is not good enough for ELM backgrounds, so make it an Evas rectangle.
  // Apparently coz ELM prefers stuff to have edjes.  A bit over the top if all I want is a transparent rectangle.
  bg = eo_add(EVAS_OBJ_RECTANGLE_CLASS, evas_object_evas_get(result->win),
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_color_set(50, 0, 100, 100),
    evas_obj_visibility_set(EINA_TRUE)
  );
  elm_win_resize_object_add(result->win, bg);
  eo_unref(bg);

  evas_object_resize(result->win, result->w, result->h);
  evas_object_show(result->win);

  return result;
}

void winFangDel(winFang *win)
{
  Widget *wid;

  if (!win)  return;

  // Elm will delete our widgets, but if we are using eo, we need to unref them.
  EINA_CLIST_FOR_EACH_ENTRY(wid, &win->widgets, Widget, node)
  {
    if (wid->on_del)  wid->on_del(wid, wid->obj, NULL);
    eo_unref(wid->obj);
  }
  if (win->on_del)  win->on_del(win, win->win, NULL);
  evas_object_del(win->win);
}

Widget *widgetAdd(winFang *win, const Eo_Class *klass, Evas_Object *parent, char *title)
{
  Widget *result;

  result = calloc(1, sizeof(Widget));
  strcpy(result->magic, "Widget");
  eina_clist_add_head(&win->widgets, &result->node);

  if (parent)
  {
    result->obj = eo_add(klass, parent,
      evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
      evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
      evas_obj_visibility_set(EINA_TRUE)
    );
    if (title)
    {
      result->label = strdup(title);
      elm_object_text_set(result->obj, result->label);
    }
  }

  return result;
}
