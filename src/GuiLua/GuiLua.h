
#define EFL_API_OVERRIDE 1
/* Enable access to unstable EFL API that are still in beta */
#define EFL_BETA_API_SUPPORT 1
/* Enable access to unstable EFL EO API. */
#define EFL_EO_API_SUPPORT 1

#include <stdio.h>
#include <ctype.h>

#include <Elementary.h>

// This got left out.
//EAPI Evas_3D_Scene *evas_3d_scene_add(Evas *e);


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

//Ecore_Evas       *ecore_evas;
Evas             *evas;
//Evas_Object      *background;
Evas_Object      *image;
};


int luaopen_widget(lua_State *L);
void GuiLuaDo(int argc, char **argv);
