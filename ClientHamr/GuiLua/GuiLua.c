/*  GuiLua - a GUI library that implements matrix-RAD style stuff.

Provides the skang and widget Lua packages.

In the initial intended use case, several applications will be using
this all at once, with one central app hosting all the GUIs.

Basically this should deal with "windows" and their contents.  A
"window" in this case is hosted in the central app as some sort of
internal window, but the user can "tear off" those windows, then they
get their own OS hosted window.  This could be done by the hosting app
sending the current window contents to the original app as a skang file.

Between the actual GUI and the app might be a socket, or a stdin/out
pipe.  Just like matrix-RAD, this should be transparent to the app. 
Also just like matrix-RAD, widgets can be connected to variable /
functions (C or Lua), and any twiddlings with those widgets runs the
function / changes the variable, again transparent to the app, except
for any registered get/set methods.

This interface between the GUI and the app is "skang" files, which are
basically Lua scripts.  The GUI and the app can send skang files back
and forth, usually the app sends actual GUI stuff, and usually the GUI
sends variable twiddles or action calls.  Usually.

To start with, this will be used to support multiple apps hosting their
windows in extantz, allowing the big viewer blob to be split up into
modules.  At some point converting LL XML based UI shit into skang could
be done.  Also, this should be an exntension to LuaSL, so in-world
scripts can have a poper GUI for a change.


NOTES and TODOs -

Lua scripts do -
  require 'widget'  -> loads widget.c
  Widget.c is a library like test_c.
  It starts up GuiLua.c app, with stdin/stdout pipe.
  Widget.c then acts as a proxy for all the widget stuff.
  So Lua modules via C libraries can work with Elm code that has a special main and has to be an app.
  Seems simplest.

  Also -
    Some of this gets shared with LuaSL, since it came from there anyway.

  Finally -
    Add a --gui command line option, that runs foo.skang.
    Than change the hash bang to use it.
    And if there's a matching module, load the module first, call gimmeSkin() on it.
    So that any with an internal default skin get that instead.
    Same if theer's a module, but no skanf file.

Making these packages all a sub package of skang seems like a great
idea.  On the other hand, looks like most things are just getting folded
into skang anyway.  See
http://www.inf.puc-rio.br/~roberto/pil2/chapter15.pdf part 15.5 for
package details.

See if I can use LuaJIT FFI here.  Since this will be a library, and
skang apps could be written in C or Lua, perhaps writing this library to
be FFI friendly instead of the usual Lua C binding might be the way to
go?

For the "GUI hosted in another app" case, we will need some sort of
internal window manager running in that other app.

This might end up running dozens of Lua scripts, and could use the LuaSL
Lua script running system.  Moving that into this library might be a
sane idea I think?  Or prehaps a separate library that both LuaSL and
GuiLua use?

Raster wants a method of sending Lua tables around as edje messages. 
Between C, Edje, Edje Lua, and Lua.  Sending between threads, and across
sockets.  Using a new edje message type, or eet for sockets, was
suggested, but perhaps Lua skang is a better choice?

Somehow access to the edje_lua2.c bindings should be provided.  And
bindings to the rest of EFL when they are done.  Assuming the other EFL
developers do proper introspection stuff, or let me do it.

The generic Lua binding helper functions I wrote for edje_lua2.c could
be used here as well, and expanded as discussed on the E devs mailing
list.  This would include the thread safe Lua function stuff copied
into the README.

There will eventually be a built in editor, like the zen editor from
matrix-RAD.  It might be a separate app.

NAWS should probably live in here to.  If I ever get around to writing
it.  lol

The pre tokenized widget structure thingy I had planned in the
matrix-RAD TODO just wont work, as it uses symbols.  On the other hand,
we will be using Lua tables anyway.  B-)

The last half of http://passingcuriosity.com/2009/extending-lua-in-c/
might be of use.

*/


/* thing package

Currently this is in skang.lua, but should bring this in here later.

*/


/* skang package

Currently this is in skang.lua, but should bring this in here later.

*/


/* stuff & squeal packages

Currently Stuff is in skang.lua, but should bring this in here later.

*/


/* widget package

Currently widget design is in skang.lua, but should bring this in here later.

*/


