#include "winFang.h"


static void _checkWindowBounds(winFang *win, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
  Evas_Object *img = win->win, *test;
  Eina_List *objs, *this;
  Evas_Coord mw, mh;
  int padding = 1, i = 0, overs[4][2];

  // Sanity check.
  if ((20 > w) || (20 > h))
    return;

  overs[0][0] = x - padding;		overs[0][1] = y - padding;
  overs[1][0] = x + padding + w;	overs[1][1] = y - padding;
  overs[2][0] = x + padding + w;	overs[2][1] = y + padding + h;
  overs[3][0] = x - padding;		overs[3][1] = y + padding + h;

  // If we are over two or more windows, stop.
  objs = evas_objects_in_rectangle_get(win->e, overs[0][0], overs[0][1], w + (padding * 2), h + (padding * 2), EINA_TRUE, EINA_FALSE);
  EINA_LIST_FOREACH(objs, this, test)
  {
    const char * name = evas_object_name_get(test);
    if (name && (strcmp(WF_LAYOUT, name) == 0))
      i++;
  }
  if (2 <= i)
  {
    return;
  }

  // Check if we are outside the parent window.
  evas_object_geometry_get(win->parent->win, NULL, NULL, &mw, &mh);
  if ((overs[0][0] < 0) || (overs[0][1] < 0))
  {
    return;
  }
  if ((overs[2][0] > mw) || (overs[2][1] > mh))
  {
    return;
  }

  // All good, do it.
  win->x = x;  win->y = y;
  win->w = w;  win->h = h;
  evas_object_geometry_set(img, x, y, w, h);
  for (i = 0; i < 4; i++)
  {
    int cx = win->x, cy = win->y;

         if (i == 1)   cx += win->w;
    else if (i == 2)  {cx += win->w;  cy += win->h;}
    else if (i == 3)   cy += win->h;
    evas_object_move(win->hand[i], cx - 15, cy - 15);
  }
}

static void cb_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Move *ev = event_info;
  winFang *win = data;
  Evas_Coord x, y, w, h, dx = 0, dy = 0, dw = 0, dh = 0, mx, my;
  Evas_Object *img = win->win;
  int i;

  if (!ev->buttons) return;

  mx = ev->cur.canvas.x - ev->prev.output.x;
  my = ev->cur.canvas.y - ev->prev.output.y;
  evas_object_geometry_get(img, &x, &y, &w, &h);

  for (i = 0; i < 4; i++)
  {
    if (obj == win->hand[i])
    {
      switch (i)
      {
        case 0 :
        {
          dw -= mx;  dh -= my;  dx += mx;  dy += my;
          break;
        }
        case 1 :
        {
          dw += mx;  dh -= my;  dy += my;
          break;
        }
        case 2 :
        {
          dw += mx;  dh += my;
          break;
        }
        case 3 :
        {
          dw -= mx;  dh += my;  dx += mx;
          break;
        }
      }
      _checkWindowBounds(win, x + dx, y + dy, w + dw, h + dh);
      return;
    }
  }
}

static void _onBgMove(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Move *ev = event_info;
  winFang *win = data;
  Evas_Object *img = win->win;
  Evas_Coord x, y, w, h;

  if (1 != ev->buttons)  return;

  // Looks like ePhysics wont cooperate about coords and other things, so plan B.

  evas_object_geometry_get(img, &x, &y, &w, &h);
  x += ev->cur.canvas.x - ev->prev.output.x;
  y += ev->cur.canvas.y - ev->prev.output.y;
  _checkWindowBounds(win, x, y, w, h);
}

static void _onBgUnclick(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Down *ev = event_info;

  if (1 == ev->button)
  {
    evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP,   _onBgUnclick);
    evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE, _onBgMove);
  }
}

static void _onBgClick(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Down *ev = event_info;

  if (1 == ev->button)
  {
    evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,   _onBgUnclick, data);
    evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE, _onBgMove, data);
  }
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

