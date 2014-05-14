#ifdef HAVE_CONFIG_H
#include "config.h"
#else
//#define PACKAGE_EXAMPLES_DIR "."
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

typedef struct _script script;			// Define this here, so LuaSL_threads.h can use it.
typedef struct _gameGlobals gameGlobals;	// Define this here, so LuaSL_threads.h can use it.

#include "LuaSL_threads.h"
#include "LumbrJack.h"


struct _gameGlobals
{
    Ecore_Con_Server	*server;
    Eina_Hash		*scripts, *names;
    const char		*address;
    int			port;
};

struct _script
{
    Eina_Clist		node;
    gameGlobals		*game;
    char		SID[PATH_MAX];
    char		fileName[PATH_MAX];
    char		*name;
    lua_State		*L;
    struct timeval	startTime;
    float		timerTime;
    int			status;
    int			args;
    Eina_Clist		messages;
    Ecore_Con_Client	*client;
    Ecore_Timer		*timer;
};

typedef struct
{
  Eina_Clist	node;
  script	*script;
  const char	message[PATH_MAX];
} scriptMessage;


void scriptSendBack(void * data);

#include "LuaSL_LSL_tree.h"
