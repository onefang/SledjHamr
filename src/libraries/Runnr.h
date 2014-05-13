#ifndef _RUNNR_H_
#define _RUNNR_H_

#include <ctype.h>

#include <Eina.h>
#include <Ecore_Con.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>


void dumpStack(lua_State *L, int i);
int pull_lua(lua_State *L, int i, char *params, ...);
int push_lua(lua_State *L, char *params, ...);

void sendBack(Ecore_Con_Client *client, const char *SID, const char *message, ...);
void sendForth(Ecore_Con_Server *server, const char *SID, const char *message, ...);

#endif