winFang *winFangAdd(winFang *parent, int x, int y, int w, int h, char *title, char *name, EPhysics_World *world)
{
  winFang *result;
  Evas_Object *obj;
  char buf[PATH_MAX];
  int i;

  result = calloc(1, sizeof(winFang));
  eina_clist_init(&result->widgets);
  eina_clist_init(&result->winFangs);

  result->parent = parent;
  result->x = x;
  result->y = y;
  result->w = w;
  result->h = h;

  if (result->parent)
  {
    eina_clist_add_head(&parent->winFangs, &result->node);

    snprintf(buf, sizeof(buf), "%s/winFang.edj", elm_app_data_dir_get());
    result->win = eo_add(ELM_OBJ_LAYOUT_CLASS, parent->win,
	evas_obj_size_set(result->w, result->h),
	evas_obj_position_set(result->x, result->y),
	evas_obj_name_set(WF_LAYOUT),
	elm_obj_layout_file_set(buf, WF_LAYOUT),
	evas_obj_visibility_set(EINA_TRUE)
      );
    result->e = evas_object_evas_get(result->win);


    // Something to catch clicks on the background, for moving the window.
    // Cov Elm is uncooperative with this sort of thing, so we need to stick in a rectangle.
    obj = eo_add(EVAS_OBJ_RECTANGLE_CLASS, result->win,
	evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
	evas_obj_name_set(WF_UNDERLAY),
        evas_obj_color_set(0, 0, 0, 0),
	evas_obj_visibility_set(EINA_TRUE)
      );
    elm_object_part_content_set(result->win, WF_UNDERLAY, obj);
    evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN, _onBgClick, result);
    eo_unref(obj);

    if (title)
      elm_object_part_text_set(result->win, WF_TITLE, title);
/*
    result->title = eo_add(ELM_OBJ_LABEL_CLASS, result->win,
	evas_obj_size_hint_align_set(EVAS_HINT_FILL, 1.0),
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, 0.0),
	evas_obj_visibility_set(EINA_TRUE)
      );
    elm_object_style_set(result->title, "slide_bounce");
    snprintf(buf, PATH_MAX, "<b>%s</b>", title);
    elm_object_text_set(result->title, buf);
    elm_box_pack_end(result->box, result->title);
    eo_unref(result->title);
*/

/*
    obj1 = eo_add(ELM_OBJ_SEPARATOR_CLASS, result->win,
	evas_obj_size_hint_align_set(EVAS_HINT_FILL, 1.0),
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, 0.0),
	elm_obj_separator_horizontal_set(EINA_TRUE),
	evas_obj_visibility_set(EINA_TRUE)
      );
    elm_box_pack_end(result->box, obj1);
    eo_unref(obj1);
*/

    result->grid = eo_add(ELM_OBJ_GRID_CLASS, result->win,
	evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
	evas_obj_name_set(WF_SWALLOW),
	// TODO - Actually, this should be minus the size of the title stuff.
	elm_obj_grid_size_set(result->w, result->h),
	evas_obj_visibility_set(EINA_TRUE)
      );
    elm_object_part_content_set(result->win, WF_SWALLOW, result->grid);

    elm_layout_sizing_eval(result->win);

    // Create corner handles.
    snprintf(buf, sizeof(buf), "%s/pt.png", elm_app_data_dir_get());
    for (i = 0; i < 4; i++)
    {
      int cx = result->x, cy = result->y;

           if (i == 1)   cx += result->w;
      else if (i == 2)  {cx += result->w;  cy += result->h;}
      else if (i == 3)   cy += result->h;
      result->hand[i] = eo_add(EVAS_OBJ_IMAGE_CLASS, result->e,
	evas_obj_image_filled_set(EINA_TRUE),
	evas_obj_image_file_set(buf, NULL),
	evas_obj_size_set(31, 31),
	evas_obj_position_set(cx - 15, cy - 15),
	evas_obj_visibility_set(EINA_TRUE)
      );
      evas_object_event_callback_add(result->hand[i], EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move, result);
      eo_unref(result->hand[i]);
    }
  }
  else
  {
    result->win = elm_win_add(NULL, name, ELM_WIN_BASIC);
    evas_object_move(result->win, result->x, result->y);
    evas_object_smart_callback_add(result->win, "delete,request", _on_done, NULL);
    elm_win_title_set(result->win, title);

    snprintf(buf, sizeof(buf), "%s/sky_04.jpg", elm_app_data_dir_get());
    result->bg = eo_add(ELM_OBJ_IMAGE_CLASS, result->win,
      evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
      elm_obj_image_fill_outside_set(EINA_TRUE),
      elm_obj_image_file_set(buf, NULL),
      evas_obj_color_set(50, 0, 100, 100),
      evas_obj_visibility_set(EINA_TRUE)
    );
    elm_win_resize_object_add(result->win, result->bg);
    evas_object_resize(result->win, result->w, result->h);
  }

  if (result->parent)
  {
#if 0
    // EPysics enable the window.
    if (world)
    {
      result->body = ephysics_body_box_add(world);
      ephysics_body_evas_object_set(result->body, obj, EINA_TRUE);
      ephysics_body_mass_set(result->body, 0);
//      ephysics_body_restitution_set(result->body, 0.7);
//      ephysics_body_friction_set(result->body, 0);
//      ephysics_body_linear_velocity_set(result->body, 0, 0, 0);
//      ephysics_body_angular_velocity_set(result->body, 0, 0, 0);
      ephysics_body_sleeping_threshold_set(result->body, 0.1, 0.1);
    }
#endif

  }

  evas_object_show(result->win);

  return result;
}

