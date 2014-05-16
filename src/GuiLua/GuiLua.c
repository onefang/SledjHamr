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
    Same if there's a module, but no skang file.

Making these packages all a sub package of skang seems like a great
idea.  On the other hand, looks like most things are just getting folded
into skang anyway.  See
http://www.inf.puc-rio.br/~roberto/pil2/chapter15.pdf part 15.5 for
package details.

See if I can use LuaJIT FFI here.  Since this will be a library, and
skang apps could be written in C or Lua, perhaps writing this library to
be FFI friendly instead of the usual Lua C binding might be the way to
go?  LuaJIT is not ready yet, since it needs include files copied into
Lua files, and does not support macros, which EFL uses a lot of.

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


#include "LumbrJack.h"
#include "GuiLua.h"


static int		logDom;		// Our logging domain.
const char	*glName = "ourGuiLua";

/* Sooo, how to do this -
widget has to be a light userdata
The rest can be Lua sub things?  Each with a C function to update the widget.

win.quitter:colour(1,2,3,4)  -> win.quitter.colour(win.quitter, 1,2,3,4)  ->  __call(win.quitter.colour, win.quitter, 1,2,3,4)  ->  skang.colour(win.quitter.colour, win.quitter, 1,2,3,4)
win.quitter.colour.r = 5     -> direct access to the table, well "direct" via Thing and Mum.  We eventually want to call skang.colour() though.
*/


// TODO - Should be able to open external and internal windows, and even switch between them on the fly.
static void _on_click(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
  Widget *wid = data;

  if (wid)
  {
    lua_State *L = wid->data;

    PD("Doing action %s", wid->action);
    if (0 != luaL_dostring(L, wid->action))
      PE("Error running - %s", wid->action);
  }
}

static int widget(lua_State *L)
{
  winFang *win = NULL;
  Widget *wid;
  char *type = "button";
  char *title = ":";
  int x = -1, y = -1, w = -1, h = -1;

  pull_lua(L, 1, "*window $type $title %x %y %w %h", &win, &type, &title, &x, &y, &w, &h);

  wid = widgetAdd(win, type, title, x, y, w, h);
  if (wid)
  {
    evas_object_smart_callback_add(wid->obj, "clicked", _on_click, wid);
    wid->data = L;

    lua_pushlightuserdata(L, (void *) wid);
    return 1;
  }

  return 0;
}

static int action(lua_State *L)
{
  Widget *wid = lua_touserdata(L, 1);
  char *action = "nada";

  pull_lua(L, 2, "$", &action);
  if (wid && strcmp(wid->magic, "Widget") == 0)
  {
PD("Setting action : %s\n", action);
    wid->action = strdup(action);
  }
  return 0;
}

static int colour(lua_State *L)
{
// TODO - This is just a stub for now.

  return 0;
}

/*  userdata vs light userdata
  Lua wants to allocate the memory for userdata itself.
  Light user data an actual pointer.
*/

static void _on_us_del(void *data, Evas_Object *obj, void *event_info)
{
  winFang *win = data;
  GuiLua *gl = win->data;

  gl->inDel = 1;
  GuiLuaDel(gl);
}

static int window(lua_State *L)
{
  winFang *win = NULL;
  winFang *parent = NULL;
  EPhysics_World *world = NULL;
  char *name = "GuiLua";
  char *title = "GuiLua test harness";
  int w = WIDTH, h = HEIGHT;
  GuiLua *gl;

  pull_lua(L, 1, "%w %h $title $name", &w, &h, &title, &name);

  lua_getfield(L, LUA_REGISTRYINDEX, glName);
  gl = lua_touserdata(L, -1);
  lua_pop(L, 1);
  if (gl)
  {
    parent = gl->parent;
    world = gl->world;
  }

  win = winFangAdd(parent, 25, 55, w, h, title, name, world);
  if (gl)
  {
    // If there's no parent, we become the parent.
    if (!parent)
      gl->parent = win;
    // If there's no us, we must be the first, so we are us.
    if (!gl->us)
    {
      gl->us = win;
      // TODO - If this invocation of GuiLuaDo never opens a window, then this GuiLua will never get deleted.
      //        Also, who ever opened this window might have other plans for on_del or data.
      win->data = gl;
      win->on_del = _on_us_del;
    }
  }
  lua_pushlightuserdata(L, win);

  return 1;
}

static int clear(lua_State *L)
{
// TODO - This is just a stub for now.

  return 0;
}

static int loopWindow(lua_State *L)
{
  elm_run();

  return 0;
}

static int quit(lua_State *L)
{
  elm_exit();

  return 0;
}

static int closeWindow(lua_State *L)
{
  winFang *win = NULL;

  pull_lua(L, 1, "*window", &win);
  if (win) winFangDel(win);

  return 0;
}

