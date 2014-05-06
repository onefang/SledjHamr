#ifndef _WINFANG_H_
#define _WINFANG_H_


#define EFL_API_OVERRIDE 1
/* Enable access to unstable EFL API that are still in beta */
#define EFL_BETA_API_SUPPORT 1
/* Enable access to unstable EFL EO API. */
#define EFL_EO_API_SUPPORT 1


#include <Eo.h>
#include <Eina.h>
#include <Evas.h>
#include <Elementary.h>


typedef struct _winFang
{
  Evas_Object	*win;
  Evas_Object	*bg;
  Eina_Clist	widgets;
  Eina_Clist	winFangs;
  int		x, y, w, h;
  Eina_Bool	internal;

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

winFang *winFangAdd(winFang *parent, int x, int y, int w, int h, char *title, char *name);
void winFangHide(winFang *win);
void winFangShow(winFang *win);
void winFangDel(winFang *win);

Widget *widgetAdd(winFang *win, const Eo_Class *klass, Evas_Object *parent, char *title);

#endif
