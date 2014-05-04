#include "extantz.h"


static void my_fileselector_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   /* event_info conatin the full path of the selected file
    * or NULL if none is selected or cancel is pressed */
    const char *selected = event_info;

    if (selected)
      printf("Selected file: %s\n", selected);
    else
      evas_object_del(data);  /* delete the test window */
}

static void my_fileselector_selected(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   /* event_info conatin the full path of the selected file */
   const char *selected = event_info;
   printf("Selected file: %s\n", selected);

   /* or you can query the selection */
   if (elm_fileselector_multi_select_get(obj))
     {
        const Eina_List *li;
        const Eina_List *paths = elm_fileselector_selected_paths_get(obj);
        char *path;
        printf("All selected files are:\n");
        EINA_LIST_FOREACH(paths, li, path)
          printf(" %s\n", path);
     }
   else
     printf("or: %s\n", elm_fileselector_selected_get(obj));
}

static void _popup_close_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void my_fileselector_invalid(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Object *popup;
   Evas_Object *btn;
   char error_msg[256];

   snprintf(error_msg, 256, "No such file or directory: %s", (char *)event_info);

   popup = elm_popup_add(data);
   elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_CHAR);
   elm_object_part_text_set(popup, "title,text", "Error");
   elm_object_text_set(popup, error_msg);

   btn = elm_button_add(popup);
   elm_object_text_set(btn, "OK");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_close_cb, popup);

   evas_object_show(popup);
}

static void my_fileselector_activated(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("Activated file: %s\n", (char *)event_info);
}

static void _expandable_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;

   if (elm_fileselector_expandable_get(fs))
     elm_fileselector_expandable_set(fs, EINA_FALSE);
   else
     elm_fileselector_expandable_set(fs, EINA_TRUE);
}

static void _hidden_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   if (elm_fileselector_hidden_visible_get(fs))
     elm_fileselector_hidden_visible_set(fs, EINA_FALSE);
   else
     elm_fileselector_hidden_visible_set(fs, EINA_TRUE);
}

static void _mode_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Elm_Fileselector_Mode mode;

   mode = (elm_fileselector_mode_get(fs) + 1) % ELM_FILESELECTOR_LAST;
   elm_radio_value_set(obj, mode);
   elm_fileselector_mode_set(data, mode);
}

static void _sort_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs = evas_object_data_get(obj, "fileselector");
   const char *selected = elm_object_item_text_get(event_info);

   elm_object_text_set(obj, selected);
   elm_fileselector_sort_method_set(fs, (Elm_Fileselector_Sort)data);
}

static void _tiny_icon_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Evas_Coord w, h;

   elm_fileselector_thumbnail_size_get(fs, &w, &h);
   elm_fileselector_thumbnail_size_set(fs, 16, 16);
}

static void _small_icon_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Evas_Coord w, h;

   elm_fileselector_thumbnail_size_get(fs, &w, &h);
   elm_fileselector_thumbnail_size_set(fs, 32, 32);
}

static void _middle_icon_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Evas_Coord w, h;

   elm_fileselector_thumbnail_size_get(fs, &w, &h);
   elm_fileselector_thumbnail_size_set(fs, 64, 64);
}

static void _big_icon_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;
   Evas_Coord w, h;

   elm_fileselector_thumbnail_size_get(fs, &w, &h);
   elm_fileselector_thumbnail_size_set(fs, 128, 128);
}

