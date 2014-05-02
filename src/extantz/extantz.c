#include "extantz.h"


globals ourGlobals;



static void gldata_init(GLData *gld)
{
    gld->useEGL = USE_EGL;
    gld->useIrr = USE_IRR;

    gld->view_rotx = -20.0;
    gld->view_roty = -30.0;
    gld->view_rotz = 0.0;
    gld->angle = 0.0;

    gld->light[0] = 1.0;
    gld->light[1] = 1.0;
    gld->light[2] = -5.0;
}

//-------------------------//



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

static void on_pixels(void *data, Evas_Object *obj)
{
    GLData *gld = data;
    Evas_GL_API *gl = gld->glApi;

    // get the image size in case it changed with evas_object_image_size_set()
    if (gld->r1)
    {
	Evas_Coord    w, h;

	// Poor mans resize check. coz Elm wont do it easily.
	evas_object_image_size_get(gld->r1, &w, &h);
	if ((gld->img_w != w) || (gld->img_h != h))
	{
	    // No idea where this crap came from.
	    //float new_w = ((float) gld->scr_w / ((float) gld->scr_w * (float) w));
	    //float new_h = ((float) gld->scr_h / ((float) gld->scr_h * (float) h));
	
	    //gld->sfc_w = new_w;
	    //gld->sfc_h = new_h;
	    //evas_object_image_fill_set(gld->r1, 0, 0, gld->sfc_w, gld->sfc_h);
	    gld->img_w = w;
	    gld->img_h = h;
	    gld->resized = 1;
	}
    }

    if (gld->useEGL)
    {
	// Yes, we DO need to do our own make current, coz aparently the Irrlicht one is useless.
	// Hopefully Elm_GL has done this for us by now.
	// Evas_GL needs it to.
	if (gld->ctx)
	    evas_gl_make_current(gld->evasGl, gld->sfc, gld->ctx);
    }

    if (!gld->doneIrr)
	gld->doneIrr = startIrr(gld);	// Needs to be after gld->win is shown, and needs to be done in the render thread.
#if DO_GEARS
    if (!gld->gearsInited)
	gears_init(gld);
#endif

    if (gld->resized)
	_resize(gld);

    drawIrr_start(gld);

#if DO_GEARS
    if (gld->useEGL)
	drawGears(gld);
#endif

    drawIrr_end(gld);

#if USE_IR
#else
   // This might get done deep within drawIrr_end, but only if we are using Irrlicht.

   // Optional - Flush the GL pipeline
   gl->glFlush();
//   gl->glFinish();
#endif

    gld->resized = 0;
}

static void _draw_gl(Evas_Object *obj)
{
//   Evas_GL_API *gl = elm_glview_gl_api_get(obj);
   GLData *gld = evas_object_data_get(obj, "gld");
   if (!gld) return;

   on_pixels(gld, obj);
}

