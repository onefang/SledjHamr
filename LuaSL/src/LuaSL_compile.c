#include "LuaSL.h"


Eina_Bool compileLSL(gameGlobals *game, char *script)
{
    Eina_Bool result = EINA_FALSE;

// Parse the  LSL script, validating it and reporting errors.

// Take the result of the parse, and convert it into Lua source.
//   Each LSL script becomes a Lua state.
//   LSL states are handled as Lua tables, with each LSL state function being a table function in a common metatable.
//   LL and OS functions are likely to be C functions. 

// Compile the Lua source by the Lua compiler.

    return result;
}

