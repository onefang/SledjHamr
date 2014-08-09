#ifndef _RUNNR_H_
#define _RUNNR_H_

#include <ctype.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

// Stick with Plan C for now.
// TODO - Should make this choosable at run time after more testing of Ecore_Thead.
#define  THREADIT   0

typedef enum
{
  RUNNR_COMPILING,
  RUNNR_NOT_STARTED,
  RUNNR_RUNNING,
  RUNNR_WAIT,
  RUNNR_READY,
  RUNNR_RESET,
  RUNNR_FINISHED
} runnrStatus;

typedef struct _LuaCompile LuaCompile;
typedef void (* compileCb)(LuaCompile *compiler);

typedef struct _LuaCompile
{
  char			*file, *SID, *luaName;
  int			bugCount;
  void			*data;
  Ecore_Con_Client	*client;
  compileCb		parser;
  compileCb		cb;
} LuaCompiler;

typedef struct _script script;
typedef void (* RunnrServerCb)(script *me, const char *message);

typedef struct _script
{
  Eina_Clist		node;
#if THREADIT
  Eina_Lock		mutex;
  Ecore_Thread		*me;
#endif
  void			*data;
  char			SID[PATH_MAX];
  char			*name;
  char			fileName[PATH_MAX];
  char			binName[PATH_MAX];
  lua_State		*L;
  struct timeval	startTime;
  float			timerTime;
  runnrStatus		status;
  RunnrServerCb		send2server;
  Eina_Clist		messages;
  Ecore_Con_Client	*client;
  Ecore_Timer		*timer;
} script;

typedef struct
{
  Eina_Clist	node;
  script	*s;
  const char	message[PATH_MAX];
} scriptMessage;


script *scriptAdd(char *file, char *SID, RunnrServerCb send2server, void *data);
void compileScript(LuaCompiler *compiler, int threadIt);
void runScript(script *me);
void resetScript(script *me);
script *getScript(char *SID);
void takeScript(script *me);
void releaseScript(script *me);
void send2script(const char *SID, const char *message);
void printScriptsStatus();

void dumpStack(lua_State *L, int i);
void doLuaString(lua_State *L, char *string, char *module);
int pull_lua(lua_State *L, int i, char *params, ...);
int push_lua(lua_State *L, char *params, ...);

#endif
