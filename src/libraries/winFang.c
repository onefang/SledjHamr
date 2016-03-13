#include "LumbrJack.h"
#include "winFang.h"


static void _checkWindowBounds(winFang *win, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
  Evas_Object *test;
  Eina_List *objs, *this;
  Evas_Coord mw, mh;
  int padding = 1, i = 0, overs[4][2];

  // Sanity check.
  if ((win->mw > w) || (win->mh > h))
    return;

  overs[0][0] = x - padding;		overs[0][1] = y - padding;
  overs[1][0] = x + padding + w;	overs[1][1] = y - padding;
  overs[2][0] = x + padding + w;	overs[2][1] = y + padding + h;
  overs[3][0] = x - padding;		overs[3][1] = y + padding + h;

  // If we are over other windows, stop.
  objs = evas_objects_in_rectangle_get(win->e, overs[0][0], overs[0][1], w + (padding * 2), h + (padding * 2), EINA_TRUE, EINA_FALSE);
  EINA_LIST_FOREACH(objs, this, test)
  {
    const char * name = evas_object_name_get(test);
    if (name && (strcmp(WF_LAYOUT, name) == 0))
      i++;
  }
  if (2 <= i)
  {
// TODO - Something in the last EFL update broke this check.
//    return;
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
  evas_object_geometry_set(win->layout, x, y, w, h);
  evas_object_resize(win->title, w, 15);
  elm_layout_sizing_eval(win->layout);
  for (i = 0; i < 4; i++)
  {
    int cx = win->x, cy = win->y;

         if (i == 1)   cx += win->w;
    else if (i == 2)  {cx += win->w;  cy += win->h;}
    else if (i == 3)   cy += win->h;
    evas_object_move(win->hand[i], cx - 15, cy - 15);
  }
}

static void _onHandleMove(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Move *ev = event_info;
  winFang *win = data;
  Evas_Coord x, y, w, h, dx, dy;
  int i;

  if (!ev->buttons) return;

  dx = ev->cur.canvas.x - ev->prev.output.x;
  dy = ev->cur.canvas.y - ev->prev.output.y;
  evas_object_geometry_get(win->layout, &x, &y, &w, &h);
  for (i = 0; i < 4; i++)
  {
    if (obj == win->hand[i])
    {
      switch (i)
      {
        case 0 :
        {
          w -= dx;  h -= dy;  x += dx;  y += dy;
          break;
        }
        case 1 :
        {
          w += dx;  h -= dy;  y += dy;
          break;
        }
        case 2 :
        {
          w += dx;  h += dy;
          break;
        }
        case 3 :
        {
          w -= dx;  h += dy;  x += dx;
          break;
        }
      }
      _checkWindowBounds(win, x, y, w, h);
      return;
    }
  }
}

static void _onBgMove(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Move *ev = event_info;
  winFang *win = data;
  Evas_Coord x, y, w, h;

  if (1 != ev->buttons)  return;

  // Looks like ePhysics wont cooperate about coords and other things, so plan B.

  evas_object_geometry_get(win->layout, &x, &y, &w, &h);
//printf("_onBgMove()  MOVED %f %f %f %f\n", x, y, w, h);
  _checkWindowBounds(win, x + ev->cur.canvas.x - ev->prev.output.x, y + ev->cur.canvas.y - ev->prev.output.y, w, h);
}

static void _onBgUnclick(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Down *ev = event_info;

  if (1 == ev->button)
  {
    // Stop moving the window.
    evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP,   _onBgUnclick);
    evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_MOVE, _onBgMove);
  }
}

