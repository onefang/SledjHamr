#include <unistd.h>

#include "extantz.h"
#include "SledjHamr.h"


#if USE_EVAS_3D
static void _onWorldClick(void *data, Evas *e EINA_UNUSED, Evas_Object *o, void *einfo);
static void on_pixels(void *data, Evas_Object *obj);
#endif


int logDom = -1;	// Our logging domain.
globals ourGlobals;
static Eina_Strbuf *serverStream;
//static char *myKey = "12345678-1234-4321-abcd-0123456789ab";
//static char *myName = "onefang rejected";



static Eina_Bool _add(void *data, int type, Ecore_Con_Event_Server_Add *ev)
{
  globals *ourGlobals = data;

  PI("Connected to love server.");
  ourGlobals->server = ev->server;
  if (ourGlobals->LSLGuiMess)  ourGlobals->LSLGuiMess->server = ourGlobals->server;
  if (ourGlobals->purkle)      ourGlobals->purkle->server     = ourGlobals->server;

  // TODO - If this is not a local love server, we should attempt to log in here.
  //        Or attempt a hypergrid style TP.

  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _data(void *data, int type, Ecore_Con_Event_Server_Data *ev)
{
  globals *ourGlobals = data;
  char buf[PATH_MAX];
  char SID[PATH_MAX];
  const char *command;
  char *ext;

  eina_strbuf_append_length(serverStream, ev->data, ev->size);
  command = eina_strbuf_string_get(serverStream);
  while ((ext = index(command, '\n')))
  {
    int length = ext - command;

    strncpy(SID, command, length + 1);
    SID[length] = '\0';
    eina_strbuf_remove(serverStream, 0, length + 1);
    ext = index(SID, '.');
    if (ext)
    {
	ext[0] = '\0';
	command = ext + 1;
	if ((0 == strncmp(command, "llOwnerSay(", 11))
	  || (0 == strncmp(command, "llWhisper(", 10))
	  || (0 == strncmp(command, "llSay(", 6))
	  || (0 == strncmp(command, "llShout(", 8)))
	{

	  sprintf(buf, "%s: %s", SID, command);
	  if (ourGlobals->purkle)
	  {
	    int _P;

	    lua_getfield(ourGlobals->purkle->L, LUA_REGISTRYINDEX, ourGlobals->purkle->name);
	    _P = lua_gettop(ourGlobals->purkle->L);
	    push_lua(ourGlobals->purkle->L, "@ ( $ )", _P, "append", buf, 0);
	  }
	  else
	    PW("No purkle to put - %s", buf);
	}
	else if (0 == strncmp(command, "llDialog(", 9))
	{
	  if (ourGlobals->LSLGuiMess)
	  {
	    int _M;

	    lua_getfield(ourGlobals->LSLGuiMess->L, LUA_REGISTRYINDEX, ourGlobals->LSLGuiMess->name);
	    _M = lua_gettop(ourGlobals->LSLGuiMess->L);

	    // TODO - Somewhere in the chain the new lines that MLP likes to put into llDialog's message munge things.  Fix that.
	    push_lua(ourGlobals->LSLGuiMess->L, "@ ( $ )", _M, "doLua", command, 0);
	  }
	  else
	    PE("No LSLGuiMess to send - %s", command);

	}
	else if (0 == strncmp(command, "loadSim(", 8))
	{
#if USE_EVAS_3D
	  char *p, *t;
	  int scenriLua;
#endif

          // Pretend we logged in.  Actually in the case of a local love server, we realy have logged in now.
          strcpy(ourGlobals->uuid, SID);
          PI("Your UUID is %s.", ourGlobals->uuid);
#if USE_EVAS_3D
	  strcpy(buf, &command[8]);
	  p = buf;
          while ('"' == p[0])
	    p++;
	  while ('\'' == p[0])
	    p++;
	  t = p;
          while (('"' != p[0]) && ('\'' != p[0]))
	    p++;
	  p[0] = '\0';
	  // TODO - For now, assume it's a file:// URL.
	  t += 7;
	  //strcat(t, "/index.omg");
	  strcpy(ourGlobals->scene->sim, t);
	  PI("Loading local sim from %s", t);

	// TODO - Later do the same with eet files in C code, but keep both implementations.
	  lua_getglobal(ourGlobals->scene->L, "package");
	  lua_getfield(ourGlobals->scene->L, lua_gettop(ourGlobals->scene->L), "loaded");
	  lua_remove(ourGlobals->scene->L, -2);				// Removes "package"
	  lua_getfield(ourGlobals->scene->L, lua_gettop(ourGlobals->scene->L), "scenriLua");
	  lua_remove(ourGlobals->scene->L, -2);				// Removes "loaded"
	  scenriLua = lua_gettop(ourGlobals->scene->L);

	  push_lua(ourGlobals->scene->L, "@ ( $ )", scenriLua, "loadSim", t, 0);
	  PI("Loaded local sim from %s", t);
#endif
	}
	else
	{
	  PI("Some random command %s", command);
	}
    }

    // Get the next blob to check it.
    command = eina_strbuf_string_get(serverStream);
  }

  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _del(void *data, int type, Ecore_Con_Event_Server_Del *ev)
{
  globals *ourGlobals = data;
  static int count = 0;

  ourGlobals->server = NULL;

  // Let it fail a couple of times during startup, then try to start our own love server.
  count++;
  if (1 < count)
  {
    char buf[PATH_MAX];

    PW("Failed to connect to a world server, starting our own.");

    // TODO - Should use Ecore_Exe for this sort of thing.
    sprintf(buf, "%s/love &", prefix_bin_get());
    system(buf);
    count = 0;
  }

  if (ourGlobals->running)
    return ECORE_CALLBACK_RENEW;
  return ECORE_CALLBACK_CANCEL;
}

#if USE_EVAS_3D
static void _onWorldClick(void *data, Evas *e EINA_UNUSED, Evas_Object *o, void *einfo)
{
  Evas_3D_Node *n = data;
//  Evas_Event_Mouse_Down *ev = einfo;

  if (n)
  {
    char *name = NULL;

    name = evas_object_data_get(n, "Name");
    if (strcmp("onefang's test bed", name) == 0)
    {
      char SID[64];

      // CUBE_UUID.events.touch_start(1), but we just make one up for now.
      snprintf(SID, sizeof(SID), FAKE_UUID);
      sendForth(ourGlobals.server, SID, "events.touch_start(1)");
    }
  }
}
#endif

static void gldata_init(GLData *gld)
{
    gld->useIrr = USE_IRR;

#if DO_GEARS
    gld->view_rotx = -20.0;
    gld->view_roty = -30.0;
    gld->view_rotz = 0.0;
    gld->angle = 0.0;

    gld->light[0] = 1.0;
    gld->light[1] = 1.0;
    gld->light[2] = -5.0;
#endif
}


static void _resize_winwin(GLData *gld)
{
    Evas_Coord x, y, w, h;

    evas_object_geometry_get(gld->elmGl, &x, &y, &w, &h);
    evas_object_move(elm_win_inlined_image_object_get (gld->winwin), x, y);
    evas_object_resize(elm_win_inlined_image_object_get(gld->winwin), w, h);
}

// Called from on_pixels (), or the Elm_gliew resize callback.
static void _resize(GLData *gld)
{
   Evas_GL_API *gl = gld->glApi;

   if (gld->elmGl)
      _resize_winwin(gld);

#if DO_GEARS
   GLfloat ar, m[16] = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 0.1, 0.0,
      0.0, 0.0, 0.0, 1.0
   };

   // GL Viewport stuff. you can avoid doing this if viewport is all the
   // same as last frame if you want
   if (gld->img_w < gld->img_h)
     ar = gld->img_w;
   else
     ar = gld->img_h;

   m[0] = 0.1 * ar / gld->img_w;
   m[5] = 0.1 * ar / gld->img_h;
   memcpy(gld->proj, m, sizeof gld->proj);
#endif

   if (gl)
      gl->glViewport(0, 0, (GLint) gld->img_w, (GLint) gld->img_h);
}

static void _resize_gl(Evas_Object *obj)
{
  globals *ourGlobals = evas_object_data_get(obj, "glob");
  GLData *gld = &ourGlobals->gld;
  int w, h;
  if (!ourGlobals)  return;

  elm_glview_size_get(obj, &w, &h);
  gld->img_w = w;
  gld->img_h = h;
  _resize(gld);
}

static void _on_resize(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  globals *ourGlobals = data;
  GLData *gld = &ourGlobals->gld;
  Evas_Coord h;

  eo_do(ourGlobals->win, efl_gfx_size_get(&ourGlobals->win_w, &ourGlobals->win_h));
  eo_do(ourGlobals->tb,
    evas_obj_size_hint_min_get(NULL, &h),
    efl_gfx_size_set(ourGlobals->win_w, h)
    );
  // Stop internal windows going under the toolbar.
  evas_object_resize(ourGlobals->mainWindow->layout, ourGlobals->win_w, h);
  if (ourGlobals->world)
    ephysics_world_render_geometry_set(ourGlobals->world, 0, 0, -50, ourGlobals->win_w, ourGlobals->win_h, 100);
  _resize(gld);
}

// Callback from Evas, also used as the general callback for deleting the GL stuff.
static void _clean_gl(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  globals *ourGlobals = data;

  ecore_animator_del(ourGlobals->animator);

  // Do a make_current before deleting all the GL stuff.
//    evas_gl_make_current(NULL, NULL, NULL);

  // TODO - Since this is created on the render thread, better hope this is being deleted on the render thread.
#if USE_IRR
  finishIrr(ourGlobals);
#endif

#if DO_GEARS
  Evas_GL_API *gl = gld->glApi;

  gl->glDeleteShader(gld->vtx_shader);
  gl->glDeleteShader(gld->fgmt_shader);
  gl->glDeleteProgram(gld->program);

  gl->glDeleteBuffers(1, &gld->gear1->vbo);
  gl->glDeleteBuffers(1, &gld->gear2->vbo);
  gl->glDeleteBuffers(1, &gld->gear3->vbo);

  free_gear(gld->gear1);
  free_gear(gld->gear2);
  free_gear(gld->gear3);
#endif

  evas_object_data_del((Evas_Object*)obj, "glob");
}

// Callback from Elm, coz they do shit different.
static void _del_gl(Evas_Object *obj)
{
  globals *ourGlobals = evas_object_data_get(obj, "glob");
  if (!ourGlobals)  return;

  _clean_gl(ourGlobals, NULL, NULL, NULL);
}

static void _on_open(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  globals *ourGlobals = data;

  filesShow(ourGlobals->files, NULL, NULL);
}

static void _on_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    elm_exit();
}

