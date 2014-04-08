/*  GuiLua - a GUI library that implements matrix-RAD style stuff.

Provides the skang and widget Lua packages.

This should be a library in the end, but for now it's just an
application that is a test bed for what goes into the library.  In the
initial intended use case, several applications will be using this all at
once, with one central app hosting all the GUIs.

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



#include <Eet.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <stdio.h>
#include <ctype.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct _globals globals;


#define WIDTH  (300)
#define HEIGHT (300)

#define PC(...) EINA_LOG_DOM_CRIT(ourGlobals->logDom, __VA_ARGS__)
#define PE(...) EINA_LOG_DOM_ERR(ourGlobals->logDom, __VA_ARGS__)
#define PW(...) EINA_LOG_DOM_WARN(ourGlobals->logDom, __VA_ARGS__)
#define PD(...) EINA_LOG_DOM_DBG(ourGlobals->logDom, __VA_ARGS__)
#define PI(...) EINA_LOG_DOM_INFO(ourGlobals->logDom, __VA_ARGS__)

#define PCm(...) EINA_LOG_DOM_CRIT(ourGlobals.logDom, __VA_ARGS__)
#define PEm(...) EINA_LOG_DOM_ERR(ourGlobals.logDom, __VA_ARGS__)
#define PWm(...) EINA_LOG_DOM_WARN(ourGlobals.logDom, __VA_ARGS__)
#define PDm(...) EINA_LOG_DOM_DBG(ourGlobals.logDom, __VA_ARGS__)
#define PIm(...) EINA_LOG_DOM_INFO(ourGlobals.logDom, __VA_ARGS__)

#define D()	PD("DEBUG")

// "01:03:52 01-01-1973\n\0"
#define DATE_TIME_LEN			21


#ifndef FALSE
// NEVER change this
typedef enum
{
  FALSE	= 0,
  TRUE	= 1
} boolean;
#endif

struct _globals
{
  Ecore_Evas	*ee;		// Our window.
  Evas		*canvas;	// The canvas for drawing directly onto.
  Evas_Object	*bg;		// Our background edje.
  lua_State	*L;		// Our Lua state.
  int		eina, logDom, ecore_evas, edje;
};

  globals ourGlobals;


static const char *ourName = "widget";
int skang, _M;

/*
static void dumpStack(lua_State *L, int i)
{
  int type = lua_type(L, i);

  switch (type)
  {
    case LUA_TNONE		:  printf("Stack %d is empty\n", i);  break;
    case LUA_TNIL		:  printf("Stack %d is a nil\n", i);  break;
    case LUA_TBOOLEAN		:  printf("Stack %d is a boolean\n", i);  break;
    case LUA_TNUMBER		:  printf("Stack %d is a number\n", i);  break;
    case LUA_TSTRING		:  printf("Stack %d is a string - %s\n", i, lua_tostring(L, i));  break;
    case LUA_TFUNCTION		:  printf("Stack %d is a function\n", i);  break;
    case LUA_TTHREAD		:  printf("Stack %d is a thread\n", i);  break;
    case LUA_TTABLE		:  printf("Stack %d is a table\n", i);  break;
    case LUA_TUSERDATA		:  printf("Stack %d is a userdata\n", i);  break;
    case LUA_TLIGHTUSERDATA	:  printf("Stack %d is a light userdata\n", i);  break;
    default			:  printf("Stack %d is unknown\n", i);  break;
  }
}
*/



char *getDateTime(struct tm **nowOut, char *dateOut, time_t *tiemOut);

#    define DATE_TIME_LEN	21

