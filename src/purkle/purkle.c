//#include "LumbrJack.h"
#include "GuiLua.h"
#include "Runnr.h"
#include "winFang.h"


static const char *ourName = "purkle";
static int skang, _M;
static Widget *entry, *history;
static winFang *me;
static Ecore_Con_Server *server;

static winFang *purkleAdd(winFang *parent, int w, int h, EPhysics_World *world)
{
  winFang *me;

  me = winFangAdd(parent, 30, 590, w, h, "chatter box", "purkle", world);

  history = widgetAdd(me, WT_TEXTBOX, "History is shown here", -1, -1, -1, -1);
  eo_do(history->obj,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_FALSE)
       );

  entry = widgetAdd(me, WT_ENTRY, "", -1, -1, -1, -1);
  eo_do(entry->obj,
    elm_obj_entry_scrollable_set(EINA_TRUE),
    elm_obj_entry_editable_set(EINA_TRUE)
       );

  winFangCalcMinSize(me);

  return me;
}

static int append(lua_State *L)
{
  char *text = NULL;

  pull_lua(L, 1, "$", &text);
  if (text)
  {
    eo_do(history->obj,
      elm_obj_entry_entry_append("<br/>"),
      // TODO - Add a time stamp, and log to a file.
      elm_obj_entry_entry_append(text),
      // TODO - really need a "scroll to the bottom" here, this cursor down wont work if lines get wrapped onto multiple lines.
      elm_obj_entry_cursor_down()
    );
  }
  return 0;
}

static int say(lua_State *L)
{
  char *name = "", *id = NULL, *text = NULL, buf[PATH_MAX];
  int channel;

printf("PURKLE.SAY .....\n");
  // TODO - Should include origin and distance?
  pull_lua(L, 1, "%channel $name $id $text", &channel, &name, &id, &text);
  if (id && text)
  {
    snprintf(buf, sizeof(buf), "events.listen(%d, %s, %s, %s)", channel, name, id, text);
printf("PURKLE.SAY -%s.%s\n", id, buf);
    if (server)  sendForth(server, id, buf);
  }
  return 0;
}

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
  _M = lua_gettop(L);

  push_lua(L, "@ ( = $ $ & $ )", skang, THINGASM, _M, "append", "Append text to the history box.", append, "string", 0);
  push_lua(L, "@ ( = $ $ & $ )", skang, THINGASM, _M, "say",    "Send chat to a channel.",         say,    "number,string,string,string", 0);

  lua_getfield(L, LUA_REGISTRYINDEX, glName);
  gl = lua_touserdata(L, -1);
  lua_pop(L, 1);
  if (gl)
  {
    parent = gl->parent;
    world = gl->world;
    server = gl->server;
  }

  if (!me)  me = purkleAdd(parent, 500, 420, world);
  push_lua(L, "@ ( = )", skang, MODULEEND, _M, 0);

  // Return _M, the table itself, not the index.
  return 1;
}