static void _draw_gl(Evas_Object *obj)
{
  globals *ourGlobals = evas_object_data_get(obj, "glob");
  GLData *gld = &ourGlobals->gld;
  if (!ourGlobals)  return;

#if USE_IRR
  if (!gld->doneIrr)		gld->doneIrr = startIrr(ourGlobals);	// Needs to be after gld->win is shown, and needs to be done in the render thread.
#endif

#if DO_GEARS
  if (!gld->gearsInited)	gears_init(gld);
#endif

//  if (gld->resized)		_resize(gld);

#if USE_IRR
  drawIrr_start(ourGlobals);
#endif

#if DO_GEARS
  drawGears(gld);
#endif

  animateScene(ourGlobals);
  animateCamera(ourGlobals->scene);

#if USE_IRR
  drawIrr_end(ourGlobals);
#endif

#if USE_IRR
#else
  // This might get done deep within drawIrr_end, but only if we are using Irrlicht.

  // Optional - Flush the GL pipeline
//  gl->glFlush();
//  gl->glFinish();
#endif

  gld->resized = 0;
}

#if USE_EVAS_3D
static void on_pixels(void *data, Evas_Object *obj)
{
  _draw_gl(obj);
}
#endif

// Callback from the animator.
static Eina_Bool doFrame(void *data)
{
  globals *ourGlobals = data;
  GLData *gld = &ourGlobals->gld;

  // Mark the pixels as dirty, so they get rerendered each frame, then Irrlicht can draw it's stuff each frame.
  // This causes on_pixel to be triggered by Evas_3D, or _draw_gl for Elm_glview.
  // Either way, _draw_gl gets called eventully.
  if (gld->elmGl)
    elm_glview_changed_set(gld->elmGl);
  else if ((ourGlobals->scene) && (ourGlobals->scene->image))
  {
//    evas_object_image_pixels_dirty_set(elm_image_object_get(ourGlobals->scene->image), EINA_TRUE);
    _draw_gl(elm_image_object_get(ourGlobals->scene->image));
  }

  return EINA_TRUE;	// Keep calling us.
}

