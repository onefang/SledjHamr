#include "extantz.h"


void chat_add(globals *ourGlobals)
{
  Evas_Object *win, *bx, *en;

  win = fang_win_add(ourGlobals);

  bx = eo_add(ELM_OBJ_BOX_CLASS, win);
  eo_do(bx,
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL)
    );
  elm_win_resize_object_add(win, bx);

  en = eo_add(ELM_OBJ_ENTRY_CLASS, win);
  elm_object_text_set(en, "History is shown here");
  eo_do(en,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_FALSE),
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
    evas_obj_visibility_set(EINA_TRUE)
    );
  elm_box_pack_end(bx, en);
  eo_unref(en);

  en = eo_add(ELM_OBJ_ENTRY_CLASS, win);
  elm_object_text_set(en, "");
  eo_do(en,
    elm_obj_entry_scrollable_set(EINA_TRUE),
// TODO - Setting editable to TRUE is what hangs up extantz on exit.
//	elm_obj_entry_editable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_FALSE),
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
    evas_obj_visibility_set(EINA_TRUE)
    );
  elm_box_pack_end(bx, en);
  eo_unref(en);

  evas_object_show(bx);
  eo_unref(bx);

  fang_win_complete(ourGlobals, win, 30, 500, ourGlobals->win_w / 3, ourGlobals->win_h / 3);
}