/* introspection

As detailed in README, EFL introspection doesn't seem to really be on
the radar, but I might get lucky, or I might have to write it myself. 
For quick and dirty early testing, I'll probably write a widget package
that has hard coded mappings between some basic "label", "button", etc.
and ordinary elementary widgets.  Proper introspection can come later.

*/



#include "GuiLua.h"

globals ourGlobals;

static const char *ourName = "widget";
static int skang, _M;


void dumpStack(lua_State *L, int i)
{
  int type = lua_type(L, i);

  switch (type)
  {
    case LUA_TNONE		:  printf("Stack %d is empty\n", i);  break;
    case LUA_TNIL		:  printf("Stack %d is a nil\n", i);  break;
    case LUA_TBOOLEAN		:  printf("Stack %d is a boolean - %d\n", i, lua_toboolean(L, i));  break;
    case LUA_TNUMBER		:  printf("Stack %d is a number\n - %f", i, lua_tonumber(L, i));  break;
    case LUA_TSTRING		:  printf("Stack %d is a string - %s\n", i, lua_tostring(L, i));  break;
    case LUA_TFUNCTION		:  printf("Stack %d is a function\n", i);  break;
    case LUA_TTHREAD		:  printf("Stack %d is a thread\n", i);  break;
    case LUA_TTABLE		:
    {
      int j;

      printf("Stack %d is a table", i);
      lua_getfield(L, i, "_NAME");
      j = lua_gettop(L);
      if (lua_isstring(L, j))
        printf(" - %s", lua_tostring(L, j));
      lua_pop(L, 1);
      printf("\n");
      break;
    }
    case LUA_TUSERDATA		:  printf("Stack %d is a userdata\n", i);  break;
    case LUA_TLIGHTUSERDATA	:  printf("Stack %d is a light userdata\n", i);  break;
    default			:  printf("Stack %d is unknown\n", i);  break;
  }
}

static char dateTime[DATE_TIME_LEN];

static void _ggg_log_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args)
{
  FILE *f = data;
  char dt[DATE_TIME_LEN + 1];
  char fileTab[256], funcTab[256];

  getDateTime(NULL, dt, NULL);
  dt[19] = '\0';
  if (12 > strlen(file))
    snprintf(fileTab, sizeof(fileTab), "%s\t\t", file);
  else
    snprintf(fileTab, sizeof(fileTab), "%s\t", file);
  snprintf(funcTab, sizeof(funcTab), "\t%s", fnc);
  fprintf(f, "%s ", dt);
  if (f == stderr)
    eina_log_print_cb_stderr(d, level, fileTab, funcTab, line, fmt, data, args);
  else if (f == stdout)
    eina_log_print_cb_stdout(d, level, fileTab, funcTab, line, fmt, data, args);
  fflush(f);
}

void loggingStartup(globals *ourGlobals)
{
  if (ourGlobals->logDom < 0)
  {
    ourGlobals->logDom = eina_log_domain_register("GuiLua", NULL);
    if (ourGlobals->logDom < 0)
    {
      EINA_LOG_CRIT("could not register log domain 'GuiLua'");
      return;
    }
  }
  eina_log_level_set(EINA_LOG_LEVEL_DBG);
  eina_log_domain_level_set("GuiLua", EINA_LOG_LEVEL_DBG);
  eina_log_print_cb_set(_ggg_log_print_cb, stderr);
}

char *getDateTime(struct tm **nowOut, char *dateOut, time_t *timeOut)
{
  struct tm *newTime;
  time_t  szClock;
  char *date = dateTime;

  // Get time in seconds
  time(&szClock);
  // Convert time to struct tm form
  newTime = localtime(&szClock);

  if (nowOut)
    *nowOut = newTime;
  if (dateOut)
    date = dateOut;
  if (timeOut)
    *timeOut = szClock;

  // format
  strftime(date, DATE_TIME_LEN, "%d/%m/%Y %H:%M:%S\r", newTime);
  return (dateTime);
}