static void init_evas_gl(globals *ourGlobals)
{
  GLData *gld = &ourGlobals->gld;

  gld->sfc_w = ourGlobals->win_w;
  gld->sfc_h = ourGlobals->win_h;

  if (USE_IRR || DO_GEARS)
  {
    gld->sfc_h = ourGlobals->win_h;

    // Add a GLView
    gld->elmGl = elm_glview_add(ourGlobals->win);
    evas_object_size_hint_align_set(gld->elmGl, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(gld->elmGl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_glview_mode_set(gld->elmGl, 0 | ELM_GLVIEW_ALPHA | ELM_GLVIEW_DEPTH | ELM_GLVIEW_DIRECT);
    elm_glview_resize_policy_set(gld->elmGl, ELM_GLVIEW_RESIZE_POLICY_RECREATE);		// Destroy the current surface on a resize and create a new one.
    elm_glview_render_policy_set(gld->elmGl, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);
//    elm_glview_render_policy_set(gld->elmGl, ELM_GLVIEW_RENDER_POLICY_ALWAYS);

    gld->glApi = elm_glview_gl_api_get(gld->elmGl);
    evas_object_data_set(gld->elmGl, "glob", ourGlobals);
    // These get called in the render thread I think.
    // None let me pass data, so this is why we are adding "glob" data to the object above.
    // Maybe we can use  elm_object_signal_callback_add or elm_object_item_signal_callback_add (edje signals)?
    //elm_glview_init_func_set(gld->elmGl, _init_gl);	// Not actually needed, it gets done in on_pixels.
    elm_glview_del_func_set(gld->elmGl, _del_gl);
    elm_glview_resize_func_set(gld->elmGl, _resize_gl);
    elm_glview_render_func_set(gld->elmGl, (Elm_GLView_Func_Cb) _draw_gl);

    elm_win_resize_object_add(ourGlobals->win, gld->elmGl);
    evas_object_show(gld->elmGl);
  }

  // TODO - apparently the proper way to deal with the new async rendering is to have this animator do the dirty thing, and call the Irrlicht rendering stuff in the _draw_gl call set above.
  //        That still leaves the problem of the Irrlicht setup being in the main thread.  Which also should be done in on_pixel, as that's done in the correct thread.

  // Jiggling this seems to produce a trade off between flickering and frame rate.  Nothing else changed the flickering.
  ecore_animator_frametime_set(0.04);	// Default is 1/30, or 0.033333
  ourGlobals->animator = ecore_animator_add(doFrame, ourGlobals);	// This animator will be called every frame tick, which defaults to 1/30 seconds.

  return;
}


//-------------------------//

static void _on_menu_focus(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
  evas_object_raise(obj);
}

static Evas_Object *_menuAdd(Evas_Object *win, Evas_Object *tb, char *label)
{
  Evas_Object *menu= NULL;
  Elm_Object_Item *tb_it;

  // Evas_Object * obj, const char *icon, const char *label, Evas_Smart_Cb  func, const void *data
  //   The function is called when the item is clicked.
  tb_it = elm_toolbar_item_append(tb, NULL, label, NULL, NULL);
  // Mark it as a menu.
  elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
  // This alledgedly marks it as a menu (not true), and gets an Evas_Object for us to play with.
  menu = elm_toolbar_item_menu_get(tb_it);
  // Alas this does not work.  B-(
  evas_object_smart_callback_add(menu, "focused", _on_menu_focus, NULL);

  // Priority is for when toolbar items are set to hide or menu when there are too many of them.  They get hidden or put on the menu based on priority.
  elm_toolbar_item_priority_set(tb_it, 9999);

  // The docs for this functien are meaningless, but it wont work right if you leave it out.
  elm_toolbar_menu_parent_set(tb, win);

  // Add a seperator after, so that there's some visual distinction between menu items.
  // Coz using all lower case and with no underline on the first letter, it's hard to tell.
  tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
  elm_toolbar_item_separator_set(tb_it, EINA_TRUE);

  return menu;
}

static void makeMainMenu(globals *ourGlobals)
{
    GLData *gld = &ourGlobals->gld;
    Evas_Object *menu, *tb;
    Elm_Object_Item *tb_it;

    // A toolbar thingy.
    tb = eo_add(ELM_TOOLBAR_CLASS, ourGlobals->win,
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, 0.0),
	evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
	elm_obj_toolbar_shrink_mode_set(ELM_TOOLBAR_SHRINK_MENU),
	efl_gfx_position_set(0, 0),
	elm_obj_toolbar_align_set(0.0)
	);
    ourGlobals->tb = tb;

    // Menus.
    menu = _menuAdd(ourGlobals->win, tb, "file");
    // Evas_Object *obj, Elm_Object_Item *parent, const char *icon, const char *label, Evas_Smart_Cb func, const void *data
    elm_menu_item_add(menu, NULL, NULL, "open", _on_open, ourGlobals);
    elm_menu_item_add(menu, NULL, NULL, "quit", _on_done, gld);

    menu = _menuAdd(ourGlobals->win, tb, "edit");
    elm_menu_item_add(menu, NULL, NULL, "preferences", NULL, NULL);

    menu = _menuAdd(ourGlobals->win, tb, "view");
    menu = _menuAdd(ourGlobals->win, tb, "world");
    menu = _menuAdd(ourGlobals->win, tb, "tools");

    menu = _menuAdd(ourGlobals->win, tb, "help");
    elm_menu_item_add(menu, NULL, NULL, "grid help", NULL, NULL);
    elm_menu_item_separator_add(menu, NULL);
    elm_menu_item_add(menu, NULL, NULL, "extantz blogs", NULL, NULL);
    elm_menu_item_add(menu, NULL, NULL, "extantz forum", NULL, NULL);
    elm_menu_item_separator_add(menu, NULL);
    elm_menu_item_add(menu, NULL, NULL, "about extantz", NULL, NULL);

    menu = _menuAdd(ourGlobals->win, tb, "advanced");
    elm_menu_item_add(menu, NULL, NULL, "debug settings", NULL, NULL);

    menu = _menuAdd(ourGlobals->win, tb, "god");

    // Other stuff in the toolbar.
    tb_it = elm_toolbar_item_append(tb, NULL, "restriction icons", NULL, NULL);
    tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);  elm_toolbar_item_separator_set(tb_it, EINA_TRUE);
    tb_it = elm_toolbar_item_append(tb, NULL, "hop://localhost/Anarchadia 152, 155, 51 - Lost plot (Adult)", NULL, NULL);
    tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);  elm_toolbar_item_separator_set(tb_it, EINA_TRUE);
    tb_it = elm_toolbar_item_append(tb, NULL, "date time:o'clock", NULL, NULL);

    evas_object_show(tb);
}

