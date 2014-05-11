#include "winFang.h"


// Elm inlined image windows needs this to change focus on mouse click.
// Evas style event callback.
static void _cb_mouse_down_elm(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  winFang *win = data;
  Evas_Event_Mouse_Down *ev = event_info;

  if (1 == ev->button)
  {
    Evas_Object *objs = evas_object_top_at_pointer_get(win->e);

// TODO - This always returns the elm_win.  So how the hell do you tell what got clicked on?
printf("%s  %s\n", evas_object_type_get(objs), evas_object_name_get(objs));

    elm_object_focus_set(obj, EINA_TRUE);
  }
}

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
    if (name && (strcmp("winFang", name) == 0))
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
  // TODO - This just stretches everything, we don't really want that.
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
  char buf[PATH_MAX];
  int i;

  result = calloc(1, sizeof(winFang));
  eina_clist_init(&result->widgets);
  eina_clist_init(&result->winFangs);

  if (parent)	result->internal = EINA_TRUE;
  result->parent = parent;

  result->x = x;
  result->y = y;
  result->w = w;
  result->h = h;

  if (result->internal)
  {
    eina_clist_add_head(&parent->winFangs, &result->node);

    result->win = elm_layout_add(parent->win);
    evas_object_name_set(result->win, "winFang");
    evas_object_size_hint_weight_set(result->win, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    snprintf(buf, sizeof(buf), "%s/winFang.edj", elm_app_data_dir_get());
    elm_layout_file_set(result->win, buf, "winFang/layout");
    if (title)
      elm_object_part_text_set(result->win, TITLE, title);

    result->grid = elm_grid_add(parent->win);
    elm_grid_size_set(result->grid, result->w, result->h);
    evas_object_size_hint_weight_set(result->grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(result->grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(result->win, SWALLOW, result->grid);
    evas_object_show(result->grid);

    evas_object_resize(result->win, result->w, result->h);
    evas_object_move(result->win, result->x, result->y);
    elm_layout_sizing_eval(result->win);
    evas_object_show(result->win);

    // On mouse down we try to shift focus to the backing image, this seems to be the correct thing to force focus onto it's widgets.
    // According to the Elm inlined image window example, this is what's needed to.
    evas_object_event_callback_add(result->win, EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_elm, result);
    evas_object_event_callback_add(result->win, EVAS_CALLBACK_MOUSE_MOVE, _onBgMove, result);

    result->e = evas_object_evas_get(result->win);
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

  if (result->internal)
  {
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
  }

  evas_object_show(result->win);

  return result;
}

void winFangDel(winFang *win)
{
  winFang *wf;
  Widget  *wid;

  if (!win)  return;

  if (win->bg)  eo_unref(win->bg);
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
