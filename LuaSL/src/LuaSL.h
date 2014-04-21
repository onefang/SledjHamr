#ifdef HAVE_CONFIG_H
#include "config.h"
#else
//#define PACKAGE_EXAMPLES_DIR "."
#define __UNUSED__
#endif

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>
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


#define WIDTH  (1024)
#define HEIGHT (768)


#define TABLE_WIDTH	7
#define TABLE_HEIGHT	42


struct _gameGlobals
{
    Ecore_Evas		*ee;		// Our window.
    Evas		*canvas;	// The canvas for drawing directly onto.
    Evas_Object		*bg;		// Our background edje, also the game specific stuff.
    Evas_Object		*edje;		// The edje of the background.
    Ecore_Con_Server	*server;
    Eina_Hash		*scripts, *names;
    int			logDom;
    const char		*address;
    int			port;
    boolean		ui;		// Wether we actually start up the UI.
};

struct _script
{
    Eina_Clist		node;
    gameGlobals		*game;
    char		SID[PATH_MAX];
    char		fileName[PATH_MAX];
    lua_State		*L;
    struct timeval	startTime;
    float		compileTime, timerTime;
    int			bugs, warnings;
    boolean		running;
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
void sendBack(gameGlobals *ourGlobals, Ecore_Con_Client *client, const char *SID, const char *message, ...);
void sendForth(gameGlobals *ourGlobals, const char *SID, const char *message, ...);
float timeDiff(struct timeval *now, struct timeval *then);

#include "LuaSL_LSL_tree.h"
