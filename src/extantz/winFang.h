#include <Elementary.h>


typedef struct _winFang
{
  Evas_Object	*win;
  Eina_Clist	widgets;
  int		x, y, w, h;

  Evas_Object *hand[4];

  Eina_Clist	node;
  void		*data;
  Evas_Smart_Cb on_del;
} winFang;

typedef struct _Widget
{
  char		magic[8];
  Evas_Object	*obj;

  char		*label, *look, *action, *help;
  // foreground / background colour
  // thing
  // types {}
  // skangCoord x, y, w, h

  Eina_Clist	node;
  void		*data;
  Evas_Smart_Cb on_del;
} Widget;

winFang *winFangAdd(Evas_Object *parent, int x, int y, int w, int h);
void winFangHide(winFang *win);
void winFangShow(winFang *win);
void winFangDel(winFang *win);

Widget *widgetAdd(winFang *win, const Eo_Class *klass, Evas_Object *parent, char *title);