// These are what the various symbols are for each type -
//  int		%
//  num		#
//  str		$
//  bool	!
//  C func	&
//  table.field	@  Expects an integer and a string.
//  nil		~
//  table       {} Starts and stops filling up a new table.
//              (  Just syntax sugar for call.
//  call        )  Expects an integer, the number of results left after the call.
// FIXME: Still to do, if we ever use them -
//  userdata	+
//  lightuserdata	*
//  thread	^

static char *_push_name(lua_State *L, char *q, int *idx)  // Stack usage [-0, +1, e or m]
{
  char *p = q;
  char temp = '\0';

  // A simplistic scan through an identifier, it's wrong, but it's quick,
  // and we don't mind that it's wrong, coz this is only internal.
  while (isalnum((int)*q))
    q++;
  temp = *q;
  *q = '\0';
  if (*idx > 0)
    lua_getfield(L, *idx, p);    // Stack usage [-0, +1, e]
  else
  {
    if (p != q)
      lua_pushstring(L, p);       // Stack usage [-0, +1, m]
    else
    {
      lua_pushnumber(L, (lua_Number) (0 - (*idx)));
      (*idx)--;
    }
  }
  *q = temp;

  return q;
}

int pull_lua(lua_State *L, int i, char *params, ...)         // Stack usage -
                                                             // if i is a table
                                                             //   [-n, +n, e]
                                                             // else
                                                             //   [-0, +0, -]
{
   va_list vl;
   char *f = strdup(params);
   char *p = f;
   int n = 0, j = i, count = 0;
   Eina_Bool table = EINA_FALSE;

   if (!f) return -1;
   va_start(vl, params);

   if (lua_istable(L, i))                                                // Stack usage [-0, +0, -]
     {
        j = -1;
        table = EINA_TRUE;
     }

   while (*p)
     {
        char *q;
        Eina_Bool get = EINA_TRUE;

        while (isspace((int)*p))
           p++;
        q = p + 1;
        switch (*p)
          {
             case '%':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_isnumber(L, j))                                // Stack usage [-0, +0, -]
                    {
                       int *v = va_arg(vl, int *);
                       *v = lua_tointeger(L, j);                         // Stack usage [-0, +0, -]
                       n++;
                    }
                  break;
               }
             case '#':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_isnumber(L, j))                                // Stack usage [-0, +0, -]
                    {
                       double *v = va_arg(vl, double *);
                       *v = lua_tonumber(L, j);                          // Stack usage [-0, +0, -]
                       n++;
                    }
                  break;
               }
             case '$':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_isstring(L, j))                                // Stack usage [-0, +0, -]
                    {
                       char **v = va_arg(vl, char **);
                       size_t len;
                       char *temp = (char *) lua_tolstring(L, j, &len);  // Stack usage [-0, +0, m]

                       len++;  // Cater for the null at the end.
                       *v = malloc(len);
                       if (*v)
                         {
                            memcpy(*v, temp, len);
                            n++;
                         }
                    }
                  break;
               }
             case '!':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_isboolean(L, j))                               // Stack usage [-0, +0, -]
                    {
                       int *v = va_arg(vl, int *);
                       *v = lua_toboolean(L, j);                         // Stack usage [-0, +0, -]
                       n++;
                    }
                  break;
               }
             default:
               {
                  get = EINA_FALSE;
                  break;
               }
          }

        if (get)
          {
             if (table)
               {
                  // If this is a table, then we pushed a value on the stack, pop it off.
                  lua_pop(L, 1);                                         // Stack usage [-n, +0, -]
               }
            else
                j++;
            count++;
          }
        p = q;
     }

   va_end(vl);
   free(f);
   if (count > n)
      n = 0;
   else if (table)
     n = 1;
   return n;
}