void winFangDel(winFang *win)
{
  winFang *wf;
  Widget  *wid;

  if (!win)  return;

  if (win->bg)    eo_unref(win->bg);
  if (win->grid)  eo_unref(win->grid);
  EINA_CLIST_FOR_EACH_ENTRY(wf, &win->winFangs, winFang, node)
  {
    winFangDel(wf);
  }

  // Elm will delete our widgets, but if we are using eo, we need to unref them.
  EINA_CLIST_FOR_EACH_ENTRY(wid, &win->widgets, Widget, node)
  {
    if (wid->on_del)  wid->on_del(wid, wid->obj, NULL);
    eo_unref(wid->obj);
  }
  if (win->on_del)  win->on_del(win, win->win, NULL);
  if (win->parent)  eo_unref(win->win);
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
      evas_object_name_set(result->obj, title);
    }
  }

  return result;
}


/*  CALLBACK types

void edje_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data)
    typedef void(* Edje_Signal_Cb)(void *data, Evas_Object *obj, const char *emission, const char *source)
    called for signals sent from edje that match emission and source

void evas_object_event_callback_add(Evas_Object *obj, Evas_Callback_Type type, Evas_Object_Event_Cb func, const void *data)
    typedef void(* 	Evas_Object_Event_Cb )(void *data, Evas *e, Evas_Object *obj, void *event_info)
    no propogation

void evas_object_smart_callback_add(Evas_Object *obj, const char *event, Evas_Smart_Cb func, const void *data)
    typedef void(* 	Evas_Smart_Cb )(void *data, Evas_Object *obj, void *event_info)
    smart events on smart objects

void elm_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data)
    typedef void(* Edje_Signal_Cb)(void *data, Evas_Object *obj, const char *emission, const char *source)
    called for signals sent from obj that match emission and source

void elm_object_event_callback_add(Evas_Object *, Elm_Event_Cb, void *)
  typedef Eina_Bool(* Elm_Event_Cb)(void *data, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info)
  called for all input events
  Key up / down / mouse wheel events on a Elm widget
  propogate up through parents
    any focusable widget with this callback can deal with it
    and tell Elm to stop propagating the event.
  Seems to almost NEVER actually be called.  Pfft.

BUUUTT....

There's also specific callbacks that don't follow the above.

static void _resize_gl(Evas_Object *obj)
static void _draw_gl(Evas_Object *obj)

    elm_glview_resize_func_set(gld->elmGl, _resize_gl);
    elm_glview_render_func_set(gld->elmGl, (Elm_GLView_Func_Cb) _draw_gl);

And others no doubt.

ALSOOOO....

Ecore events.  lol


BTW.....

Elm has C&P / DND, but it's very limited.  So far only does Unix X text (plain, markup, html), images, and vcards.
Might as well imlpement it myself.
On the other hand, image is all I really need to fake it.
elm_cnp.h seems to be the only docs, not actually linked to the rest of Elm docs.

*/
