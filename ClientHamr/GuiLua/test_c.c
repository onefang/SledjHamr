/* Should be a Lua skang module, roughly the same as test.lua


Seems to be several problems with linking in various OSes, heres some
possibly helpful links -

http://lua.2524044.n2.nabble.com/C-Lua-modules-not-compatible-with-every-Lua-interpreter-td7647522.html
http://lua-users.org/wiki/LuaProxyDllFour
http://stackoverflow.com/questions/11492194/how-do-you-create-a-lua-plugin-that-calls-the-c-lua-api?rq=1
http://lua-users.org/lists/lua-l/2008-01/msg00671.html



*/


#include <lua.h>
#include <lauxlib.h>
//#include <lualib.h>


static int ffunc (lua_State *L)
{
  double arg1 = luaL_checknumber(L, 1);
  const char *arg2 = luaL_checkstring(L, 2);

  printf("Inside test_c.ffunc(%f, %s)\n", arg1, arg2);
  return 0;
}


static const struct luaL_reg test_c [] =
{
  {"ffunc", ffunc},
  {NULL, NULL}
};


/* local test_c = require 'test_c'

Lua's require() function will strip any stuff from the front of the name
separated by a hypen, so 'GuiLua-test_c' -> 'test_c'.  Then it will
search through a path, and eventually find this test_c.so (or test_c.dll
or whatever), then call luaopen_test_c(), which should return a table.

Normally luaL_register() creates a table of functions, that is the table
returned, but we want to do something different with skang.

*/
int luaopen_test_c(lua_State *L)
{
// This is a moving target, old ways get deperecated, new ways get added, 
// would have to check version before doing any of these.
//  luaL_openlib(L, "test_c", test_c, 0);		// Lua 5.0 way.
//  luaL_register (L, "test_c", test_c);		// Lua 5.1 way.
//  luaL_newlib() or luaL_setfuncs()			// Lua 5.2 way.
    // Creates a global table "test_c", does the package.loaded[test_c] thing.
  lua_newtable(L);
  luaL_register (L, NULL, test_c);		// Lua 5.1 way.
  // Puts the funcions in a table on top of the stack.

/* BUT REALLY ...

We are in fact NOT putting any functions into the returned table.

skang.moduleBegin() returns the table we need to send back to Lua.
    it saves getfenv(2) as the old environment, which should in theory be L
    and setfenv(_M, 2) to set the tbale to be it's own environment
    it does the package.loaded[test_c] thing for us
    it returns the table it created, so we should just leave that on the stack as our result

skang.thing() also uses getfenv(2) to grab the module's table

*/

/* TODO - load skang, create things, etc.

local skang = require "skang"
local _M = skang.moduleBegin("test_c", nil, "Copyright 2014 David Seikel", "0.1", "2014-03-27 03:57:00")

skang.thing("fooble,f", "Help text goes here", 1, "number", "'edit', 'The fooble:', 1, 1, 10, 50", true)
skang.thing("bar", "Help text", "Default")
skang.thing("foo")
skang.thing("ffunc", "Help Text", ffunc, "number,string")

skang.moduleEnd(_M)

*/
  return 1;
}
