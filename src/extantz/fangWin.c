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

Evas_Object *fang_win_add(globals *ourGlobals)
{
  Evas_Object *win, *bg;

  // In theory this should create an EWS window, in practice, I'm not seeing any difference.
  // Guess I'll have to implement my own internal window manager.  I don't think a basic one will be that hard.  Famous last words.
//  elm_config_engine_set("ews");
  win = elm_win_add(ourGlobals->win, "inlined", ELM_WIN_INLINED_IMAGE);
  // On mouse down we try to shift focus to the backing image, this seems to be the correct thing to force focus onto it's widgets.
  // According to the Elm inlined image window example, this is what's needed to.
  evas_object_event_callback_add(elm_win_inlined_image_object_get(win), EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_elm, NULL);
  elm_win_alpha_set(win, EINA_TRUE);

  // Apparently transparent is not good enough for ELM backgrounds, so make it a rectangle.
  // Apparently coz ELM prefers stuff to have edjes.  A bit over the top if all I want is a transparent rectangle.
  bg = evas_object_rectangle_add(evas_object_evas_get(win));
  evas_object_color_set(bg, 50, 0, 100, 100);
  evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(win, bg);
  evas_object_show(bg);

  return win;
}

void fang_win_complete(globals *ourGlobals, Evas_Object *win, int x, int y, int w, int h)
{
  // image object for win is unlinked to its pos/size - so manual control
  // this allows also for using map and other things with it.
  evas_object_move(elm_win_inlined_image_object_get(win), x, y);
  // Odd, it needs to be resized twice.  WTF?
  evas_object_resize(win, w, h);
  evas_object_resize(elm_win_inlined_image_object_get(win), w, h);
  evas_object_show(win);
  create_handles(elm_win_inlined_image_object_get(win));
}

void overlay_add(globals *ourGlobals)
{
  GLData *gld = &ourGlobals->gld;
  Evas_Object *bg;

  // There are many reasons for this window.
  // The first is to cover the GL and provide something to click on to change focus.
  // The second is to provide something to click on for all the GL type clicking stuff that needs to be done.  In other words, no click through, we catch the clicks here.
  //   So we can probably avoid the following issue -
  //     How to do click through?  evas_object_pass_events_set(rectangle, EINA_TRUE), and maybe need to do that to the underlaying window to?
  //     Though if the rectangle is entirely transparent, or even hidden, events might pass through anyway.
  //   Gotta have click through on the parts where there's no other window.
  // The third is to have the other windows live here.
  //   This idea doesn't work, as it breaks the damn focus again.
  //   Don't think it's needed anyway.
  // While on the subject of layers, need a HUD layer of some sort, but Irrlicht might support that itself.

  gld->winwin = elm_win_add(ourGlobals->win, "inlined", ELM_WIN_INLINED_IMAGE);
  // On mouse down we try to shift focus to the backing image, this seems to be the correct thing to force focus onto it's widgets.
  // According to the Elm inlined image window example, this is what's needed to.
  evas_object_event_callback_add(elm_win_inlined_image_object_get(gld->winwin), EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_elm, NULL);
  // In this code, we are making our own camera, so grab it's input when we are focused.
  cameraAdd(ourGlobals, gld->winwin);

  elm_win_alpha_set(gld->winwin, EINA_TRUE);
  // Apparently transparent is not good enough for ELM backgrounds, so make it a rectangle.
  // Apparently coz ELM prefers stuff to have edjes.  A bit over the top if all I want is a transparent rectangle.
  bg = evas_object_rectangle_add(evas_object_evas_get(gld->winwin));
  evas_object_color_set(bg, 0, 0, 0, 0);
  evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(gld->winwin, bg);
  evas_object_show(bg);

  // image object for win is unlinked to its pos/size - so manual control
  // this allows also for using map and other things with it.
  evas_object_move(elm_win_inlined_image_object_get(gld->winwin), 0, 0);
  // Odd, it needs to be resized twice.  WTF?
  evas_object_resize(gld->winwin, ourGlobals->win_w, ourGlobals->win_h);
  evas_object_resize(elm_win_inlined_image_object_get(gld->winwin), ourGlobals->win_w, ourGlobals->win_h);
  evas_object_show(gld->winwin);
}
