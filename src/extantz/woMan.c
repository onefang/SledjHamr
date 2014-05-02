#include "extantz.h"


Eina_Hash *grids;
Eina_Hash *viewers;

static char *gridTest[][3] =
{
    {"3rd Rock Grid", "http://grid.3rdrockgrid.com:8002/", "http://grid.3rdrockgrid.com/3rg_login"},
    {"Infinite Grid", "http://grid.infinitegrid.org:8002/", "http://www.infinitegrid.org/loginscreen.php"},
    {"Second Life Grid", "https://login.agni.lindenlab.com/cgi-bin/login.cgi", "http://secondlife.com/"},
    {NULL, NULL, NULL}
};

static char *accountTest[][3] =
{
    {"3rd Rock Grid", "onefang rejected", "password"},
    {"Infinite Grid", "infinite onefang", "MyB1GSecrit"},
    {"Infinite Grid", "onefang rejected", "MySecrit"},
    {NULL, NULL, NULL}
};


static char *viewerTest[][3] =
{
    {"Imprudence",	"1.4.0 beta 3", ""},
    {"Kokua",		"3.4.4.25633", ""},
    {"meta-impy",	"1.4.0 beta 1.5", ""},
    {"SL",		"v3", ""},
    {NULL, NULL, NULL}
};


static Elm_Genlist_Item_Class *grid_gic = NULL;
static Elm_Genlist_Item_Class *account_gic = NULL;
static Elm_Genlist_Item_Class *viewer_gic = NULL;

//static const char *img1 = PACKAGE_DATA_DIR "plant_01.jpg";
//static const char *img2 = PACKAGE_DATA_DIR "sky_01.jpg";
static const char *img3 = "rock_01.jpg";



static Evas_Object *_content_image_new(Evas_Object *parent, const char *img)
{
   Evas_Object *ic;

   ic = elm_icon_add(parent);
   elm_image_file_set(ic, img, NULL);
   return ic;
}

static void _promote(void *data, Evas_Object *obj , void *event_info )
{
   elm_naviframe_item_promote(data);
}

static char *_grid_label_get(void *data, Evas_Object *obj, const char *part)
{
    ezGrid *thisGrid = data;
    char buf[256];

    if (!strcmp(part, "elm.text"))
    {
	int count = eina_clist_count(&(thisGrid->accounts));

	if (0 == count)
	    snprintf(buf, sizeof(buf), "%s (no accounts)", thisGrid->name);
	else if (1 == count)
	    snprintf(buf, sizeof(buf), "%s (%d account)", thisGrid->name, count);
	else
	    snprintf(buf, sizeof(buf), "%s (%d accounts)", thisGrid->name, count);
    }
    else
	snprintf(buf, sizeof(buf), "%s", thisGrid->loginURI);
    return strdup(buf);
}

static Evas_Object *_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
    ezGrid *thisGrid = data;
    Evas_Object *ic = elm_icon_add(obj);

    if (!strcmp(part, "elm.swallow.icon"))
	elm_icon_standard_set(ic, thisGrid->icon);

    evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
    return ic;
}

static char * _account_label_get(void *data, Evas_Object *obj, const char *part)
{
    ezAccount *thisAccount = data;
    char buf[256];

    buf[0] = '\0';
    if (!strcmp(part, "elm.text"))
	snprintf(buf, sizeof(buf), "%s", thisAccount->name);

    return strdup(buf);
}

static Evas_Object *_account_content_get(void *data, Evas_Object *obj, const char *part)
{
    ezAccount *thisAccount = data;
    Evas_Object *ic = elm_icon_add(obj);

    if (!strcmp(part, "elm.swallow.icon"))
	elm_icon_standard_set(ic, thisAccount->icon);

    evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
    return ic;
}

static char *_viewer_label_get(void *data, Evas_Object *obj, const char *part)
{
    ezViewer *thisViewer = data;
    char buf[256];

    if (!strcmp(part, "elm.text"))
	snprintf(buf, sizeof(buf), "%s", thisViewer->name);
    else
	snprintf(buf, sizeof(buf), "%s", thisViewer->version);
    return strdup(buf);
}