// Elm inlined image windows needs this to change focus on mouse click.
// Evas style event callback.
static void _cb_mouse_down_elm(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Down *ev = event_info;

    if (1 == ev->button)
	elm_object_focus_set(obj, EINA_TRUE);
}

void overlay_add(globals *ourGlobals)
{
  GLData *gld = &ourGlobals->gld;
  Evas_Object *bg;

  // There are many reasons for this window.
  // The first is to cover the GL and provide something to click on to change focus.
  // The second is to provide something to click on for all the GL type clicking stuff that needs to be done.  In other words, no click through, we catch the clicks here.
  //   So we can probably avoid the following issue -
  //     How to do click through?  evas_object_pass_events_set(rectangle, EINA_TRUE), and maybe need to do that to the underlaying window to?
  //     Though if the rectangle is entirely transparent, or even hidden, events might pass through anyway.
  //   Gotta have click through on the parts where there's no other window.
  // The third is to have the other windows live here.
  //   This idea doesn't work, as it breaks the damn focus again.
  //   Don't think it's needed anyway.
  // While on the subject of layers, need a HUD layer of some sort, but Irrlicht might support that itself.

  gld->winwin = elm_win_add(ourGlobals->win, "inlined", ELM_WIN_INLINED_IMAGE);
  // On mouse down we try to shift focus to the backing image, this seems to be the correct thing to force focus onto it's widgets.
  // According to the Elm inlined image window example, this is what's needed to.
  evas_object_event_callback_add(elm_win_inlined_image_object_get(gld->winwin), EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_elm, NULL);
  // In this code, we are making our own camera, so grab it's input when we are focused.
//  cameraAdd(ourGlobals, gld->winwin);

  elm_win_alpha_set(gld->winwin, EINA_TRUE);
  // Apparently transparent is not good enough for ELM backgrounds, so make it a rectangle.
  // Apparently coz ELM prefers stuff to have edjes.  A bit over the top if all I want is a transparent rectangle.
  bg = evas_object_rectangle_add(evas_object_evas_get(gld->winwin));
  evas_object_color_set(bg, 0, 0, 0, 0);
  evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(gld->winwin, bg);
  evas_object_show(bg);

  // image object for win is unlinked to its pos/size - so manual control
  // this allows also for using map and other things with it.
  evas_object_move(elm_win_inlined_image_object_get(gld->winwin), 0, 0);
  // Odd, it needs to be resized twice.  WTF?
  evas_object_resize(gld->winwin, ourGlobals->win_w, ourGlobals->win_h);
  evas_object_resize(elm_win_inlined_image_object_get(gld->winwin), ourGlobals->win_w, ourGlobals->win_h);
  evas_object_show(gld->winwin);
}