/* local widget = require 'libGuiLua'

Lua's require() function will strip any stuff from the front of the name
separated by a hyphen, so 'ClientHamr-GuiLua-libGuiLua' -> 'libGuiLua'.  Then
it will search through a path, and eventually find this libGuiLua.so (or
libGuiLua.dll or whatever), then call luaopen_libGuiLua(), which should return
a table.  The argument (only thing on the stack) for this function will
be 'libGuiLua'.

Normally luaL_register() creates a table of functions, that is the table
returned, but we want to do something different with skang.
*/
int luaopen_GuiLua(lua_State *L)
{
  int skang;

printf("**********************require GuiLua\n");
  // In theory this function only ever gets called once.
  logDom = loggingStartup("GuiLua", logDom);

  elm_policy_set(ELM_POLICY_EXIT,	ELM_POLICY_EXIT_NONE);
  elm_policy_set(ELM_POLICY_QUIT,	ELM_POLICY_QUIT_NONE);
  elm_policy_set(ELM_POLICY_THROTTLE,	ELM_POLICY_THROTTLE_HIDDEN_ALWAYS);

  // These are set via the elementary_config tool, which is hard to find.
  elm_config_finger_size_set(0);
  elm_config_scale_set(1.0);

// pseudo-indices, special tables that can be accessed like the stack -
//    LUA_GLOBALSINDEX - thread environment, where globals are
//    LUA_ENVIRONINDEX - C function environment, in this case luaopen_GuiLUa() is the C function
//    LUA_REGISTRYINDEX - C registry, global, for unique keys use the module name as a string, or a lightuserdata address to a C object in our module.
//    lua_upvalueindex(n) - C function upvalues

  // The skang module should have been loaded by now, so we can just grab it out of package.loaded[].
  lua_getglobal(L, "package");
  lua_getfield(L, lua_gettop(L), "loaded");
  lua_remove(L, -2);				// Removes "package"
  lua_getfield(L, lua_gettop(L), SKANG);
  lua_remove(L, -2);				// Removes "loaded"
  lua_setfield(L, LUA_REGISTRYINDEX, SKANG);
  lua_getfield(L, LUA_REGISTRYINDEX, SKANG);	// Puts the skang table back on the stack.
  skang = lua_gettop(L);

  // Define our functions.
//thingasm{'window', 'The size and title of the application Frame.', window, 'x,y,name', acl='GGG'}
  push_lua(L, "@ ( { = $ $ & $ $acl } )",	skang, THINGASM, skang, "Cwindow",	"Opens our window.",				window, "number,number,string", "GGG", 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "clear",	"The current skin is cleared of all widgets.",	clear, 0);
PD("GuiLua 2");
// TODO - This one crashes sometimes.  Figure out why later.
  push_lua(L, "@ ( = $ $ & $ )",		skang, THINGASM, skang, "widget",	"Create a widget.",				widget, "userdata,string,string,number,number,number,number");
///  push_lua(L, "@ ( = $ $ & )", skang, THINGASM, skang, "widget",	"Create a widget.",				widget, 0);
PD("GuiLua 3");
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "action",	"Add an action to a widget.",			action, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "Colour",	"Change widget colours.",			colour, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "loopWindow",	"Run our windows main loop.",			loopWindow, 0);
  push_lua(L, "@ ( = $ $ & )",			skang, THINGASM, skang, "quit",		"Quit, exit, remove thyself.",			quit, 0);
  push_lua(L, "@ ( = $ $ & $ )",			skang, THINGASM, skang, "closeWindow",	"Closes a window.",				closeWindow, "userdata", 0); // TODO - closeWindow, "userdata");

  // A test of the array building stuff.
  push_lua(L, "@ ( { = $ $ % $widget !required } )", skang, THINGASM, skang, "wibble", "It's wibbly!", 1, "'edit', 'The wibblinator:', 1, 1, 10, 50", 1, 0);

  // Makes no difference what we return, but it's expecting something.
  return 1;
}

GuiLua *GuiLuaDo(int argc, char **argv, winFang *parent, EPhysics_World *world)
{
  GuiLua *result;
  lua_State  *L;
  lua_Number  i;

  result = calloc(1, sizeof(GuiLua));
  result->parent = parent;
  result->world = world;

  L = luaL_newstate();
  if (L)
  {
    result->L = L;
    luaL_openlibs(L);

    // Pass all our command line arguments to Lua.
    i = 1;
    lua_newtable(L);
    while (--argc > 0 && *++argv != '\0')
    {
      lua_pushnumber(L, i++);
      lua_pushstring(L, *argv);
      lua_settable(L, -3);
    }
    lua_setfield(L, LUA_GLOBALSINDEX, "arg");

    // Shove our structure into the registry.
    lua_pushlightuserdata(L, result);
    lua_setfield(L, LUA_REGISTRYINDEX, glName);

    // When we do this, skang will process all the arguments passed to GuiLuaDo().
    // This likely includes a module load, which likely opens a window.
    lua_getglobal(L, "require");
    lua_pushstring(L, SKANG);
    lua_call(L, 1, 1);
    lua_setfield(L, LUA_GLOBALSINDEX, SKANG);

    if (!parent)
    {
      // Run the main loop via a Lua call.
      // This does nothing if no module opened a window.
      if (0 != luaL_dostring(L, "skang.loopWindow()"))
        PE("Error running - skang.loopWindow()");
      GuiLuaDel(result);
      result = NULL;
      if (logDom >= 0)
      {
	eina_log_domain_unregister(logDom);
	logDom = -1;
      }

      // This shuts down Elementary, but keeps the main loop running until all ecore_evas are freed.
      elm_shutdown();
    }
  }
  else
    fprintf(stderr, "Failed to start Lua!\n");

  return result;
}

GuiLua *GuiLuaLoad(char *module, winFang *parent, EPhysics_World *world)
{
  GuiLua *result;
  char *args[] = {"GuiLUa", "-l", ""};

  args[2] = module;
  result = GuiLuaDo(3, args, parent, world);
  result->name = module;
  return result;
}

void GuiLuaDel(GuiLua *gl)
{
  if (gl)
  {
    gl->us->on_del = NULL;
    if (!gl->inDel)  winFangDel(gl->us);
    if (gl->L) lua_close(gl->L);
    free(gl);
  }
}
