#include "SledjHamr.h"
#include "LumbrJack.h"
#include "Runnr.h"
#include "winFang.h"


#define WIDTH  (300)
#define HEIGHT (300)

#define SKANG		"skang"
#define MODULEBEGIN	"moduleBegin"
#define MODULEEND	"moduleEnd"
#define THINGASM	"thingasm"


typedef struct _globals
{
  winFang	*win;
  int		logDom;		// Our logging domain.
} globals;


void GuiLuaDo(int argc, char **argv);
