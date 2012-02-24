#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
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

typedef struct _script script;	// Define this here, so LuaSL_threads.h can use it.

#include "LuaSL_threads.h"


#define WIDTH  (1024)
#define HEIGHT (768)

#define PC(...) EINA_LOG_DOM_CRIT(game->logDom, __VA_ARGS__)
#define PE(...) EINA_LOG_DOM_ERR(game->logDom, __VA_ARGS__)
#define PW(...) EINA_LOG_DOM_WARN(game->logDom, __VA_ARGS__)
#define PD(...) EINA_LOG_DOM_DBG(game->logDom, __VA_ARGS__)
#define PI(...) EINA_LOG_DOM_INFO(game->logDom, __VA_ARGS__)

#define PCm(...) EINA_LOG_DOM_CRIT(game.logDom, __VA_ARGS__)
#define PEm(...) EINA_LOG_DOM_ERR(game.logDom, __VA_ARGS__)
#define PWm(...) EINA_LOG_DOM_WARN(game.logDom, __VA_ARGS__)
#define PDm(...) EINA_LOG_DOM_DBG(game.logDom, __VA_ARGS__)
#define PIm(...) EINA_LOG_DOM_INFO(game.logDom, __VA_ARGS__)

#define D()	PD("DEBUG")

// "01:03:52 01-01-1973\n\0"
#define DATE_TIME_LEN			21

#define TABLE_WIDTH	7
#define TABLE_HEIGHT	42

#ifndef FALSE
// NEVER change this
typedef enum
{
    FALSE	= 0,
    TRUE	= 1
} boolean;
#endif

typedef struct
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
} gameGlobals;

struct _script
{
    Eina_Clist		node;
    gameGlobals		*game;
    char		SID[PATH_MAX];
    char		fileName[PATH_MAX];
    lua_State		*lstate;
    struct timeval	startTime;
    float		compileTime, timerTime;
    int			bugs, warnings;
    boolean		running;
    int			status;
    int			args;
    channel		chan;
    Ecore_Con_Client	*client;
    Ecore_Timer		*timer;
};

typedef struct
{
  script	*script;
  const char	message[PATH_MAX];
} scriptMessage;


void loggingStartup(gameGlobals *game);
char *getDateTime(struct tm **nowOut, char *dateOut, time_t *tiemOut);
void scriptSendBack(void * data);
void sendBack(gameGlobals *game, Ecore_Con_Client *client, const char *SID, const char *message, ...);
void sendForth(gameGlobals *game, const char *SID, const char *message, ...);
float timeDiff(struct timeval *now, struct timeval *then);

#include "LuaSL_LSL_tree.h"
