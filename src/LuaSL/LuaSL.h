#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define __UNUSED__
#endif

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_File.h>
#include <stdio.h>
#include <ctype.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct _gameGlobals gameGlobals;	// Define this here, so LuaSL_threads.h can use it.

#include "LumbrJack.h"
#include "Runnr.h"


struct _gameGlobals
{
    Ecore_Con_Server	*server;
    Eina_Hash		*names;
    const char		*address;
    int			port;
};


#include "LuaSL_LSL_tree.h"
