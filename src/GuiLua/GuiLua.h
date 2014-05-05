#define WIDTH  (300)
#define HEIGHT (300)

#define SKANG		"skang"
#define MODULEBEGIN	"moduleBegin"
#define MODULEEND	"moduleEnd"
#define THINGASM	"thingasm"


typedef struct _GuiLua
{
  lua_State	*L;
  Evas_Object	*parent;	// Our parent window, if it exists.
  Eina_Clist	winFangs;	// The windows we might open.

  Eina_Clist	node;
  void		*data;
  Evas_Smart_Cb on_del;
} GuiLua;

GuiLua *GuiLuaDo(int argc, char **argv, Evas_Object *parent);
