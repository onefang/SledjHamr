#include "extantz.h"


globals ourGlobals;


static void gldata_init(GLData *gld)
{
    gld->useIrr = USE_IRR;

    gld->view_rotx = -20.0;
    gld->view_roty = -30.0;
    gld->view_rotz = 0.0;
    gld->angle = 0.0;

    gld->light[0] = 1.0;
    gld->light[1] = 1.0;
    gld->light[2] = -5.0;
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
   int w, h;
   GLData *gld = evas_object_data_get(obj, "gld");

   elm_glview_size_get(obj, &w, &h);

   gld->img_w = w;
   gld->img_h = h;
   _resize(gld);
}

static void _on_resize(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
  globals *ourGlobals = data;
  GLData *gld = &ourGlobals->gld;

  eo_do(gld->win, evas_obj_size_get(&gld->win_w, &gld->win_h));
  eo_do(ourGlobals->tb, evas_obj_size_set(gld->win_w, 25));
  _resize(gld);
}

// Callback from Evas, also used as the general callback for deleting the GL stuff.
static void _clean_gl(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    GLData *gld = data;

    ecore_animator_del(gld->animator);

    // Do a make_current before deleting all the GL stuff.
//    evas_gl_make_current(NULL, NULL, NULL);

    // TODO - Since this is created on the render thread, better hope this is being deleted on the render thread.
    finishIrr(gld);

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
}

// Callback from Elm, coz they do shit different.
static void _del_gl(Evas_Object *obj)
{
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld)
     {
        printf("Unable to get GLData. \n");
        return;
     }

    _clean_gl(gld, NULL, NULL, NULL);

   evas_object_data_del((Evas_Object*)obj, "gld");
}

// Callback for when the app quits.
static void _on_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
//    GLData *gld = data;

    elm_exit();
}

static void _draw_gl(Evas_Object *obj)
{
  globals *ourGlobals = evas_object_data_get(obj, "glob");
  GLData *gld = &ourGlobals->gld;
  if (!ourGlobals)  return;

  if (!gld->doneIrr)		gld->doneIrr = startIrr(gld);	// Needs to be after gld->win is shown, and needs to be done in the render thread.
#if DO_GEARS
  if (!gld->gearsInited)	gears_init(gld);
#endif

//  if (gld->resized)		_resize(gld);

  drawIrr_start(gld);

#if DO_GEARS
  drawGears(gld);
#endif

  _animate_scene(ourGlobals);

  drawIrr_end(gld);

#if USE_IRR
#else
  // This might get done deep within drawIrr_end, but only if we are using Irrlicht.

  // Optional - Flush the GL pipeline
//  gl->glFlush();
//  gl->glFinish();
#endif

  gld->resized = 0;
}

static void on_pixels(void *data, Evas_Object *obj)
{
  _draw_gl(obj);
}

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
  else if (ourGlobals->scene->image)
  {
//    evas_object_image_pixels_dirty_set(elm_image_object_get(ourGlobals->scene->image), EINA_TRUE);
    _draw_gl(elm_image_object_get(ourGlobals->scene->image));
  }

  return EINA_TRUE;	// Keep calling us.
}

