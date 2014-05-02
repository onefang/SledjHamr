#include "extantz.h"


void chat_add(GLData *gld)
{
    Evas_Object *win, *bx, *en;

    win = fang_win_add(gld);

    bx = elm_box_add(win);
    elm_win_resize_object_add(win, bx);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

    en = elm_entry_add(win);
    elm_entry_scrollable_set(en, EINA_TRUE);
    evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_text_set(en, "History is shown here");
    elm_entry_editable_set(en, EINA_FALSE);
    evas_object_show(en);
    elm_box_pack_end(bx, en);

    en = elm_entry_add(win);
    elm_entry_scrollable_set(en, EINA_TRUE);
    evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_text_set(en, "");
    elm_entry_editable_set(en, EINA_TRUE);
    evas_object_show(en);
    elm_box_pack_end(bx, en);

    evas_object_show(bx);

    fang_win_complete(gld, win, 30, 500, gld->win_w / 3, gld->win_h / 3);
}