EAPI_MAIN int elm_main(int argc, char **argv)
{
  GLData *gld = NULL;
  char buf[PATH_MAX * 2];
//  Eina_Bool gotWebKit = elm_need_web();	// Initialise ewebkit if it exists, or return EINA_FALSE if it don't.

  logDom = HamrTime(argv[0], elm_main, logDom);

  /* Set the locale according to the system pref.
   * If you don't do so the file selector will order the files list in
   * a case sensitive manner
   */
  setlocale(LC_ALL, "");

  elm_need_ethumb();
  elm_need_efreet();

  ourGlobals.running = 1;


  // Don't do this, we need to clean up other stuff to, so set a clean up function below.
  //elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
  elm_policy_set(ELM_POLICY_EXIT,	ELM_POLICY_EXIT_NONE);
  elm_policy_set(ELM_POLICY_QUIT,	ELM_POLICY_QUIT_NONE);
  elm_policy_set(ELM_POLICY_THROTTLE,	ELM_POLICY_THROTTLE_HIDDEN_ALWAYS);

  // These are set via the elementary_config tool, which is hard to find.
  elm_config_finger_size_set(0);
  elm_config_scale_set(1.0);

  if (!ephysics_init())
    return 1;

  gld = &ourGlobals.gld;
  gldata_init(gld);

  // One or more of these lets us use the 3D stuff.
  setenv("ELM_ENGINE", "opengl_x11", 1);
  setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);
  elm_config_accel_preference_set("3d");
  ourGlobals.mainWindow = winFangAdd(NULL, 0, 0, 50, 20, "extantz virtual world viewer", "extantz", NULL);

  ourGlobals.win = ourGlobals.mainWindow->win;
  // TODO, or not TODO - I keep getting rid of these, but keep bringing them back.
  // Get the Evas / canvas from the elm window (that the Evas_Object "lives on"), which is itself an Evas_Object created by Elm, so not sure if it was created internally with Ecore_Evas.
  ourGlobals.evas = evas_object_evas_get(ourGlobals.win);
  // An Ecore_Evas holds an Evas.
  // Get the Ecore_Evas that wraps an Evas.
