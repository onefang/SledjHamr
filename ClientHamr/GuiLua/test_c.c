/* Should be a Lua skang module, roughly the same as test.lua

TODO - see if this still works if it's an app instead of a library.


Seems to be several problems with linking in various OSes, here's some
possibly helpful links -

http://lua.2524044.n2.nabble.com/C-Lua-modules-not-compatible-with-every-Lua-interpreter-td7647522.html
http://lua-users.org/wiki/LuaProxyDllFour
http://stackoverflow.com/questions/11492194/how-do-you-create-a-lua-plugin-that-calls-the-c-lua-api?rq=1
http://lua-users.org/lists/lua-l/2008-01/msg00671.html
*/


#include <lua.h>
#include <lauxlib.h>


static const char *ourName = "test_c";
int skang, _M;

static int cfunc (lua_State *L)
{
  double arg1 = luaL_checknumber(L, 1);
  const char *arg2 = luaL_checkstring(L, 2);

  printf("Inside %s.cfunc(%f, %s)\n", ourName, arg1, arg2);
  return 0;
}


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

/* local test_c = require 'test_c'

Lua's require() function will strip any stuff from the front of the name
separated by a hyphen, so 'ClientHamr-GuiLua-test_c' -> 'test_c'.  Then
it will search through a path, and eventually find this test_c.so (or
test_c.dll or whatever), then call luaopen_test_c(), which should return
a table.  The argument (only thing on the stack) for this function will
be 'test_c'.

Normally luaL_register() creates a table of functions, that is the table
returned, but we want to do something different with skang.
*/
int luaopen_test_c(lua_State *L)
{
  lua_Number i;

  // In theory, the only thing on the stack now is 'test_c' from the require() call.

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
//  dumpStack(L, skang);

// local _M = skang.moduleBegin('test_c', nil, 'Copyright 2014 David Seikel', '0.1', '2014-03-27 03:57:00', nil, false)
  lua_getfield(L, skang, "moduleBegin");
  lua_pushstring(L, ourName);
  lua_pushnil(L);				// Author comes from copyright.
  lua_pushstring(L, "Copyright 2014 David Seikel");
  lua_pushstring(L, "0.1");
  lua_pushstring(L, "2014-03-27 03:57:00");
  lua_pushnil(L);				// No skin.
  lua_pushboolean(L, 0);			// We are not a Lua module.
  lua_call(L, 7, 1);				// call 'skang.moduleBegin' with 7 arguments and 1 result.
  _M = lua_gettop(L);
//  dumpStack(L, _M);

  // At this point the stack should be - 'test_c', skang, _M.  Let's test that.
/*
  int top = 0, i;

  top = lua_gettop(L);
  printf("MODULE test_c has %d stack items.\n", top);
  for (i = 1; i <= top; i++)
    dumpStack(L, i);
*/

  // Save this module in the C registry.
  lua_setfield(L, LUA_REGISTRYINDEX, ourName);

// TODO - This is too verbose.  I've had an idea for writing some sort of generic wrapper, though others have done the same.
//          http://www.lua.org/pil/25.3.html seems the most reasonable of the examples I've found.

// This uses function{} style.
// skang.thingasm{_M, 'cfooble,c', 'cfooble help text', 1, widget=\"'edit', 'The cfooble:', 1, 1, 10, 50\", required=true}
  lua_getfield(L, skang, "thingasm");
  i = 1;
  lua_newtable(L);
  lua_pushnumber(L, i++);
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);	// Coz getfenv() can't find C environment.
  lua_settable(L, -3);

  lua_pushnumber(L, i++);
  lua_pushstring(L, "cfooble,c");
  lua_settable(L, -3);

  lua_pushnumber(L, i++);
  lua_pushstring(L, "cfooble help text");
  lua_settable(L, -3);

  lua_pushnumber(L, i++);
  lua_pushnumber(L, 1);
  lua_settable(L, -3);

  lua_pushstring(L, "'edit', 'The cfooble:', 1, 1, 10, 50");
  lua_setfield(L, -2, "widget");
  lua_pushboolean(L, 1);			// Is required.
  lua_setfield(L, -2, "required");
  lua_call(L, 1, 0);

// skang.thing(_M, 'cbar', 'Help text', 'Default')
  lua_getfield(L, skang, "thingasm");
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);	// Coz getfenv() can't find C environment.
  lua_pushstring(L, "cbar");
  lua_pushstring(L, "Help text");
  lua_pushstring(L, "Default");
  lua_call(L, 4, 0);

// skang.thingasm(_M, 'cfoo')
  lua_getfield(L, skang, "thingasm");
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);	// Coz getfenv() can't find C environment.
  lua_pushstring(L, "cfoo");
  lua_call(L, 2, 0);

// skang.thingasm(_M, 'cfunc', 'cfunc does nothing really', cfunc, 'number,string')
  lua_getfield(L, skang, "thingasm");
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);	// Coz getfenv() can't find C environment.
  lua_pushstring(L, "cfunc");
  lua_pushstring(L, "cfunc does nothing really");
  lua_pushcfunction(L, cfunc);
  lua_pushstring(L, "number,string");
  lua_call(L, 5, 0);

// skang.moduleEnd(_M)
  lua_getfield(L, skang, "moduleEnd");
  lua_getfield(L, LUA_REGISTRYINDEX, ourName);
  lua_call(L, 1, 1);

  return 1;
}
