#include "extantz.h"


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

static void create_handles(Evas_Object *obj)
{
   int i;
   Evas_Coord x, y, w, h;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   for (i = 0; i < 4; i++)
     {
        Evas_Object *hand;
        char buf[PATH_MAX];
        char key[32];

        hand = evas_object_image_filled_add(evas_object_evas_get(obj));
        evas_object_resize(hand, 31, 31);
        snprintf(buf, sizeof(buf), "%s/pt.png", elm_app_data_dir_get());
        evas_object_image_file_set(hand, buf, NULL);
        if (i == 0)      evas_object_move(hand, x     - 15, y     - 15);
        else if (i == 1) evas_object_move(hand, x + w - 15, y     - 15);
        else if (i == 2) evas_object_move(hand, x + w - 15, y + h - 15);
        else if (i == 3) evas_object_move(hand, x     - 15, y + h - 15);
        evas_object_event_callback_add(hand, EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move, obj);
        evas_object_show(hand);
        snprintf(key, sizeof(key), "h-%i\n", i);
        evas_object_data_set(obj, key, hand);
     }
}

winFang *winFangAdd(globals *ourGlobals)
{
  winFang *result;
  Evas_Object *bg;

  result = calloc(1, sizeof(winFang));
  eina_clist_add_head(&ourGlobals->winFangs, &result->node);
  eina_clist_init(&result->widgets);

  // In theory this should create an EWS window, in practice, I'm not seeing any difference.
  // Guess I'll have to implement my own internal window manager.  I don't think a basic one will be that hard.  Famous last words.
//  elm_config_engine_set("ews");
  result->win = elm_win_add(ourGlobals->win, "inlined", ELM_WIN_INLINED_IMAGE);
  // On mouse down we try to shift focus to the backing image, this seems to be the correct thing to force focus onto it's widgets.
  // According to the Elm inlined image window example, this is what's needed to.
  evas_object_event_callback_add(elm_win_inlined_image_object_get(result->win), EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_elm, NULL);
  elm_win_alpha_set(result->win, EINA_TRUE);

  // Apparently transparent is not good enough for ELM backgrounds, so make it a rectangle.
  // Apparently coz ELM prefers stuff to have edjes.  A bit over the top if all I want is a transparent rectangle.
  bg = evas_object_rectangle_add(evas_object_evas_get(result->win));
  evas_object_color_set(bg, 50, 0, 100, 100);
  evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(result->win, bg);
  evas_object_show(bg);

  return result;
}

void winFangComplete(globals *ourGlobals, winFang *win, int x, int y, int w, int h)
{
  // image object for win is unlinked to its pos/size - so manual control
  // this allows also for using map and other things with it.
  evas_object_move(elm_win_inlined_image_object_get(win->win), x, y);
  // Odd, it needs to be resized twice.  WTF?
  evas_object_resize(win->win, w, h);
  evas_object_resize(elm_win_inlined_image_object_get(win->win), w, h);
  evas_object_show(win->win);
  create_handles(elm_win_inlined_image_object_get(win->win));
}

void winFangDel(globals *ourGlobals, winFang *win)
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
    if (title)  elm_object_text_set(result->obj, title);
  }

  return result;
}
