#include "extantz.h"
#include <elm_interface_fileselector.h>


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

   elm_fileselector_thumbnail_size_set(fs, 16, 16);
}

static void _small_icon_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;

   elm_fileselector_thumbnail_size_set(fs, 32, 32);
}

static void _middle_icon_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;

   elm_fileselector_thumbnail_size_set(fs, 64, 64);
}

static void _big_icon_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fs = data;

   elm_fileselector_thumbnail_size_set(fs, 128, 128);
}

static void my_fileselector_activated(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  winFang *me = data;
  Evas_Object *fs = me->data;

   if (elm_fileselector_multi_select_get(fs))
   {
     Eina_List const *files = elm_fileselector_selected_paths_get(fs), *i;
     char *file;

     printf("SELECTED files : \n");
     EINA_LIST_FOREACH(files, i, file)
     {
       printf("              %s\n", file);
     }
   }
   else
   {
     char const *file = elm_fileselector_selected_get(fs);

     printf("SELECTED file : %s\n", file);
   }
  winFangHide(me);
}

static void _CANCEL_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  winFang *me = data;

  winFangHide(me);
}

static void _OK_clicked(void *data, Evas_Object *obj, void *event_info)
{
   my_fileselector_activated(data, obj, event_info);
}

#if 0
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
#endif

// TODO - This is to work around a bug in Elm entry, remove it when the bug is fixed.
//   The bug is that editable entry widgets cause the app to hang on exit.
//   In this case, we have the name and path entry widgets.
void _on_fs_del(void *data, Evas_Object *obj, void *event_info)
{
  // Make sure name entry is not editable.
  elm_fileselector_is_save_set(obj, EINA_FALSE);

/*  For future reference, these are the swallowed parts as of 2014-05-04 -
   SWALLOW("elm.swallow.up", sd->up_button);
   SWALLOW("elm.swallow.home", sd->home_button);
   SWALLOW("elm.swallow.spinner", sd->spinner);

   elm_layout_content_set(obj, "elm.swallow.files", sd->files_view);

   SWALLOW("elm.swallow.path", sd->path_entry);
   SWALLOW("elm.swallow.filename", sd->name_entry);

   SWALLOW("elm.swallow.filters", sd->filter_hoversel);
   SWALLOW("elm.swallow.cancel", sd->cancel_button);
   SWALLOW("elm.swallow.ok", sd->ok_button);
*/

  // Make sure path entry is not editable.  We have to dig it out using private info here.
  obj = elm_layout_content_get(obj, "elm.swallow.path");
  elm_entry_editable_set(obj, EINA_FALSE);
}

winFang *filesAdd(globals *ourGlobals, char *path, Eina_Bool multi, Eina_Bool save)
{
  winFang *me;
  Widget  *wid;
  Evas_Object *bx, *vbox, *fs, *bt, *rd = NULL, *rdg = NULL, *hoversel;

  me = winFangAdd(ourGlobals->mainWindow, ourGlobals->win_w - 380, ourGlobals->win_w - 530, 350, 500, "file selector", "files");

  bx = eo_add(ELM_OBJ_BOX_CLASS, me->win,
    elm_obj_box_homogeneous_set(EINA_FALSE),
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL)
  );
  elm_win_resize_object_add(me->win, bx);

  wid = widgetAdd(me, ELM_OBJ_FILESELECTOR_CLASS, bx, NULL);
  fs = wid->obj;
  wid->data = ourGlobals;
  wid->on_del = _on_fs_del;
  me->data = fs;
  eo_do(fs,
    elm_obj_fileselector_buttons_ok_cancel_set(EINA_FALSE),
    elm_interface_fileselector_expandable_set(EINA_TRUE),
    elm_interface_fileselector_folder_only_set(EINA_FALSE)
       );
  elm_box_pack_end(bx, fs);

  elm_fileselector_path_set(fs, path);
  elm_fileselector_is_save_set(fs, save);
  elm_fileselector_multi_select_set(fs, multi);

  // TODO - Should allow these to be set from the caller.
  // TODO - Don't do these, it adds a horribly out of place button.
  // Either fix Elm file selector to be more general purpose,
  //        clone and fix the fileselector theme,
  //        or write my own file selector.
