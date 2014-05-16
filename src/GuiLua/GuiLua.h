#ifndef _GUILUA_H_
#define _GUILUA_H_


#include "Runnr.h"
#include "winFang.h"


#define WIDTH  (300)
#define HEIGHT (300)

#define SKANG		"skang"
#define MODULEBEGIN	"moduleBegin"
#define MODULEEND	"moduleEnd"
#define THINGASM	"thingasm"


typedef struct _GuiLua
{
  lua_State	*L;
  char		*name;		// Name of the module.
  winFang	*us;		// Our window, if it exists.
  winFang	*parent;	// Our parent window, if it exists.
  EPhysics_World *world;	// Our world, if it exists.
  Ecore_Con_Server	*server;
  int		inDel;

  Eina_Clist	node;
  void		*data;
  Evas_Smart_Cb on_del;
} GuiLua;

extern const char	*glName;

GuiLua *GuiLuaDo(int argc, char **argv, winFang *parent, Ecore_Con_Server *server, EPhysics_World *world);
GuiLua *GuiLuaLoad(char *module, winFang *parent, Ecore_Con_Server *server, EPhysics_World *world);
void GuiLuaDel(GuiLua *gl);

#endif
