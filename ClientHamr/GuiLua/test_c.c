/* Should be a Lua module, roughly the same as test.lua

*/



/* NOTES -

From http://www.inf.puc-rio.br/~roberto/pil2/chapter15.pdf

"Well-behaved C libraries should export one function called
luaopen_modname, which is the function that require tries to call after
linking the library.  In Section 26.2 we will discuss how to write C
libraries."

The "modname" bit is replaced by the name of the module.  Though if the
module name includes a hyphen, the "require" function strips out the
hyphen and the bit before it.

Though it seems that chapter 26 is not in the same place?

http://www.lua.org/pil/26.2.html  doesn't say much really, and is for
Lua 5.0



An example -

// build@ gcc -shared -I/home/sdonovan/lua/include -o mylib.so mylib.c
// includes for your code
#include <string.h>
#include <math.h>

// includes for Lua
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// defining functions callable from Lua
static int l_createtable (lua_State *L) {
  int narr = luaL_optint(L,1,0);         // initial array slots, default 0
  int nrec = luaL_optint(L,2,0);   // intialof hash slots, default 0
  lua_createtable(L,narr,nrec);
  return 1;
}

static int l_solve (lua_State *L) {
    double a = lua_tonumber(L,1);  // coeff of x*x
    double b = lua_tonumber(L,2);  // coef of x
    double c = lua_tonumber(L,3);  // constant
    double abc = b*b - 4*a*c;
    if (abc < 0.0) {
        lua_pushnil(L);
        lua_pushstring(L,"imaginary roots!");
        return 2;
    } else {
        abc = sqrt(abc);
        a = 2*a;
        lua_pushnumber(L,(-b + abc)/a);
        lua_pushnumber(L,(+b - abc)/a);
        return 2;
    }
}

static const luaL_reg mylib[] = {
    {"createtable",l_createtable},
    {"solve",l_solve},
    {NULL,NULL}
};

int luaopen_mylib(lua_State *L)
{
    luaL_register (L, "mylib", mylib);
    return 1;
}


*/
