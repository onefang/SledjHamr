
#include <stdio.h>
#include <ctype.h>

#include <Elementary.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct _globals globals;


#define WIDTH  (300)
#define HEIGHT (300)

#define PC(...) EINA_LOG_DOM_CRIT(ourGlobals->logDom, __VA_ARGS__)
#define PE(...) EINA_LOG_DOM_ERR(ourGlobals->logDom, __VA_ARGS__)
#define PW(...) EINA_LOG_DOM_WARN(ourGlobals->logDom, __VA_ARGS__)
#define PD(...) EINA_LOG_DOM_DBG(ourGlobals->logDom, __VA_ARGS__)
#define PI(...) EINA_LOG_DOM_INFO(ourGlobals->logDom, __VA_ARGS__)

#define PCm(...) EINA_LOG_DOM_CRIT(ourGlobals.logDom, __VA_ARGS__)
#define PEm(...) EINA_LOG_DOM_ERR(ourGlobals.logDom, __VA_ARGS__)
#define PWm(...) EINA_LOG_DOM_WARN(ourGlobals.logDom, __VA_ARGS__)
#define PDm(...) EINA_LOG_DOM_DBG(ourGlobals.logDom, __VA_ARGS__)
#define PIm(...) EINA_LOG_DOM_INFO(ourGlobals.logDom, __VA_ARGS__)

#define D()	PD("DEBUG")

// "01:03:52 01-01-1973\n\0"
#define DATE_TIME_LEN			21
#    define DATE_TIME_LEN	21


#ifndef FALSE
// NEVER change this
typedef enum
{
  FALSE	= 0,
  TRUE	= 1
} boolean;
#endif

struct _globals
{
  Evas_Object	*win;		// Our Elm window.
  lua_State	*L;		// Our Lua state.
  int		logDom;		// Our logging domain.
};


void dumpStack(lua_State *L, int i);

void loggingStartup(globals *ourGlobals);
char *getDateTime(struct tm **nowOut, char *dateOut, time_t *tiemOut);

int pull_lua(lua_State *L, int i, char *params, ...);
int push_lua(lua_State *L, char *params, ...);

int luaopen_widget(lua_State *L);
void GuiLuaDo(int argc, char **argv);