// Callback from Evas, also used as the general callback for deleting the GL stuff.
static void _clean_gl(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    GLData *gld = data;
    Evas_GL_API *gl = gld->glApi;

    ecore_animator_del(gld->animator);

    if (gld->useEGL)
    {
	// Do a make_current before deleting all the GL stuff.
	evas_gl_make_current(gld->evasGl, gld->sfc, gld->ctx);

    }

    gl->glDeleteShader(gld->vtx_shader);
    gl->glDeleteShader(gld->fgmt_shader);
    gl->glDeleteProgram(gld->program);

    if (gld->evasGl)
    {
	// Irrlicht wants to destroy the context and surface, so only do this if Irrlicht wont.
	if (!gld->doneIrr)
	{
	    evas_gl_surface_destroy(gld->evasGl, gld->sfc);
	    evas_gl_context_destroy(gld->evasGl, gld->ctx);
	}
	// TODO - hope this is OK, considering the context and surface might get dealt with by Irrlicht.
	// Might be better to teach Irrlicht to not destroy shit it did not create.
	evas_gl_config_free(gld->cfg);
	evas_gl_free(gld->evasGl);
    }

    // TODO - Since this is created on the render thread, better hope this is being deleted on the render thread.
    finishIrr(gld);

#if DO_GEARS
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

// Callback from the animator.
static Eina_Bool doFrame(void *data)
{
    GLData *gld = data;

    // Mark the pixels as dirty, so they get rerendered each frame, then Irrlicht can draw it's stuff each frame.
    // This causes on_pixel to be triggered by Evas_GL, and _draw_gl for Elm_glview.
    if (gld->r1)
	evas_object_image_pixels_dirty_set(gld->r1, EINA_TRUE);
    if (gld->elmGl)
	elm_glview_changed_set(gld->elmGl);

    // If not using Evas_GL, we need to call on_pixel() manually.
    if (!gld->useEGL)
	on_pixels(gld, gld->r1);

    return EINA_TRUE;	// Keep calling us.
}

static void init_evas_gl(GLData *gld)
{
    if (!gld->useEGL)
	return;

    gld->sfc_w = gld->win_w;
    gld->sfc_h = gld->win_h;

    // Get the Evas / canvas from the elm window (that the Evas_Object "lives on"), which is itself an Evas_Object created by Elm, so not sure if it was created internally with Ecore_Evas.
    gld->canvas = evas_object_evas_get(gld->win);
    // An Ecore_Evas holds an Evas.
    // Get the Ecore_Evas that wraps an Evas.
    gld->ee = ecore_evas_ecore_evas_get(gld->canvas);	// Only use this on Evas that was created with Ecore_Evas.

#if USE_ELM_GL
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
    evas_object_show(gld->elmGl);
    elm_box_pack_end(gld->bx, gld->elmGl);
#else
    // get the evas gl handle for doing gl things
    gld->evasGl = evas_gl_new(gld->canvas);
    gld->glApi = evas_gl_api_get(gld->evasGl);

    // Set a surface config
    gld->cfg = evas_gl_config_new();
    gld->cfg->color_format = EVAS_GL_RGBA_8888;
    gld->cfg->depth_bits   = EVAS_GL_DEPTH_BIT_32;
    gld->cfg->stencil_bits = EVAS_GL_STENCIL_NONE;
    gld->cfg->options_bits = EVAS_GL_OPTIONS_DIRECT;

    // create a surface and context
    gld->sfc = evas_gl_surface_create(gld->evasGl, gld->cfg, gld->sfc_w, gld->sfc_h);
    gld->ctx = evas_gl_context_create(gld->evasGl, NULL);		// The second NULL is for sharing contexts I think, which might be what we want to do with Irrlicht.  It's not documented.

    // Set up the image object, a filled one by default.
    gld->r1 = evas_object_image_filled_add(gld->canvas);

   // attach important data we need to the object using key names. This just
   // avoids some global variables and means we can do nice cleanup. You can
   // avoid this if you are lazy
   // Not actually needed, with evas we can pass data pointers to stuff.
    //evas_object_data_set(gld->r1, "gld", gld);

    // when the object is deleted - call the on_del callback. like the above,
    // this is just being clean
    evas_object_event_callback_add(gld->r1, EVAS_CALLBACK_DEL, _clean_gl, gld);

    // set up an actual pixel size for the buffer data. it may be different
    // to the output size. any windowing system has something like this, just
    // evas has 2 sizes, a pixel size and the output object size
    evas_object_image_size_set(gld->r1, gld->sfc_w, gld->sfc_h);
    // Not actualy needed, as we create the image already filled.
    //evas_object_image_fill_set(gld->r1, 0, 0, gld->sfc_w, gld->sfc_h);

    // These two are not in the original example, but I get black r1 when I leave them out.
    evas_object_size_hint_align_set(gld->r1, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(gld->r1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    // set up the native surface info to use the context and surface created
    // above
    evas_gl_native_surface_get(gld->evasGl, gld->sfc, &(gld->ns));
    evas_object_image_native_surface_set(gld->r1, &(gld->ns));
    evas_object_image_pixels_get_callback_set(gld->r1, on_pixels, gld);

   // move the image object somewhere, resize it and show it. any windowing
   // system would need this kind of thing - place a child "window"
   // Hmm, no need to resize it anyway, it's sized above.
    evas_object_move(gld->r1, 0, 0);
    //evas_object_resize(gld->r1, gld->sfc_w, gld->sfc_h);
    elm_win_resize_object_add(gld->win, gld->r1);
    evas_object_show(gld->r1);
    elm_box_pack_end(gld->bx, gld->r1);

//    evas_object_event_callback_add(gld->r1, EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down_GL, gld);
//    evas_object_event_callback_add(gld->r1, EVAS_CALLBACK_KEY_DOWN, _on_camera_input_down, gld);
//    evas_object_event_callback_add(gld->r1, EVAS_CALLBACK_KEY_UP,   _on_camera_input_up, gld);
#endif

    // NOTE: if you delete r1, this animator will keep running trying to access
    // r1 so you'd better delete this animator with ecore_animator_del() or
    // structure how you do animation differently. you can also attach it like
    // evasGl, sfc, etc. etc. if this animator is specific to this object
    // only and delete it in the del handler for the obj.
    //
    // TODO - apparently the proper way to deal with the new async rendering is to have this animator do the dirty thing, and call the Irrlicht rendering stuff in the on_pixel call set above.
    //        That still leaves the problem of the Irrlicht setup being in the main thread.  Which also should be done in on_pixel, as that's done in the correct thread.
    
    // Jiggling this seems to produce a trade off between flickering and frame rate.  Nothing else changed the flickering.
    ecore_animator_frametime_set(0.04);	// Default is 1/30, or 0.033333
    gld->animator = ecore_animator_add(doFrame, gld);	// This animator will be called every frame tick, which defaults to 1/30 seconds.

    return;
}


//-------------------------//


static Evas_Object *_toolbar_menu_add(Evas_Object *win, Evas_Object *tb, char *label)
{
    Evas_Object *menu= NULL;
    Elm_Object_Item *tb_it;
//, *menu_it;

    tb_it = elm_toolbar_item_append(tb, NULL, label, NULL, NULL);
    elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
    // Priority is for when toolbar items are set to hide or menu when there are too many of them.  They get hidden or put on the menu based on priority.
    elm_toolbar_item_priority_set(tb_it, 9999);
    elm_toolbar_menu_parent_set(tb, win);
    menu = elm_toolbar_item_menu_get(tb_it);

    return menu;
}


EAPI_MAIN int elm_main(int argc, char **argv)
{
    Evas_Object *obj, *menu, *tb;
    Elm_Object_Item *tb_it;
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

    // alloc a data struct to hold our relevant gl info in
    if (!(gld = calloc(1, sizeof(GLData)))) return 1;
    gldata_init(gld);

    // Set the engine to opengl_x11, then open our window.
    if (gld->useEGL)
	elm_config_preferred_engine_set("opengl_x11");
    gld->win = elm_win_add(NULL, "extantz", ELM_WIN_BASIC);
    gld->win = elm_win_util_standard_add("extantz", "extantz virtual world viewer");
    ourGlobals.win = gld->win;

    // Set preferred engine back to default from config
    elm_config_preferred_engine_set(NULL);

#if USE_PHYSICS
    if (!ephysics_init())
	return 1;
#endif

    evas_object_smart_callback_add(gld->win, "delete,request", _on_done, gld);

    // Get the screen size.
    elm_win_screen_size_get(gld->win, &gld->win_x, &gld->win_y, &gld->scr_w, &gld->scr_h);
    gld->win_x = gld->win_x + (gld->scr_w / 3);
    gld->win_w = gld->scr_w / 2;
    gld->win_h = gld->scr_h - 30;

    // Get the Evas / canvas from the elm window (that the Evas_Object "lives on"), which is itself an Evas_Object created by Elm, so not sure if it was created internally with Ecore_Evas.
    ourGlobals.evas = evas_object_evas_get(gld->win);

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

    Evas_3D_Demo_add(&ourGlobals);

    gld->bx = elm_box_add(gld->win);
    evas_object_size_hint_weight_set(gld->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(gld->bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_win_resize_object_add(gld->win, gld->bx);
    evas_object_show(gld->bx);

    overlay_add(gld);
    woMan_add(gld);
    // TODO - This is what causes it to hang after quitting.  Fix it.
//    chat_add(gld);

    // Gotta do this after adding the windows, otherwise the menu renders under the window.
    //   This sucks, gotta redefine this menu each time we create a new window?
    // Also, GL focus gets lost when any menu is used.  sigh

    // A toolbar thingy.
    tb = elm_toolbar_add(gld->win);
    evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_SCROLL);
    elm_toolbar_align_set(tb, 0.0);

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

    // The toolbar needs to be packed into the box AFTER the menus are added.
    evas_object_show(tb);
    elm_box_pack_start(gld->bx, tb);

    // This does elm_box_pack_end(), so needs to be after the others.
    init_evas_gl(gld);

    evas_object_show(gld->bx);

#if USE_PHYSICS
    // ePhysics stuff.
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
	evas_object_del(gld->win);
    }
    free(gld);

    if (ourGlobals.logDom >= 0)
    {
	eina_log_domain_unregister(ourGlobals.logDom);
	ourGlobals.logDom = -1;
    }

    elm_shutdown();

    return 0;
}
ELM_MAIN()
