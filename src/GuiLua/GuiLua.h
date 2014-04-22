
#include <stdio.h>
#include <ctype.h>

#include <Elementary.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

#include "LumbrJack.h"
#include "Runnr.h"

typedef struct _globals globals;


#define WIDTH  (300)
#define HEIGHT (300)

#define SKANG		"skang"
#define MODULEBEGIN	"moduleBegin"
#define MODULEEND	"moduleEnd"
#define THINGASM	"thingasm"


struct _globals
{
  Evas_Object	*win;		// Our Elm window.
  int		logDom;		// Our logging domain.
};


int luaopen_widget(lua_State *L);
void GuiLuaDo(int argc, char **argv);
