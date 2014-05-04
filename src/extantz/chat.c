#include "extantz.h"


// TODO - This is to work around a bug in Elm entry, remove it when the bug is fixed.
//   The bug is that editable entry widgets cause the app to hang on exit.
void _on_entry_del(void *data, Evas_Object *obj, void *event_info)
{
//  winFang *me = data;

  elm_entry_editable_set(obj, EINA_FALSE);
}

winFang *chat_add(globals *ourGlobals)
{
  winFang *me;
  Widget  *wid;
  Evas_Object *bx, *en;

  me = winFangAdd(ourGlobals->win, 30, 500, ourGlobals->win_w / 3, ourGlobals->win_h / 3, "chatter box", "chat");
  eina_clist_add_head(&ourGlobals->winFangs, &me->node);

  bx = eo_add(ELM_OBJ_BOX_CLASS, me->win,
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL)
       );
  elm_win_resize_object_add(me->win, bx);

  en = eo_add(ELM_OBJ_ENTRY_CLASS, me->win,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_FALSE),
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
    evas_obj_visibility_set(EINA_TRUE)
       );
  elm_object_text_set(en, "History is shown here");
  elm_box_pack_end(bx, en);
  eo_unref(en);

  wid = widgetAdd(me, ELM_OBJ_ENTRY_CLASS, me->win, "");
  wid->on_del = _on_entry_del;
  eo_do(wid->obj,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_TRUE)
       );
  elm_box_pack_end(bx, wid->obj);

  evas_object_show(bx);
  eo_unref(bx);

  return me;
}
