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
  winFang	*us;		// Our window, if it exists.
  winFang	*parent;	// Our parent window, if it exists.
  int		inDel;

  Eina_Clist	node;
  void		*data;
  Evas_Smart_Cb on_del;
} GuiLua;

extern const char	*glName;

GuiLua *GuiLuaDo(int argc, char **argv, winFang *parent);
void GuiLuaLoad(char *module, winFang *parent);
void GuiLuaDel(GuiLua *gl);

#endif
