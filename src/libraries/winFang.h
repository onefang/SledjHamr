#ifndef _WINFANG_H_
#define _WINFANG_H_


#include <Elementary.h>
#include <EPhysics.h>


#define WF_BACKGROUND	"winFang/background"
#define WF_LAYOUT	"winFang/layout"
#define WF_UNDERLAY	"winFang/underlay"
#define WF_TITLE	"winFang/title"
#define WF_BOX		"winFang/box"
#define WF_SWALLOW	"winFang/content"


typedef struct _widgetSpec
{
  char *name;
  const Eo_Class *klass;
} widgetSpec;

#define WT_CHECK	"check"
#define WT_BOX		"box"
#define WT_BUTTON	"button"
#define WT_FILES	"files"
#define WT_ENTRY	"entry"
#define WT_GRID		"grid"
#define WT_HOVER	"hoversel"
#define WT_IMAGE	"image"
#define WT_LABEL	"label"
#define WT_LAYOUT	"layout"
#define WT_RADIO	"radio"
#define WT_RECT		"rectangle"
#define WT_TEXT		"text"
#define WT_TEXTBOX	"textbox"
#define WT_TOOLBAR	"toolbar"


typedef struct _winFang
{
  Evas		*e;
  struct _winFang *parent;
  Evas_Object	*win;
  Evas_Object	*layout;
  Evas_Object	*title;
  Evas_Object	*bg;
  Evas_Object	*grid;
  EPhysics_Body *body;
  Eina_Clist	widgets;
  Eina_Clist	winFangs;
  char		*module;
  int		x, y, w, h, mw, mh;

  Evas_Object *hand[4];

  Eina_Clist	node;
  void		*data;
  Evas_Smart_Cb on_del;
} winFang;

typedef struct _Widget
{
  char		magic[8];
  char		type[16];
  Evas_Object	*obj;
  winFang	*win;

  char		*label, *look, *action, *help;
  // foreground / background colour
  // thing
  // types {}
  // skangCoord x, y, w, h

  Eina_Clist	node;
  void		*data;
  Evas_Smart_Cb on_del;
} Widget;

winFang *winFangAdd(winFang *parent, int x, int y, int w, int h, char *title, char *name, EPhysics_World *world);
void winFangHide(winFang *win);
void winFangShow(winFang *win);
void widgetHide(Widget *wid);
void widgetShow(Widget *wid);
void winFangCalcMinSize(winFang *win);
void winFangDel(winFang *win);

Widget *widgetAdd(winFang *win, char *type, char *title, int x, int y, int w, int h);
void widgetDel(Widget *wid);

#endif