#if USE_IRR
  ourGlobals.ee = ecore_evas_ecore_evas_get(ourGlobals.evas);	// Only use this on Evas that was created with Ecore_Evas.
#endif

  // Get the screen size.
  elm_win_screen_size_get(ourGlobals.win, &ourGlobals.win_x, &ourGlobals.win_y, &ourGlobals.scr_w, &ourGlobals.scr_h);
  ourGlobals.win_x = ourGlobals.win_x + (ourGlobals.scr_w / 3);
  ourGlobals.win_w = ourGlobals.scr_w / 2;
  ourGlobals.win_h = ourGlobals.scr_h - 30;
  evas_object_move(ourGlobals.win, ourGlobals.win_x, ourGlobals.win_y);
  evas_object_resize(ourGlobals.win, ourGlobals.win_w, ourGlobals.win_h);

  evas_object_event_callback_add(ourGlobals.win, EVAS_CALLBACK_RESIZE, _on_resize, &ourGlobals);

  /*  Our "layers".  TODO - This is out of date, I should update it.

      Elm win		- our real main window
        These have some sort of inlined image if they are internal.
        Elm image	- purple shaded sky image
        Elm box		- so everyone gets a freebie

      Elm glview	- added by init_evas_gl() if this is Irrlicht

      Elm image		- added by Evas_3D_Demo_add() -> scenriAdd()
        Evas image is extracted to pass to Evas_3d functions,
          to catch hovers and clicks
          and to catch input for the camera

//      Elm win		- added by overlay_add()
//        Evas rectangle added, a fully transparent one to catch clicks
//          added to  evas_object_evas_get(gld->winwin)

      The various internal windows.

      Elm toolbar

      Ephysics objects
  */

  // Override the background image