int push_lua(lua_State *L, char *params, ...)       // Stack usage [-0, +n, em]
{
  va_list vl;
  char *f = strdup(params);
  char *p = f;
  int n = 0, table = 0, i = -1;

  if (!f) return -1;

  va_start(vl, params);

  while (*p)
  {
    char *q;
    Eina_Bool set = EINA_TRUE;

    while (isspace((int)*p))
      p++;
    q = p + 1;
    switch (*p)
    {
      case '%':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushinteger(L, va_arg(vl, int));    // Stack usage [-0, +1, -]
        break;
      }
      case '#':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushnumber(L, va_arg(vl, double));  // Stack usage [-0, +1, -]
        break;
      }
      case '$':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushstring(L, va_arg(vl, char *));  // Stack usage [-0, +1, m]
        break;
      }
      case '!':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushboolean(L, va_arg(vl, int));    // Stack usage [-0, +1, -]
        break;
      }
      case '@':
      {
        int   tabl = va_arg(vl, int);
        char *field = va_arg(vl, char *);

        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_getfield(L, tabl, field);           // Stack usage [-0, +1, e]
        break;
      }
      case '&':
      {
        if (table)  q = _push_name(L, q, &i);     // Stack usage [-0, +1, m]
        lua_pushcfunction(L, va_arg(vl, void *)); // Stack usage [-0, +1, m]
        break;
      }
      case '~':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushnil(L);                         // Stack usage [-0, +1, -]
        break;
      }
      case '(':		// Just syntax sugar.
      {
        set = EINA_FALSE;
        break;
      }
      case ')':
      {
        lua_call(L, n - 1, va_arg(vl, int));
        n = 0;
        set = EINA_FALSE;
        break;
      }
      case '{':
      {
        lua_newtable(L);
        table++;
        n++;
        set = EINA_FALSE;
        break;
      }
      case '}':
      {
        table--;
        set = EINA_FALSE;
        break;
      }
      default:
      {
        set = EINA_FALSE;
        break;
      }
    }

    if (set)
    {
      if (table > 0)
        lua_settable(L, -3);                         // Stack usage [-2, +0, e]
      else
        n++;
    }
    p = q;
  }

  va_end(vl);
  free(f);
  return n;
}


static void _on_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
//  globals *ourGlobals = data;

  elm_exit();
}

static int window(lua_State *L)
{
  globals *ourGlobals;
  char *title = NULL;
  int w = WIDTH, h = HEIGHT;

  lua_getfield(L, LUA_REGISTRYINDEX, "ourGlobals");
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  loggingStartup(ourGlobals);
  PI("GuiLua running as an application.\n");
  if (pull_lua(L, 1, "%w %h $title", &w, &h, &title) > 0)
    PI("Setting window to %d %d %s", w, h, title);
  else
    title = "GuiLua test harness";

  if ((ourGlobals->win = elm_win_util_standard_add("GuiLua", title)))
  {
    evas_object_smart_callback_add(ourGlobals->win, "delete,request", _on_done, ourGlobals);
    evas_object_resize(ourGlobals->win, w, h);
    evas_object_move(ourGlobals->win, 0, 0);
    evas_object_show(ourGlobals->win);
  }

  return 0;
}

static int loopWindow(lua_State *L)
{
  globals *ourGlobals;

  lua_getfield(L, LUA_REGISTRYINDEX, "ourGlobals");
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (ourGlobals->win)
    elm_run();

  return 0;
}

static int closeWindow(lua_State *L)
{
  globals *ourGlobals;

  lua_getfield(L, LUA_REGISTRYINDEX, "ourGlobals");
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (ourGlobals->win)
    evas_object_del(ourGlobals->win);

  if (ourGlobals->logDom >= 0)
  {
    eina_log_domain_unregister(ourGlobals->logDom);
    ourGlobals->logDom = -1;
  }

  elm_shutdown();

  return 0;
}