static void init_evas_gl(globals *ourGlobals)
{
  GLData *gld = &ourGlobals->gld;

  gld->sfc_w = gld->win_w;
  gld->sfc_h = gld->win_h;

  if (USE_IRR || DO_GEARS)
  {
    gld->sfc_h = gld->win_h;

    // Get the Evas / canvas from the elm window (that the Evas_Object "lives on"), which is itself an Evas_Object created by Elm, so not sure if it was created internally with Ecore_Evas.
    gld->canvas = evas_object_evas_get(gld->win);
    // An Ecore_Evas holds an Evas.
    // Get the Ecore_Evas that wraps an Evas.
    gld->ee = ecore_evas_ecore_evas_get(gld->canvas);	// Only use this on Evas that was created with Ecore_Evas.

    // Add a GLView
    gld->elmGl = elm_glview_add(gld->win);
    evas_object_size_hint_align_set(gld->elmGl, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(gld->elmGl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_glview_mode_set(gld->elmGl, 0 | ELM_GLVIEW_ALPHA | ELM_GLVIEW_DEPTH | ELM_GLVIEW_DIRECT);
    elm_glview_resize_policy_set(gld->elmGl, ELM_GLVIEW_RESIZE_POLICY_RECREATE);		// Destroy the current surface on a resize and create a new one.
    elm_glview_render_policy_set(gld->elmGl, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);
//    elm_glview_render_policy_set(gld->elmGl, ELM_GLVIEW_RENDER_POLICY_ALWAYS);
    // These get called in the render thread I think.
    // None let me pass data, so this is why we are adding "gld" data to the object below.
    // Maybe we can use  elm_object_signal_callback_add or elm_object_item_signal_callback_add (edje signals)?
    //elm_glview_init_func_set(gld->elmGl, _init_gl);	// Not actually needed, it gets done in on_pixels.
    elm_glview_del_func_set(gld->elmGl, _del_gl);
    elm_glview_resize_func_set(gld->elmGl, _resize_gl);
    elm_glview_render_func_set(gld->elmGl, (Elm_GLView_Func_Cb) _draw_gl);

    // Not needed, the resize callback above deals with that.
    //elm_win_resize_object_add(gld->win, gld->elmGl);
    gld->glApi = elm_glview_gl_api_get(gld->elmGl);
    evas_object_data_set(gld->elmGl, "gld", gld);
    evas_object_data_set(gld->elmGl, "glob", ourGlobals);
    evas_object_show(gld->elmGl);
    elm_box_pack_end(gld->bx, gld->elmGl);
  }

  // TODO - apparently the proper way to deal with the new async rendering is to have this animator do the dirty thing, and call the Irrlicht rendering stuff in the _draw_gl call set above.
  //        That still leaves the problem of the Irrlicht setup being in the main thread.  Which also should be done in on_pixel, as that's done in the correct thread.

  // Jiggling this seems to produce a trade off between flickering and frame rate.  Nothing else changed the flickering.
  ecore_animator_frametime_set(0.04);	// Default is 1/30, or 0.033333
  gld->animator = ecore_animator_add(doFrame, ourGlobals);	// This animator will be called every frame tick, which defaults to 1/30 seconds.

  return;
}


//-------------------------//


static Evas_Object *_toolbar_menu_add(Evas_Object *win, Evas_Object *tb, char *label)
{
    Evas_Object *menu= NULL;
    Elm_Object_Item *tb_it;

    tb_it = elm_toolbar_item_append(tb, NULL, label, NULL, NULL);
    elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
    // Priority is for when toolbar items are set to hide or menu when there are too many of them.  They get hidden or put on the menu based on priority.
    elm_toolbar_item_priority_set(tb_it, 9999);
    elm_toolbar_menu_parent_set(tb, win);
    menu = elm_toolbar_item_menu_get(tb_it);

    return menu;
}

static void makeMainMenu(globals *ourGlobals)
{
    GLData *gld = &ourGlobals->gld;
    Evas_Object *menu, *tb;
    Elm_Object_Item *tb_it;

    // A toolbar thingy.
    tb = eo_add(ELM_OBJ_TOOLBAR_CLASS, gld->win);
    ourGlobals->tb = tb;
    eo_do(tb,
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, 0.0),
	evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
	elm_obj_toolbar_shrink_mode_set(ELM_TOOLBAR_SHRINK_MENU),
	evas_obj_size_set(gld->win_w, 25),
	evas_obj_position_set(0, 0),
	elm_obj_toolbar_align_set(0.0)
	);

    // Menus.
    menu = _toolbar_menu_add(gld->win, tb, "file");
    elm_menu_item_add(menu, NULL, NULL, "quit", _on_done, gld);

    menu = _toolbar_menu_add(gld->win, tb, "edit");
    elm_menu_item_add(menu, NULL, NULL, "preferences", NULL, NULL);

    menu = _toolbar_menu_add(gld->win, tb, "view");
    menu = _toolbar_menu_add(gld->win, tb, "world");
    menu = _toolbar_menu_add(gld->win, tb, "tools");

    menu = _toolbar_menu_add(gld->win, tb, "help");
    elm_menu_item_add(menu, NULL, NULL, "grid help", NULL, NULL);
    elm_menu_item_separator_add(menu, NULL);
    elm_menu_item_add(menu, NULL, NULL, "extantz blogs", NULL, NULL);
    elm_menu_item_add(menu, NULL, NULL, "extantz forum", NULL, NULL);
    elm_menu_item_separator_add(menu, NULL);
    elm_menu_item_add(menu, NULL, NULL, "about extantz", NULL, NULL);

    menu = _toolbar_menu_add(gld->win, tb, "advanced");
    elm_menu_item_add(menu, NULL, NULL, "debug settings", NULL, NULL);

    menu = _toolbar_menu_add(gld->win, tb, "god");

    // Other stuff in the toolbar.
    tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
    elm_toolbar_item_separator_set(tb_it, EINA_TRUE);
    tb_it = elm_toolbar_item_append(tb, NULL, "restriction icons", NULL, NULL);
    tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
    elm_toolbar_item_separator_set(tb_it, EINA_TRUE);
    tb_it = elm_toolbar_item_append(tb, NULL, "hop://localhost/Anarchadia 152, 155, 51 - Lost plot (Adult)", NULL, NULL);
    tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
    elm_toolbar_item_separator_set(tb_it, EINA_TRUE);
    tb_it = elm_toolbar_item_append(tb, NULL, "date time:o'clock", NULL, NULL);

    evas_object_show(tb);
}

EAPI_MAIN int elm_main(int argc, char **argv)
{
    Evas_Object *obj;
    EPhysics_World *world;
    GLData *gld = NULL;
    char buf[PATH_MAX];
//    Eina_Bool gotWebKit = elm_need_web();	// Initialise ewebkit if it exists, or return EINA_FALSE if it don't.

    HamrTime(elm_main, "extantz");
    fprintf(stdout, "prefix was set to: %s\n", elm_app_prefix_dir_get());
    fprintf(stdout, "data directory is: %s\n", elm_app_data_dir_get());
    fprintf(stdout, "library directory is: %s\n", elm_app_lib_dir_get());
    fprintf(stdout, "locale directory is: %s\n", elm_app_locale_dir_get());

    ourGlobals.logDom = loggingStartup("extantz", ourGlobals.logDom);

    // Don't do this, we need to clean up other stuff to, so set a clean up function below.
    //elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    elm_policy_set(ELM_POLICY_EXIT,	ELM_POLICY_EXIT_NONE);
    elm_policy_set(ELM_POLICY_QUIT,	ELM_POLICY_QUIT_NONE);
    elm_policy_set(ELM_POLICY_THROTTLE,	ELM_POLICY_THROTTLE_HIDDEN_ALWAYS);

    // These are set via the elementary_config tool, which is hard to find.
    elm_config_finger_size_set(0);
    elm_config_scale_set(1.0);

    gld = &ourGlobals.gld;
    gldata_init(gld);

    // Set the engine to opengl_x11, then open our window.
    elm_config_preferred_engine_set("opengl_x11");
    gld->win = elm_win_util_standard_add("extantz", "extantz virtual world viewer");
    ourGlobals.win = gld->win;
    // Get the Evas / canvas from the elm window (that the Evas_Object "lives on"), which is itself an Evas_Object created by Elm, so not sure if it was created internally with Ecore_Evas.
    ourGlobals.evas = evas_object_evas_get(gld->win);

    // Set preferred engine back to default from config
    elm_config_preferred_engine_set(NULL);

#if USE_PHYSICS
    if (!ephysics_init())
	return 1;
#endif

    evas_object_smart_callback_add(gld->win, "delete,request", _on_done, gld);
    evas_object_event_callback_add(gld->win, EVAS_CALLBACK_RESIZE, _on_resize, &ourGlobals);

    // Get the screen size.
    elm_win_screen_size_get(gld->win, &gld->win_x, &gld->win_y, &gld->scr_w, &gld->scr_h);
    gld->win_x = gld->win_x + (gld->scr_w / 3);
    gld->win_w = gld->scr_w / 2;
    gld->win_h = gld->scr_h - 30;

    // Add a background image object.
    obj = eo_add(ELM_OBJ_IMAGE_CLASS, gld->win);
    snprintf(buf, sizeof(buf), "%s/sky_03.jpg", elm_app_data_dir_get());
    eo_do(obj,
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
	elm_obj_image_fill_outside_set(EINA_TRUE),
	elm_obj_image_file_set(buf, NULL),
	evas_obj_visibility_set(EINA_TRUE)
	);
    elm_win_resize_object_add(gld->win, obj);
    eo_unref(obj);

    gld->bx = eo_add(ELM_OBJ_BOX_CLASS, gld->win);
    eo_do(gld->bx,
	evas_obj_size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
	evas_obj_size_hint_align_set(EVAS_HINT_FILL, EVAS_HINT_FILL),
	evas_obj_visibility_set(EINA_TRUE)
	);
    elm_win_resize_object_add(gld->win, gld->bx);

//    overlay_add(gld);
    woMan_add(gld);
    chat_add(gld);

    // Gotta do this after adding the windows, otherwise the menu renders under the window.
    //   This sucks, gotta redefine this menu each time we create a new window?
    // Also, GL focus gets lost when any menu is used.  sigh
    makeMainMenu(&ourGlobals);

    // This does elm_box_pack_end(), so needs to be after the others.
    init_evas_gl(&ourGlobals);

    Evas_3D_Demo_add(&ourGlobals);
    evas_object_data_set(elm_image_object_get(ourGlobals.scene->image), "glob", &ourGlobals);
    evas_object_image_pixels_get_callback_set(elm_image_object_get(ourGlobals.scene->image), on_pixels, &ourGlobals);

#if USE_PHYSICS
    world = ephysicsAdd(gld);
#endif

    evas_object_move(gld->win, gld->win_x, gld->win_y);
    evas_object_resize(gld->win, gld->win_w, gld->win_h);
    evas_object_show(gld->win);

    _resize_winwin(gld);

    elm_run();

#if USE_PHYSICS
    ephysics_world_del(world);
    ephysics_shutdown();
#endif

    if (gld->win)
    {
	Evas_3D_Demo_fini(&ourGlobals);
	eo_unref(ourGlobals.tb);
	eo_unref(gld->bx);
	evas_object_del(gld->win);
    }

    if (ourGlobals.logDom >= 0)
    {
	eina_log_domain_unregister(ourGlobals.logDom);
	ourGlobals.logDom = -1;
    }

    elm_shutdown();

    return 0;
}
ELM_MAIN()