#if 1
  snprintf(buf, sizeof(buf), "%s/sky_03.jpg", prefix_data_get());
  ourGlobals.mainWindow->bg = eo_add(ELM_IMAGE_CLASS, ourGlobals.mainWindow->win,
    evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
    elm_obj_image_fill_outside_set(EINA_TRUE),
    efl_file_set(buf, NULL),
    efl_gfx_visible_set(EINA_TRUE)
  );
  elm_win_resize_object_add(ourGlobals.mainWindow->win, ourGlobals.mainWindow->bg);
#else
  snprintf(buf, sizeof(buf), "%s/sky_03.jpg", prefix_data_get());
  eo_do(ourGlobals.mainWindow->bg,
    elm_obj_image_file_set(buf, NULL),
    efl_gfx_color_set(255, 255, 255, 255)
  );
#endif

  init_evas_gl(&ourGlobals);

#if USE_EVAS_3D
  // Setup our Evas_3D stuff.
  ourGlobals.scene = scenriAdd(ourGlobals.win);
  // TODO - Just a temporary hack so Irrlicht and Evas_3D can share the camera move.
  ourGlobals.gld.move = ourGlobals.scene->move;
  evas_object_data_set(elm_image_object_get(ourGlobals.scene->image), "glob", &ourGlobals);
  evas_object_image_pixels_get_callback_set(elm_image_object_get(ourGlobals.scene->image), on_pixels, &ourGlobals);
  ourGlobals.scene->clickCb = _onWorldClick;
#endif

  // Gotta do this after adding the windows, otherwise the menu renders under the window.
  //   This sucks, gotta redefine this menu each time we create a new window?
  // Also, GL focus gets lost when any menu is used.  sigh
  makeMainMenu(&ourGlobals);

  ourGlobals.world = ephysicsAdd(&ourGlobals);

//  overlay_add(&ourGlobals);
//  GuiLuaLoad("test", ourGlobals.mainWindow, ourGlobals.world);
  woMan_add(&ourGlobals);
  ourGlobals.purkle     = GuiLuaLoad("purkle",     ourGlobals.mainWindow, ourGlobals.world);
  ourGlobals.LSLGuiMess = GuiLuaLoad("LSLGuiMess", ourGlobals.mainWindow, ourGlobals.world);
  ourGlobals.files = filesAdd(&ourGlobals, (char *) prefix_data_get(), EINA_TRUE, EINA_FALSE);

  // Try to connect to a local love server.
  serverStream = eina_strbuf_new();
  reachOut("127.0.0.1", 8211 + 1, &ourGlobals, (Ecore_Event_Handler_Cb) _add, (Ecore_Event_Handler_Cb) _data, (Ecore_Event_Handler_Cb) _del);

   // Bump the top toolbar above the windows.
  evas_object_raise(ourGlobals.tb);

  _on_resize(&ourGlobals, NULL, NULL, NULL);

  elm_run();
  ourGlobals.running = 0;

  ephysics_world_del(ourGlobals.world);
  ephysics_shutdown();

  if (ourGlobals.server)  ecore_con_server_del(ourGlobals.server);

  if (ourGlobals.win)
  {
    ecore_animator_del(ourGlobals.animator);
#if USE_EVAS_3D
    Evas_3D_Demo_fini(&ourGlobals);
    scenriDel(ourGlobals.scene);
#endif
//    eo_unref(ourGlobals.tb);
    winFangDel(ourGlobals.mainWindow);
  }

  pantsOff(logDom);
  logDom = -1;

  elm_shutdown();

  return 0;
}
ELM_MAIN()