static Eina_Bool _all_filter(const char *path EINA_UNUSED, Eina_Bool dir EINA_UNUSED, void *data EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool _edje_filter(const char *path, Eina_Bool dir, void *data EINA_UNUSED)
{
   if (dir) return EINA_TRUE;

   if (eina_str_has_extension(path, ".edc") ||
       eina_str_has_extension(path, ".edj"))
     return EINA_TRUE;
   return EINA_FALSE;
}

fangWin *files_add(globals *ourGlobals)
{
  fangWin *me;
  Evas_Object *bx, *vbox, *fs, *bt, *rd = NULL, *rdg = NULL, *hoversel;

  me = fang_win_add(ourGlobals);

  bx = eo_add(ELM_OBJ_BOX_CLASS, me->win);
  eo_do(bx,
    elm_obj_box_homogeneous_set(EINA_FALSE),
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL)
  );
  elm_win_resize_object_add(me->win, bx);

  fs = eo_add(ELM_OBJ_FILESELECTOR_CLASS, bx);
  eo_do(fs,
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
    evas_obj_visibility_set(EINA_TRUE)
       );
  elm_box_pack_end(bx, fs);

   /* disnable the fs file name entry */
   elm_fileselector_is_save_set(fs, EINA_FALSE);
   /* make the file list a tree with dir expandable in place */
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   /* start the fileselector in the home dir */
   elm_fileselector_path_set(fs, getenv("HOME"));
   elm_fileselector_folder_only_set(fs, EINA_FALSE);
   elm_fileselector_buttons_ok_cancel_set(fs, EINA_TRUE);
   elm_fileselector_multi_select_set(fs, EINA_TRUE);

   /* add filesters */
   elm_fileselector_custom_filter_append(fs, _all_filter, NULL, "all files");
   elm_fileselector_custom_filter_append(fs, _edje_filter, NULL, "edje files");
   elm_fileselector_mime_types_filter_append(fs, "image/*", "image files");
   elm_fileselector_mime_types_filter_append(fs, "text/*", "text files");

   evas_object_smart_callback_add(fs, "done", my_fileselector_done, win);
   evas_object_smart_callback_add(fs, "selected", my_fileselector_selected, win);
   evas_object_smart_callback_add(fs, "selected,invalid", my_fileselector_invalid, win);
   evas_object_smart_callback_add(fs, "activated", my_fileselector_activated, win);

  eo_unref(fs);

  vbox = eo_add(ELM_OBJ_BOX_CLASS, me->win);
  eo_do(vbox,
    elm_obj_box_homogeneous_set(EINA_FALSE),
    elm_obj_box_horizontal_set(EINA_TRUE),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL)
  );

   bt = elm_check_add(vbox);
   elm_object_text_set(bt, "tree");
   elm_check_state_set(bt, elm_fileselector_expandable_get(fs));
   evas_object_smart_callback_add(bt, "changed", _expandable_clicked, fs);
   elm_box_pack_end(vbox, bt);
   evas_object_show(bt);

   bt = elm_check_add(vbox);
   elm_object_text_set(bt, "hidden");
   elm_check_state_set(bt, elm_fileselector_hidden_visible_get(fs));
   evas_object_smart_callback_add(bt, "changed", _hidden_clicked, fs);
   elm_box_pack_end(vbox, bt);
   evas_object_show(bt);

   rdg = rd = elm_radio_add(vbox);
   elm_radio_state_value_set(rd, ELM_FILESELECTOR_LIST);
   elm_object_text_set(rd, "list");
   elm_box_pack_end(vbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _mode_changed_cb, fs);

   rd = elm_radio_add(vbox);
   elm_radio_group_add(rd, rdg);
   elm_radio_state_value_set(rd, ELM_FILESELECTOR_GRID);
   elm_object_text_set(rd, "grid");
   elm_box_pack_end(vbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _mode_changed_cb, fs);


   hoversel = elm_hoversel_add(vbox);
   elm_hoversel_hover_parent_set(hoversel, me->win);
   evas_object_data_set(hoversel, "fileselector", fs);
   elm_object_text_set(hoversel, "sorting");

   elm_hoversel_item_add(hoversel, "name(asc)",  NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_FILENAME_ASC);
   elm_hoversel_item_add(hoversel, "name(desc)", NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_FILENAME_DESC);
   elm_hoversel_item_add(hoversel, "type(asc)",  NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_TYPE_ASC);
   elm_hoversel_item_add(hoversel, "type(desc)", NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_TYPE_DESC);
   elm_hoversel_item_add(hoversel, "size(asc)",  NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_SIZE_ASC);
   elm_hoversel_item_add(hoversel, "size(desc)", NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_SIZE_DESC);
   elm_hoversel_item_add(hoversel, "time(asc)",  NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_MODIFIED_ASC);
   elm_hoversel_item_add(hoversel, "time(desc)", NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_MODIFIED_DESC);

   elm_box_pack_end(vbox, hoversel);
   evas_object_show(hoversel);

   hoversel = elm_hoversel_add(vbox);
   elm_hoversel_hover_parent_set(hoversel, me->win);
   evas_object_data_set(hoversel, "fileselector", fs);
   elm_object_text_set(hoversel, "size");

   elm_hoversel_item_add(hoversel, "tiny",   NULL, ELM_ICON_NONE, _tiny_icon_clicked,   fs);
   elm_hoversel_item_add(hoversel, "small",  NULL, ELM_ICON_NONE, _small_icon_clicked,  fs);
   elm_hoversel_item_add(hoversel, "medium", NULL, ELM_ICON_NONE, _middle_icon_clicked, fs);
   elm_hoversel_item_add(hoversel, "big",    NULL, ELM_ICON_NONE, _big_icon_clicked,    fs);

   elm_box_pack_end(vbox, hoversel);
   evas_object_show(hoversel);

   elm_box_pack_end(bx, vbox);
   evas_object_show(vbox);
  evas_object_show(bx);
  eo_unref(bx);

  fang_win_complete(ourGlobals, me, ourGlobals->win_w - 380, ourGlobals->win_w - 530, 350, 500);
  return me;
}
