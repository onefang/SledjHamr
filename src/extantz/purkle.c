#include "extantz.h"


// TODO - This is to work around a bug in Elm entry, remove it when the bug is fixed.
//   The bug is that editable entry widgets cause the app to hang on exit.
static void _on_entry_del(void *data, Evas_Object *obj, void *event_info)
{
//  winFang *me = data;

  elm_entry_editable_set(obj, EINA_FALSE);
}

winFang *purkleAdd(globals *ourGlobals)
{
  winFang *me;
  Widget  *wid;
  Evas_Object *en;

  me = winFangAdd(ourGlobals->mainWindow, 30, 520, ourGlobals->win_w / 3, ourGlobals->win_h / 3, "chatter box", "purkle");

  en = eo_add(ELM_OBJ_ENTRY_CLASS, me->win,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_FALSE),
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
    evas_obj_visibility_set(EINA_TRUE)
       );
  elm_object_text_set(en, "History is shown here");
  elm_box_pack_end(me->box, en);
  eo_unref(en);

  wid = widgetAdd(me, ELM_OBJ_ENTRY_CLASS, me->win, "");
  wid->on_del = _on_entry_del;
  eo_do(wid->obj,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_TRUE)
       );
  elm_box_pack_end(me->box, wid->obj);

  evas_object_show(me->box);

  return me;
}