/* local widget = require 'widget'

Lua's require() function will strip any stuff from the front of the name
separated by a hyphen, so 'ClientHamr-GuiLua-test_c' -> 'test_c'.  Then
it will search through a path, and eventually find this test_c.so (or
test_c.dll or whatever), then call luaopen_test_c(), which should return
a table.  The argument (only thing on the stack) for this function will
be 'test_c'.

Normally luaL_register() creates a table of functions, that is the table
returned, but we want to do something different with skang.
*/
int luaopen_widget(lua_State *L)
{
  // In theory, the only thing on the stack now is 'widget' from the require() call.

// pseudo-indices, special tables that can be accessed like the stack -
//    LUA_GLOBALSINDEX - thread environment, where globals are
//    LUA_ENVIRONINDEX - C function environment, in this case luaopen_test_c() is the C function
//    LUA_REGISTRYINDEX - C registry, global, for unique keys use the module name as a string, or a lightuserdata address to a C object in our module.
//    lua_upvalueindex(n) - C function upvalues

// The only locals we care about are skang and _M.
// All modules go into package.loaded[name] as well.
// skang is essentially a global anyway.
// _M we pass back as the result, and our functions get added to it by skang.thingasm()
//   Not entirely true, _M is a proxy table, getmetatable(_M).__values[cfunc] would be our function.

// local skang = require 'skang'
  lua_getglobal(L, "require");
  lua_pushstring(L, "skang");
  lua_call(L, 1, 1);
  skang = lua_gettop(L);
  lua_setfield(L, LUA_REGISTRYINDEX, "skang");
  lua_getfield(L, LUA_REGISTRYINDEX, "skang");

// local _M = skang.moduleBegin('widget', nil, 'Copyright 2014 David Seikel', '0.1', '2014-04-08 00:42:00', nil, false)
  push_lua(L, "@ ( $ ~ $ $ $ ~ ! )", skang, "moduleBegin", ourName, "Copyright 2014 David Seikel", "0.1", "2014-04-08 00:42:00", 0, 1);
  _M = lua_gettop(L);
  // Save this module in the C registry.
  lua_setfield(L, LUA_REGISTRYINDEX, ourName);

  // Define our functions.
  push_lua(L, "@ ( @ $ $ & $ )", skang, "thingasm", LUA_REGISTRYINDEX, ourName, "window", "Opens our window.", window, "number,number,string", 0);
  push_lua(L, "@ ( @ $ $ & )", skang, "thingasm", LUA_REGISTRYINDEX, ourName, "loopWindow", "Run our windows main loop.", loopWindow, 0);
  push_lua(L, "@ ( @ $ $ & )", skang, "thingasm", LUA_REGISTRYINDEX, ourName, "closeWindow", "Closes our window.", closeWindow, 0);

  // A test of the array building stuff.
  push_lua(L, "@ ( { @ $ $ % $widget !required } )", skang, "thingasm", LUA_REGISTRYINDEX, ourName, "wibble", "It's wibbly!", 1, "'edit', 'The wibblinator:', 1, 1, 10, 50", 1, 0);

  push_lua(L, "@ ( @ )", skang, "moduleEnd", LUA_REGISTRYINDEX, ourName, 0);

  return 1;
}

void GuiLuaDo(int argc, char **argv)
{
  lua_Number i;

  memset(&ourGlobals, 0, sizeof(globals));

  ourGlobals.L = luaL_newstate();
  if (ourGlobals.L)
  {
    luaL_openlibs(ourGlobals.L);
    // Shove ourGlobals into the registry.
    lua_pushlightuserdata(ourGlobals.L, &ourGlobals);
    lua_setfield(ourGlobals.L, LUA_REGISTRYINDEX, "ourGlobals");

    // luaopen_widget() expects a string argument, and returns a table.
    // Though in this case, both get ignored anyway.
    lua_pushstring(ourGlobals.L, "widget");
    lua_pop(ourGlobals.L, luaopen_widget(ourGlobals.L) + 1);

    // Pass all our command line arguments to skang.
    i = 1;
    push_lua(ourGlobals.L, "@ ( @", LUA_REGISTRYINDEX, "skang", 1, "scanArguments");
    lua_newtable(ourGlobals.L);
    while (--argc > 0 && *++argv != '\0')
    {
      lua_pushnumber(ourGlobals.L, i++);
      lua_pushstring(ourGlobals.L, *argv);
      lua_settable(ourGlobals.L, -3);
    }
    lua_call(ourGlobals.L, 1, 0);
    push_lua(ourGlobals.L, "@ ( @ )", 1, "pullArguments", LUA_REGISTRYINDEX, "skang", 0);

    // Run the main loop via a Lua call.
    lua_pop(ourGlobals.L, loopWindow(ourGlobals.L));

    lua_pop(ourGlobals.L, closeWindow(ourGlobals.L));
    lua_close(ourGlobals.L);
  }
  else
    fprintf(stderr, "Failed to start Lua!\n");
}
