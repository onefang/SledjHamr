#include <Elementary.h>


// Forward references.
typedef struct _globals globals;

typedef struct _winFang
{
  Evas_Object	*win;
  Eina_Clist	widgets;
  void		*data;
  Evas_Smart_Cb on_del;
} winFang;

typedef struct _Widget
{
  char		magic[8];
  Evas_Object	*obj;
  Eina_Clist	node;
  char		*label, *look, *action, *help;
  // foreground / background colour
  // thing
  // types {}
  // skangCoord x, y, w, h
  void		*data;
  Evas_Smart_Cb on_del;
} Widget;

winFang *winFangAdd(globals *ourGlobals);
void winFangComplete(globals *ourGlobals, winFang *win, int x, int y, int w, int h);
void winFangDel(globals *ourGlobals, winFang *win);
Widget *widgetAdd(winFang *win, const Eo_Class *klass, Evas_Object *parent, char *title);