//  elm_fileselector_custom_filter_append(fs, _all_filter, NULL, "all files");
//  elm_fileselector_custom_filter_append(fs, _edje_filter, NULL, "edje files");
//  elm_fileselector_mime_types_filter_append(fs, "image/*", "image files");
//  elm_fileselector_mime_types_filter_append(fs, "text/*", "text files");

  // Call back for double click or Enter pressed on file.
  evas_object_smart_callback_add(fs, "activated", my_fileselector_activated, me);

  vbox = eo_add(ELM_OBJ_BOX_CLASS, me->win,
    elm_obj_box_homogeneous_set(EINA_FALSE),
    elm_obj_box_horizontal_set(EINA_TRUE),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL)
  );

  hoversel = eo_add(ELM_OBJ_HOVERSEL_CLASS, vbox,
    elm_obj_hoversel_hover_parent_set(me->win),
    eo_key_data_set("fileselector", fs, NULL),
    elm_obj_hoversel_item_add("name(asc)",  NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_FILENAME_ASC),
    elm_obj_hoversel_item_add("name(desc)", NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_FILENAME_DESC),
    elm_obj_hoversel_item_add("type(asc)",  NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_TYPE_ASC),
    elm_obj_hoversel_item_add("type(desc)", NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_TYPE_DESC),
    elm_obj_hoversel_item_add("size(asc)",  NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_SIZE_ASC),
    elm_obj_hoversel_item_add("size(desc)", NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_SIZE_DESC),
    elm_obj_hoversel_item_add("time(asc)",  NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_MODIFIED_ASC),
    elm_obj_hoversel_item_add("time(desc)", NULL, ELM_ICON_NONE, _sort_selected_cb, (const void *) ELM_FILESELECTOR_SORT_BY_MODIFIED_DESC),
    evas_obj_visibility_set(EINA_TRUE)
    );
  elm_object_text_set(hoversel, "sorting");
  elm_box_pack_end(vbox, hoversel);
  eo_unref(hoversel);

  hoversel = eo_add(ELM_OBJ_HOVERSEL_CLASS, vbox,
    elm_obj_hoversel_hover_parent_set(me->win),
    eo_key_data_set("fileselector", fs, NULL),
    elm_obj_hoversel_item_add("tiny",   NULL, ELM_ICON_NONE, _tiny_icon_clicked,   fs),
    elm_obj_hoversel_item_add("small",  NULL, ELM_ICON_NONE, _small_icon_clicked,  fs),
    elm_obj_hoversel_item_add("medium", NULL, ELM_ICON_NONE, _middle_icon_clicked, fs),
    elm_obj_hoversel_item_add("big",    NULL, ELM_ICON_NONE, _big_icon_clicked,    fs),
    evas_obj_visibility_set(EINA_TRUE)
    );
  elm_object_text_set(hoversel, "size");
  elm_box_pack_end(vbox, hoversel);
  // Make sure it starts off as small, works around "hitting grid mode before hitting size not showing anything" bug.
  _small_icon_clicked(fs, hoversel, NULL);
  eo_unref(hoversel);


  bt = eo_add(ELM_OBJ_CHECK_CLASS, vbox,
    elm_obj_check_state_set(elm_fileselector_hidden_visible_get(fs)),
    evas_obj_visibility_set(EINA_TRUE)
    );
  elm_object_text_set(bt, "hidden");
  evas_object_smart_callback_add(bt, "changed", _hidden_clicked, fs);
  elm_box_pack_end(vbox, bt);
  eo_unref(bt);

  rdg = rd = eo_add(ELM_OBJ_RADIO_CLASS, vbox,
    elm_obj_radio_state_value_set(ELM_FILESELECTOR_GRID),
    evas_obj_visibility_set(EINA_TRUE)
    );
  elm_object_text_set(rd, "grid");
  elm_box_pack_end(vbox, rd);
  evas_object_smart_callback_add(rd, "changed", _mode_changed_cb, fs);
  // Make it start in grid mode.  It defaults to list mode, so this swaps it over.
  _mode_changed_cb(fs, rd, NULL);
  eo_unref(rd);

  rd = eo_add(ELM_OBJ_RADIO_CLASS, vbox,
    elm_obj_radio_state_value_set(ELM_FILESELECTOR_LIST),
    evas_obj_visibility_set(EINA_TRUE)
    );
  elm_radio_group_add(rd, rdg);
  elm_object_text_set(rd, "list");
  elm_box_pack_end(vbox, rd);
  evas_object_smart_callback_add(rd, "changed", _mode_changed_cb, fs);
  eo_unref(rd);
  // No need to unref this, it's taken care of already.
  //eo_unref(rdg);

  bt = eo_add(ELM_OBJ_BUTTON_CLASS, me->win,
    evas_obj_visibility_set(EINA_TRUE)
  );
  elm_object_text_set(bt, "OK");
  evas_object_smart_callback_add(bt, "clicked", _OK_clicked, me);
  elm_box_pack_end(vbox, bt);
  eo_unref(bt);

  bt = eo_add(ELM_OBJ_BUTTON_CLASS, me->win,
    evas_obj_visibility_set(EINA_TRUE)
  );
  elm_object_text_set(bt, "CANCEL");
  evas_object_smart_callback_add(bt, "clicked", _CANCEL_clicked, me);
  elm_box_pack_end(vbox, bt);
  eo_unref(bt);

  elm_box_pack_end(bx, vbox);
  evas_object_show(vbox);
  evas_object_show(bx);
  eo_unref(vbox);
  eo_unref(bx);

  winFangHide(me);
  return me;
}

void filesShow(winFang *me, Evas_Smart_Cb func, void *data)
{
  Evas_Object *fs = me->data;

  if (!data)  data = me;

  if (func)
    evas_object_smart_callback_add(fs, "activated", func, data);
  else
    evas_object_smart_callback_add(fs, "activated", my_fileselector_activated, me);
  winFangShow(me);
}
