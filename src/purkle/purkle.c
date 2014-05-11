//#include "LumbrJack.h"
#include "GuiLua.h"
#include "Runnr.h"
#include "winFang.h"


// TODO - This is to work around a bug in Elm entry, remove it when the bug is fixed.
//   The bug is that editable entry widgets cause the app to hang on exit.
static void _on_entry_del(void *data, Evas_Object *obj, void *event_info)
{
//  winFang *me = data;

  elm_entry_editable_set(obj, EINA_FALSE);
}

static winFang *purkleAdd(winFang *parent, int w, int h, EPhysics_World *world)
{
  winFang *me;
  Widget  *wid;
  Evas_Object *en;

  me = winFangAdd(parent, 30, 590, w, h, "chatter box", "purkle", world);

  en = eo_add(ELM_OBJ_ENTRY_CLASS, me->win,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_FALSE),
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
    evas_obj_visibility_set(EINA_TRUE)
       );
  elm_object_text_set(en, "History is shown here");
  elm_layout_box_append(me->win, WF_BOX, en);
  eo_unref(en);

  wid = widgetAdd(me, ELM_OBJ_ENTRY_CLASS, "", -1, -1, -1, -1);
  wid->on_del = _on_entry_del;
  eo_do(wid->obj,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_TRUE)
       );

  winFangCalcMinSize(me);

  return me;
}

static const char *ourName = "purkle";
int skang, _M;

int luaopen_purkle(lua_State *L)
{
  GuiLua *gl;
  winFang *parent = NULL;
  EPhysics_World *world = NULL;

// local skang = require 'skang'
  lua_getglobal(L, "require");
  lua_pushstring(L, SKANG);
  lua_call(L, 1, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, SKANG);
  lua_getfield(L, LUA_REGISTRYINDEX, SKANG);
  skang = lua_gettop(L);

// local _M = skang.moduleBegin('test_c', nil, 'Copyright 2014 David Seikel', '0.1', '2014-03-27 03:57:00', nil, false)
  push_lua(L, "@ ( $ ~ $ $ $ ~ ! )", skang, MODULEBEGIN, ourName, "Copyright 2014 David Seikel", "0.1", "2014-05-08 07:18:00", 0, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, ourName);
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);
  _M = lua_gettop(L);

  lua_getfield(L, LUA_REGISTRYINDEX, glName);
  gl = lua_touserdata(L, -1);
  lua_pop(L, 1);
  if (gl)
  {
    parent = gl->parent;
    world = gl->world;
  }

  purkleAdd(parent, 200, 400, world);

  push_lua(L, "@ ( = )", skang, MODULEEND, _M, 0);

  // Return _M, the table itself, not the index.
  return 1;
}
