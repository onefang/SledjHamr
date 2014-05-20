/* Should be a Lua skang module, roughly the same as test.lua

Seems to be several problems with linking in various OSes, here's some
possibly helpful links -

http://lua.2524044.n2.nabble.com/C-Lua-modules-not-compatible-with-every-Lua-interpreter-td7647522.html
http://lua-users.org/wiki/LuaProxyDllFour
http://stackoverflow.com/questions/11492194/how-do-you-create-a-lua-plugin-that-calls-the-c-lua-api?rq=1
http://lua-users.org/lists/lua-l/2008-01/msg00671.html
*/


#include "Runnr.h"
#include "GuiLua.h"


static const char *ourName = "test_c";
int skang, _M;

static int cfunc (lua_State *L)
{
  double arg1 = luaL_checknumber(L, 1);
  const char *arg2 = luaL_checkstring(L, 2);

  PI("Inside %s.cfunc(%f, %s)\n", ourName, arg1, arg2);
  return 0;
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
  lua_pushstring(L, SKANG);
  lua_call(L, 1, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, SKANG);
  lua_getfield(L, LUA_REGISTRYINDEX, SKANG);
  skang = lua_gettop(L);

// local _M = skang.moduleBegin('test_c', nil, 'Copyright 2014 David Seikel', '0.1', '2014-03-27 03:57:00', nil, false)
  push_lua(L, "@ ( $ ~ $ $ $ ~ ! )", skang, MODULEBEGIN, ourName, "Copyright 2014 David Seikel", "0.1", "2014-03-27 03:57:00", 0, 1);
  _M = lua_gettop(L);

// This uses function{} style.
// skang.thingasm{_M, 'cfooble,c', 'cfooble help text', 1, widget=\"'edit', 'The cfooble:', 1, 1, 10, 50\", required=true}
  push_lua(L, "@ ( { = $ $ % $widget !required } )", skang, THINGASM, _M, "cfooble,c", "cfooble help text", 1, "'edit', 'The cfooble:', 1, 1, 10, 50", 1, 0);

// skang.thing(_M, 'cbar', 'Help text', 'Default')
  push_lua(L, "@ ( = $ $ $ )", skang, THINGASM, _M, "cbar", "Help text", "Default", 0);

// skang.thingasm(_M, 'cfoo')
  push_lua(L, "@ ( = $ )", skang, THINGASM, _M, "cfoo", 0);

// skang.thingasm(_M, 'cfunc', 'cfunc does nothing really', cfunc, 'number,string')
 push_lua(L, "@ ( = $ $ & $ )", skang, THINGASM, _M, "cfunc", "cfunc does nothing really", cfunc, "number,string", 0);

// skang.moduleEnd(_M)
  push_lua(L, "@ ( = )", skang, MODULEEND, _M, 0);

  // Return _M, the table itself, not the index.
  return 1;
}