static Evas_Object *_viewer_content_get(void *data, Evas_Object *obj, const char *part)
{
    ezViewer *thisViewer = data;
    Evas_Object *ic = elm_icon_add(obj);

    if (!strcmp(part, "elm.swallow.icon"))
	elm_icon_standard_set(ic, thisViewer->icon);

    evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
    return ic;
}


static void _grid_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
    ezGrid *thisGrid = data;
    GLData *gld = thisGrid->gld;
    char buf[PATH_MAX];

//    sprintf(buf, "dillo -f -g '%dx%d+%d+%d' %s &", gld->win_w - (gld->win_w / 5), gld->win_h - 30, gld->win_w / 5, gld->win_y, thisGrid->splashPage);
    sprintf(buf, "uzbl -g '%dx%d+%d+%d' -u %s &", gld->win_w - (gld->win_w / 5), gld->win_h - 30, gld->win_w / 5, gld->win_y, thisGrid->splashPage);
    printf("%s   ### genlist obj [%p], item pointer [%p]\n", buf, obj, event_info);
// comment this out for now, busy dealing with input stuff, don't want to trigger this multiple times.
//    system(buf);
}


void woMan_add(GLData *gld)
{
//    Evas_Object *win, *bg, *bx, *ic, *bb, *av, *en, *bt, *nf, *tab, *tb, *gridList, *viewerList, *menu;
    Evas_Object *win, *bx, *bt, *nf, *tab, *tb, *gridList, *viewerList, *menu;
    Elm_Object_Item *tb_it, *menu_it, *tab_it;
    char buf[PATH_MAX];
    int i;

    win = fang_win_add(gld);

    bx = elm_box_add(win);
    elm_win_resize_object_add(win, bx);
    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

    // A tab thingy.
    tb = elm_toolbar_add(win);
    evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_SCROLL);

    // Menu.
    tb_it = elm_toolbar_item_append(tb, NULL, "Menu", NULL, NULL);
    elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
    // Priority is for when toolbar items are set to hide or menu when there are too many of them.  They get hidden or put on the menu based on priority.
    elm_toolbar_item_priority_set(tb_it, 9999);
    elm_toolbar_menu_parent_set(tb, win);
    menu = elm_toolbar_item_menu_get(tb_it);

    menu_it = elm_menu_item_add(menu, NULL, NULL, "edit", NULL, NULL);
    elm_menu_item_add(menu, menu_it, NULL, "preferences", NULL, NULL);
    menu_it = elm_menu_item_add(menu, NULL, NULL, "help", NULL, NULL);
    elm_menu_item_add(menu, menu_it, NULL, "about woMan", NULL, NULL);
    elm_menu_item_separator_add(menu, NULL);
    menu_it = elm_menu_item_add(menu, NULL, NULL, "advanced", NULL, NULL);
    elm_menu_item_add(menu, menu_it, NULL, "debug settings", NULL, NULL);

    // The toolbar needs to be packed into the box AFTER the menus are added.
    elm_box_pack_end(bx, tb);
    evas_object_show(tb);

    gridList = elm_genlist_add(win);
    grids = eina_hash_stringshared_new(free);

    grid_gic = elm_genlist_item_class_new();
    grid_gic->item_style = "double_label";
    grid_gic->func.text_get = _grid_label_get;
    grid_gic->func.content_get = _grid_content_get;
    grid_gic->func.state_get = NULL;
    grid_gic->func.del = NULL;
    for (i = 0; NULL != gridTest[i][0]; i++)
    {
	ezGrid *thisGrid = calloc(1, sizeof(ezGrid));
	
	if (thisGrid)
	{
	    eina_clist_init(&(thisGrid->accounts));
	    eina_clist_init(&(thisGrid->landmarks));
	    thisGrid->name		= gridTest[i][0];
	    thisGrid->loginURI		= gridTest[i][1];
	    thisGrid->splashPage 	= gridTest[i][2];
	    thisGrid->icon		= "folder";
	    thisGrid->gld		= gld;
	    thisGrid->item = elm_genlist_item_append(gridList, grid_gic, thisGrid, NULL, ELM_GENLIST_ITEM_TREE, _grid_sel_cb, thisGrid);
	    eina_hash_add(grids, thisGrid->name, thisGrid);
	}
    }

    account_gic = elm_genlist_item_class_new();
    account_gic->item_style = "default";
    account_gic->func.text_get = _account_label_get;
    account_gic->func.content_get = _account_content_get;
    account_gic->func.state_get = NULL;
    account_gic->func.del = NULL;
    for (i = 0; NULL != accountTest[i][0]; i++)
    {
	ezAccount *thisAccount = calloc(1, sizeof(ezAccount));
	ezGrid *grid = eina_hash_find(grids, accountTest[i][0]);
	
	if (thisAccount && grid)
	{
	    thisAccount->name		= accountTest[i][1];
	    thisAccount->password 	= accountTest[i][2];
	    thisAccount->icon		= "file";
	    elm_genlist_item_append(gridList, account_gic, thisAccount, grid->item, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	    eina_clist_add_tail(&(grid->accounts), &(thisAccount->grid));
	}
    }

    // Viewers stuff
    viewerList = elm_genlist_add(win);
    viewer_gic = elm_genlist_item_class_new();
    viewer_gic->item_style = "double_label";
    viewer_gic->func.text_get = _viewer_label_get;
    viewer_gic->func.content_get = _viewer_content_get;
    viewer_gic->func.state_get = NULL;
    viewer_gic->func.del = NULL;
    for (i = 0; NULL != viewerTest[i][0]; i++)
    {
	ezViewer *thisViewer = calloc(1, sizeof(ezViewer));
	
	if (thisViewer)
	{
	    thisViewer->name		= viewerTest[i][0];
	    thisViewer->version		= viewerTest[i][1];
	    thisViewer->path		= viewerTest[i][2];
	    thisViewer->icon		= "file";
	    thisViewer->item = elm_genlist_item_append(viewerList, viewer_gic, thisViewer, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}
    }

    // Toolbar pages
    nf = elm_naviframe_add(win);
    evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(nf, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(nf);

    sprintf(buf, "%s/%s", elm_app_data_dir_get(), img3);
    tab = viewerList;				tab_it = elm_naviframe_item_push(nf, NULL, NULL, NULL, tab, NULL);	elm_naviframe_item_title_enabled_set(tab_it, EINA_FALSE, EINA_TRUE);	elm_toolbar_item_append(tb, NULL, "Viewers", _promote, tab_it);
    tab = _content_image_new(win, strdup(buf));	tab_it = elm_naviframe_item_push(nf, NULL, NULL, NULL, tab, NULL);	elm_naviframe_item_title_enabled_set(tab_it, EINA_FALSE, EINA_TRUE);	elm_toolbar_item_append(tb, NULL, "Landmarks", _promote, tab_it);
    tab = gridList;				tab_it = elm_naviframe_item_push(nf, NULL, NULL, NULL, tab, NULL);	elm_naviframe_item_title_enabled_set(tab_it, EINA_FALSE, EINA_TRUE);	elm_toolbar_item_append(tb, NULL, "Grids", _promote, tab_it);
    elm_box_pack_end(bx, nf);

    bt = eo_add(ELM_OBJ_BUTTON_CLASS, win);
    elm_object_text_set(bt, "Login");		// No eo interface for this that I can find.
    eo_do(bt, 
//		evas_obj_text_set("Login"),
		evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
		evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, 0.0),
		evas_obj_visibility_set(EINA_TRUE)
	);
//    evas_object_smart_callback_add(bt, "clicked", NULL, NULL);
    elm_box_pack_end(bx, bt);
    eo_unref(bt);
    evas_object_show(bx);

    fang_win_complete(gld, win, 30, 30, gld->win_w / 3, gld->win_h / 3);
}