char dateTime[DATE_TIME_LEN];

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

  // Shut up the excess debugging shit from EFL.
  eina_log_domain_level_set("eo", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("eet", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_audio", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_evas", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_input_evas", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_system_upower", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_x", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("evas_main", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("eldbus", EINA_LOG_LEVEL_WARN);
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



static void _on_delete(Ecore_Evas *ee /*__UNUSED__*/)
{
  ecore_main_loop_quit();
}


static int openWindow(lua_State *L)
{
  globals *ourGlobals;

  lua_getfield(L, LUA_REGISTRYINDEX, "ourGlobals");
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  if ((ourGlobals->eina = eina_init()))
  {
    loggingStartup(ourGlobals);

    PI("GuiLua running as an application.\n");

    if ((ourGlobals->ecore_evas = ecore_evas_init()))
    {
      if ((ourGlobals->edje = edje_init()))
      {
        /* this will give you a window with an Evas canvas under the first engine available */
        ourGlobals->ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
        if (ourGlobals->ee)
        {
	  ourGlobals->canvas = ecore_evas_get(ourGlobals->ee);
	  ecore_evas_title_set(ourGlobals->ee, "GuiLua test harness");
	  ecore_evas_show(ourGlobals->ee);
	  ecore_evas_callback_delete_request_set(ourGlobals->ee, _on_delete);
        }
        else
	  PC("You got to have at least one evas engine built and linked up to ecore-evas for this to run properly.");
      }
      else
	PC("Failed to init edje!");
    }
    else
      PC("Failed to init ecore_evas!");
  }
  else
    fprintf(stderr, "Failed to init eina!\n");

  return 0;
}

static int loopWindow(lua_State *L)
{
  globals *ourGlobals;

  lua_getfield(L, LUA_REGISTRYINDEX, "ourGlobals");
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (ourGlobals->canvas)
    ecore_main_loop_begin();

  return 0;
}

static int closeWindow(lua_State *L)
{
  globals *ourGlobals;

  lua_getfield(L, LUA_REGISTRYINDEX, "ourGlobals");
  ourGlobals = lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (ourGlobals->eina)
  {
    if (ourGlobals->ecore_evas)
    {
      if (ourGlobals->edje)
      {
        if (ourGlobals->ee)
        {
          ecore_evas_free(ourGlobals->ee);
          ourGlobals->ee = NULL;
        }
        ourGlobals->edje = edje_shutdown();
      }
      ourGlobals->ecore_evas = ecore_evas_shutdown();
    }
    if (ourGlobals->logDom >= 0)
    {
      eina_log_domain_unregister(ourGlobals->logDom);
      ourGlobals->logDom = -1;
    }
    ourGlobals->eina = eina_shutdown();
  }

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
  lua_getfield(L, skang, "moduleBegin");
  lua_pushstring(L, ourName);
  lua_pushnil(L);				// Author comes from copyright.
  lua_pushstring(L, "Copyright 2014 David Seikel");
  lua_pushstring(L, "0.1");
  lua_pushstring(L, "2014-04-08 00:42:00");
  lua_pushnil(L);				// No skin.
  lua_pushboolean(L, 0);			// We are not a Lua module.
  lua_call(L, 7, 1);				// call 'skang.moduleBegin' with 7 arguments and 1 result.

  _M = lua_gettop(L);
  // Save this module in the C registry.
  lua_setfield(L, LUA_REGISTRYINDEX, ourName);

  // Define our functions.
  lua_getfield(L, skang, "thingasm");
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);	// Coz getfenv() can't find C environment.
  lua_pushstring(L, "openWindow");
  lua_pushstring(L, "Opens our window.");
  lua_pushcfunction(L, openWindow);
  lua_call(L, 4, 0);

  lua_getfield(L, skang, "thingasm");
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);	// Coz getfenv() can't find C environment.
  lua_pushstring(L, "loopWindow");
  lua_pushstring(L, "Run our windows main loop.");
  lua_pushcfunction(L, loopWindow);
  lua_call(L, 4, 0);

  lua_getfield(L, skang, "thingasm");
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);	// Coz getfenv() can't find C environment.
  lua_pushstring(L, "closeWindow");
  lua_pushstring(L, "Closes our window.");
  lua_pushcfunction(L, closeWindow);
  lua_call(L, 4, 0);

  lua_pop(L, openWindow(L));

// skang.moduleEnd(_M)
  lua_getfield(L, skang, "moduleEnd");
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);
  lua_call(L, 1, 0);

  return 1;
}


int main(int argc, char **argv)
{
  int result = EXIT_FAILURE;
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
    lua_getfield(ourGlobals.L, LUA_REGISTRYINDEX, "skang");
    lua_getfield(ourGlobals.L, 1, "scanArguments");
    lua_newtable(ourGlobals.L);
    while (--argc > 0 && *++argv != '\0')
    {
      lua_pushnumber(ourGlobals.L, i++);
      lua_pushstring(ourGlobals.L, *argv);
      lua_settable(ourGlobals.L, -3);
    }
    lua_call(ourGlobals.L, 1, 0);
    lua_getfield(ourGlobals.L, 1, "pullArguments");
    lua_getfield(ourGlobals.L, LUA_REGISTRYINDEX, "skang");
    lua_call(ourGlobals.L, 1, 0);

    // Run the main loop via a Lua call.
    lua_pop(ourGlobals.L, loopWindow(ourGlobals.L));

    lua_pop(ourGlobals.L, closeWindow(ourGlobals.L));
    lua_close(ourGlobals.L);
  }
  else
    fprintf(stderr, "Failed to start Lua!\n");

  return result;
}
