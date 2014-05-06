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
  winFang	*parent;	// Our parent window, if it exists.

  Eina_Clist	node;
  void		*data;
  Evas_Smart_Cb on_del;
} GuiLua;

GuiLua *GuiLuaDo(int argc, char **argv, winFang *parent);

#endif