static void _onBgClick(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  Evas_Event_Mouse_Down *ev = event_info;

  if (1 == ev->button)
  {
    // Start moving the window.
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

void widgetHide(Widget *wid)
{
  evas_object_hide(wid->obj);
}

void widgetShow(Widget *wid)
{
  evas_object_show(wid->obj);
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

  // NOTE: parent = NULL is a special case, it's our main window.  Not sure yet what to do if there's more than one.
  if (result->parent)
  {
    eina_clist_add_head(&parent->winFangs, &result->node);
    obj = parent->win;
    h += 15;
    result->h = h;
  }
  else
  {
    // This is our main window, so create it.
    result->win = elm_win_add(NULL, name, ELM_WIN_BASIC);
    evas_object_smart_callback_add(result->win, "delete,request", _on_done, NULL);
    evas_object_resize(result->win, result->w, result->h);
    evas_object_move(result->win, result->x, result->y);
    elm_win_title_set(result->win, title);
    evas_object_show(result->win);

    obj = result->win;
    x = 0;  y = 0;
  }

  // Create the layout, and add the Edje to it.
  snprintf(buf, sizeof(buf), "%s/winFang.edj", prefix_data_get());
  result->layout = eo_add(ELM_LAYOUT_CLASS, obj,
    efl_gfx_size_set(eoid, w, h),
    efl_gfx_position_set(eoid, x, y),
    evas_obj_name_set(eoid, WF_LAYOUT),
    efl_file_set(eoid, buf, WF_LAYOUT),
    efl_gfx_visible_set(eoid, EINA_TRUE)
  );
  result->e = evas_object_evas_get(result->layout);

  // Sort out the background.
  if (result->parent)
    snprintf(buf, sizeof(buf), "%s/sky_04.jpg", prefix_data_get());
  else
    snprintf(buf, sizeof(buf), "%s/sky_03.jpg", prefix_data_get());
  result->bg = eo_add(ELM_BG_CLASS, obj,
    evas_obj_size_hint_weight_set(eoid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    efl_file_set(eoid, buf, NULL),
    efl_gfx_visible_set(eoid, EINA_TRUE)
  );
  elm_win_resize_object_add(result->win, result->bg);

  if (result->parent)
  {
//	  // RGBA, so this is purple, and semi transparent.
//	  color: 50   0  100   100;  // pre multiplied  R = (r * a) / 255
//	  color: 126  0  255   100;  //                 r = (R * 255) / a
    efl_gfx_color_set(result->bg, 50, 0, 100, 100);
    // This is how fragile things are, internal windows wont show background unless swallowed into the layout,
    // but Evas_3D is fussy about it's background image, and wont work with a layout swallowed one.
    elm_object_part_content_set(result->layout, WF_BACKGROUND, result->bg);
  }

  // Evil hacks for no icons, until toolbars get fixed.
  if (0 == NoIcon[0])
    snprintf(NoIcon, sizeof(NoIcon), "%s/spacer.png", prefix_data_get());
/*  This aint doing shit.  B-(
  obj = elm_icon_add(obj);
  elm_icon_order_lookup_set(obj, ELM_ICON_LOOKUP_THEME_FDO);
  elm_icon_standard_set(obj, NoIcon);
  elm_image_file_set(obj, NoIcon, NULL);
*/

  // Create our box, the default for where content goes.
  result->box = eo_add(ELM_BOX_CLASS, result->layout,
    elm_obj_box_homogeneous_set(eoid, EINA_FALSE),
    elm_obj_box_horizontal_set(eoid, EINA_FALSE),
    evas_obj_size_hint_align_set(eoid, EVAS_HINT_FILL, EVAS_HINT_FILL),
    evas_obj_size_hint_weight_set(eoid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND)
  );
  elm_object_part_content_set(result->layout, WF_BOX, result->box);

  if (result->parent)
  {
    result->win = result->layout;

    // Something to catch clicks on the background, for moving the window.
    // Coz Elm is uncooperative with this sort of thing, so we need to stick in a rectangle.
    obj = eo_add(EVAS_RECTANGLE_CLASS, result->layout,
      evas_obj_size_hint_align_set(eoid, EVAS_HINT_FILL, EVAS_HINT_FILL),
      evas_obj_size_hint_weight_set(eoid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
      evas_obj_name_set(eoid, WF_UNDERLAY),
      efl_gfx_color_set(eoid, 0, 0, 0, 0),
      efl_gfx_visible_set(eoid, EINA_TRUE)
    );
    elm_object_part_content_set(result->layout, WF_UNDERLAY, obj);
    evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN, _onBgClick, result);

    // Create corner handles.
    snprintf(buf, sizeof(buf), "%s/pt.png", prefix_data_get());
    for (i = 0; i < 4; i++)
    {
      int cx = result->x, cy = result->y;

           if (i == 1)   cx += result->w;
      else if (i == 2)  {cx += result->w;  cy += result->h;}
      else if (i == 3)   cy += result->h;
      result->hand[i] = eo_add(EVAS_IMAGE_CLASS, result->e,
        evas_obj_image_filled_set(eoid, EINA_TRUE),
        efl_file_set(eoid, buf, NULL),
        efl_gfx_size_set(eoid, 31, 31),
        efl_gfx_position_set(eoid, cx - 15, cy - 15),
        efl_gfx_visible_set(eoid, EINA_TRUE)
      );
      evas_object_event_callback_add(result->hand[i], EVAS_CALLBACK_MOUSE_MOVE, _onHandleMove, result);
    }

    // Create our window title.
    result->title = eo_add(ELM_LABEL_CLASS, result->layout,
      evas_obj_size_hint_align_set(eoid, EVAS_HINT_FILL, EVAS_HINT_FILL),
      evas_obj_size_hint_weight_set(eoid, EVAS_HINT_EXPAND, 0.0),
      efl_gfx_visible_set(eoid, EINA_TRUE)
    );
    elm_object_style_set(result->title, "marker");
    elm_object_text_set(result->title, title);
    elm_object_part_content_set(result->layout, WF_TITLE, result->title);
  }
  else
  {
    // Hide the window title.
    elm_layout_signal_emit(result->layout, "isMain", "isMain");
  }

  // Create our grid, where things go if you supply coords.
  result->grid = eo_add(ELM_GRID_CLASS, result->layout,
    evas_obj_size_hint_align_set(eoid, EVAS_HINT_FILL, EVAS_HINT_FILL),
    evas_obj_size_hint_weight_set(eoid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_name_set(eoid, WF_GRID),
    elm_obj_grid_size_set(eoid, result->w, result->h),
    efl_gfx_visible_set(eoid, EINA_TRUE)
  );

  // This could live inside the box, but then we have to keep track of the size of it's contents.
  // For now we simply track the position and size of the box.
//  elm_layout_box_append(result->layout, WF_BOX, result->grid);
  elm_object_part_content_set(result->layout, WF_GRID, result->grid);

  elm_layout_sizing_eval(result->layout);

  if (result->parent)
  {
#if 0
    // EPhysics enable the window.
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

    evas_object_resize(result->win, result->w, result->h);
    evas_object_show(result->win);
  }
  winFangCalcMinSize(result);

  return result;
}

void winFangCalcMinSize(winFang *win)
{
  Evas_Object *edje = elm_layout_edje_get(win->layout);
  int w, h;

  // Would be nice if everything properly set their minimums, but they don't.
  //   Actually it looks like BOX gets it's minimum width set to the width of the window.
  win->mw = 0;  win->mh = 0;
  evas_object_size_hint_min_get(edje_object_part_object_get(edje, WF_BOX), &w, &h);
  if (w > win->mw)  win->mw = w;
  if (h > win->mh)  win->mh = h;
  // SWALLOW just returns 0.
  evas_object_size_hint_min_get(edje_object_part_object_get(edje, WF_GRID), &w, &h);
  if (w > win->mw)  win->mw = w;
  if (h > win->mh)  win->mh = h;
  if (win->title)
  {
    // This at least returns proper values.
    evas_object_size_hint_min_get(win->title, &w, &h);
    if (w > win->mw)  win->mw = w;
    win->mh += h;
  }
  // TODO - should peek and poke at the Edje sizes as well.
}

void winFangDel(winFang *win)
{
  winFang *wf, *wf2;
  Widget  *wid, *wid2;

  if (!win)  return;

//  if (win->bg)      eo_unref(win->bg);
//  if (win->grid)    eo_unref(win->grid);
//  if (win->layout)  eo_unref(win->layout);
  EINA_CLIST_FOR_EACH_ENTRY_SAFE(wf, wf2, &win->winFangs, winFang, node)
  {
    winFangDel(wf);
  }

  // Elm will delete our widgets, but if we are using eo, we need to unref them.
  EINA_CLIST_FOR_EACH_ENTRY_SAFE(wid, wid2, &win->widgets, Widget, node)
  {
    if (wid->on_del)  wid->on_del(wid, wid->obj, NULL);
    widgetDel(wid);
  }
  if (win->on_del)  win->on_del(win, win->win, NULL);
  evas_object_del(win->win);
  free(win->module);
  free(win);
}


static widgetSpec widgetClasses[15];

Widget *widgetAdd(winFang *win, char *type , char *title, int x, int y, int w, int h)
{
  Widget *result;
  const Eo_Class *klass = NULL;
  int i;

  // Poor mans introspection.
  if (NULL == widgetClasses[0].name)
  {
    i = 0;
    widgetClasses[i].name = WT_CHECK;	widgetClasses[i++].klass = 	ELM_CHECK_CLASS;
    widgetClasses[i].name = WT_BOX;	widgetClasses[i++].klass = 	ELM_BOX_CLASS;
    widgetClasses[i].name = WT_BUTTON;	widgetClasses[i++].klass = 	ELM_BUTTON_CLASS;
    widgetClasses[i].name = WT_ENTRY;	widgetClasses[i++].klass = 	ELM_ENTRY_CLASS;
    widgetClasses[i].name = WT_FILES;	widgetClasses[i++].klass = 	ELM_FILESELECTOR_CLASS;
    widgetClasses[i].name = WT_GRID;	widgetClasses[i++].klass = 	ELM_GRID_CLASS;
    widgetClasses[i].name = WT_HOVER;	widgetClasses[i++].klass = 	ELM_HOVERSEL_CLASS;
    widgetClasses[i].name = WT_IMAGE;	widgetClasses[i++].klass = 	ELM_IMAGE_CLASS;
    widgetClasses[i].name = WT_LABEL;	widgetClasses[i++].klass = 	ELM_LABEL_CLASS;
    widgetClasses[i].name = WT_LAYOUT;	widgetClasses[i++].klass = 	ELM_LAYOUT_CLASS;
    widgetClasses[i].name = WT_RADIO;	widgetClasses[i++].klass = 	ELM_RADIO_CLASS;
    widgetClasses[i].name = WT_RECT;	widgetClasses[i++].klass = 	EVAS_RECTANGLE_CLASS;
    widgetClasses[i].name = WT_TEXT;	widgetClasses[i++].klass = 	EVAS_TEXT_CLASS;
    widgetClasses[i].name = WT_TEXTBOX; widgetClasses[i++].klass = 	ELM_ENTRY_CLASS;
    widgetClasses[i].name = WT_TOOLBAR;	widgetClasses[i++].klass = 	ELM_TOOLBAR_CLASS;
  }

  for (i = 0; i < ARRAY_LENGTH(widgetClasses); i++)
  {
    if (strcmp(type, widgetClasses[i].name) == 0)
    {
      klass = widgetClasses[i].klass;
      break;
    }
  }

  if (klass)
  {
    result = calloc(1, sizeof(Widget));
    strcpy(result->magic, "Widget");
    strcpy(result->type, type);
    eina_clist_add_head(&win->widgets, &result->node);
    result->win = win;

    result->obj = eo_add(klass, win->win,
      evas_obj_size_hint_weight_set(eoid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
      evas_obj_size_hint_align_set(eoid, EVAS_HINT_FILL, EVAS_HINT_FILL),
      efl_gfx_visible_set(eoid, EINA_TRUE),
      eo_key_data_set(eoid, "Widget", result)
    );

    if (strcmp(WT_ENTRY, type) == 0)
    {
      elm_obj_entry_scrollable_set(result->obj, EINA_TRUE);
      elm_obj_entry_editable_set(result->obj, EINA_TRUE);
    }
    else if (strcmp(WT_TEXTBOX, type) == 0)
    {
      elm_obj_entry_scrollable_set(result->obj, EINA_TRUE);
      elm_obj_entry_editable_set(result->obj, EINA_FALSE);
    }

    if (x < 0)
      elm_box_pack_end(win->box, result->obj);			// WF_BOX
    else
      elm_grid_pack(win->grid, result->obj, x, y, w, h);	// WF_GRID

    if (title)
    {
      result->label = strdup(title);
      elm_object_text_set(result->obj, result->label);
      evas_object_name_set(result->obj, title);
    }
    winFangCalcMinSize(win);
  }

  return result;
}

void widgetDel(Widget *wid)
{
  if (wid)
  {
    free(wid->action);
    free(wid->label);
    // TODO - This is to work around a bug in Elm entry, remove it when the bug is fixed.
    //   The bug is that editable entry widgets cause the app to hang on exit.
    if (strcmp(WT_ENTRY, wid->type) == 0)
      elm_entry_editable_set(wid->obj, EINA_FALSE);
//    eo_unref(wid->obj);
    free(wid);
  }
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


Evas_Object *makeMainMenu(winFang *win)
{
  // A toolbar thingy.
  return eo_add(ELM_TOOLBAR_CLASS, win->layout,
    evas_obj_size_hint_weight_set(eoid, EVAS_HINT_EXPAND, 0.0),
    evas_obj_size_hint_align_set(eoid, EVAS_HINT_FILL, EVAS_HINT_FILL),
    elm_obj_toolbar_shrink_mode_set(eoid, ELM_TOOLBAR_SHRINK_MENU),
    efl_gfx_position_set(eoid, 0, 0),
    elm_obj_toolbar_align_set(eoid, 0.0)
  );
}

//static void _on_menu_focus(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
//{
//  evas_object_raise(obj);
//}

Evas_Object *menuAdd(winFang *win, Evas_Object *tb, char *label)
{
  Evas_Object *menu= NULL;
  Elm_Object_Item *tb_it;

  // Evas_Object * obj, const char *icon, const char *label, Evas_Smart_Cb  func, const void *data
  //   The function is called when the item is clicked.
  tb_it = elm_toolbar_item_append(tb, NoIcon, label, NULL, NULL);
  // Mark it as a menu.
  elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
  // This alledgedly marks it as a menu (not true), and gets an Evas_Object for us to play with.
  menu = elm_toolbar_item_menu_get(tb_it);
  // Alas this does not work.  B-(
//  evas_object_smart_callback_add(menu, "focused", _on_menu_focus, NULL);

  // Priority is for when toolbar items are set to hide or menu when there are too many of them.  They get hidden or put on the menu based on priority.
  elm_toolbar_item_priority_set(tb_it, 9999);

  // The docs for this functien are meaningless, but it wont work right if you leave it out.
  elm_toolbar_menu_parent_set(tb, win->win);

  elm_toolbar_homogeneous_set(tb, EINA_FALSE);

  // Add a seperator after, so that there's some visual distinction between menu items.
  // Coz using all lower case and with no underline on the first letter, it's hard to tell.
  tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
  elm_toolbar_item_separator_set(tb_it, EINA_TRUE);

  return menu;
}

void makeMainMenuFinish(winFang *win, Evas_Object *tb)
{
  // The toolbar needs to be packed into the box AFTER the menus are added.
  elm_object_part_content_set(win->layout, WF_TOOLBAR, tb);
  elm_layout_signal_emit(win->layout, "isToolbar", "isToolbar");
  evas_object_show(tb);

  // Bump the menu above the windows.
  evas_object_raise(tb);
}
