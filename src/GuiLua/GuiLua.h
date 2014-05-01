#include "SledjHamr.h"
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
  Evas		*evas;
  Evas_Object	*win;		// Our Elm window.
  Eina_Clist	widgets;	// Our windows widgets.
  int		logDom;		// Our logging domain.
};


int luaopen_widget(lua_State *L);
void GuiLuaDo(int argc, char **argv);
