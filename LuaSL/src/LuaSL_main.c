#include "LuaSL.h"


static const char *names[] =
{
     "bub1", "sh1",
     "bub2", "sh2",
     "bub3", "sh3",
};


static void
_edje_signal_cb(void *data, Evas_Object *obj __UNUSED__, const char  *emission, const char  *source)
{
//    gameGlobals *game = data;
}

static
Eina_Bool anim(void *data)
{
    gameGlobals *game = data;
    Evas_Object *bub, *sh;
    Evas_Coord x, y, w, h, vw, vh;
    double t, xx, yy, zz, r, fac;
    double lx, ly;
    unsigned int i;

    evas_output_viewport_get(game->canvas, 0, 0, &vw, &vh);
    r = 48;
    t = ecore_loop_time_get();
    fac = 2.0 / (double)((sizeof(names) / sizeof(char *) / 2));
    evas_pointer_canvas_xy_get(game->canvas, &x, &y);
    lx = x;
    ly = y;

    for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
    {
	bub = evas_object_data_get(game->bg, names[i * 2]);
	sh = evas_object_data_get(game->bg, names[(i * 2) + 1]);
	zz = (((2 + sin(t * 6 + (M_PI * (i * fac)))) / 3) * 64) * 2;
	xx = (cos(t * 4 + (M_PI * (i * fac))) * r) * 2;
	yy = (sin(t * 6 + (M_PI * (i * fac))) * r) * 2;

	w = zz;
	h = zz;
	x = (vw / 2) + xx - (w / 2);
	y = (vh / 2) + yy - (h / 2);

	evas_object_move(bub, x, y);
	evas_object_resize(bub, w, h);

	x = x - ((lx - (x + (w / 2))) / 4);
	y = y - ((ly - (y + (h / 2))) / 4);

	evas_object_move(sh, x, y);
	evas_object_resize(sh, w, h);
	evas_object_raise(sh);
	evas_object_raise(bub);
    }
    return ECORE_CALLBACK_RENEW;
}

static void
_on_delete(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

void dirList_cb(const char *name, const char *path, void *data)
{
    gameGlobals *game = data;
    char buf[PATH_MAX];
    char *ext = rindex(name, '.');

    if (ext)
    {
	if (0 == strcmp(ext, ".lsl"))
	{
	    snprintf(buf, sizeof(buf), "%s/%s", path, name);
	    if (compileLSL(game, buf, FALSE))
		PD("Against all odds, the compile of %s worked!  lol", buf);
	    else
		PE("The compile of %s failed!", buf);
	}
    }
}

int
main(int argc, char **argv)
{
    /* put here any init specific to this app like parsing args etc. */
    gameGlobals game;

    if (!ecore_evas_init())
	return EXIT_FAILURE;

    if (!edje_init())
    {
	ecore_evas_shutdown();
	return EXIT_FAILURE;
    }

    memset(&game, 0, sizeof(gameGlobals));
    // Since we increment at the begining, we need to pre decrement this so it starts at 0.

    loggingStartup(&game);

//    else if ((game.config) && (game.data))
    {
	char buf[PATH_MAX];
	char *group = "main";
	Evas_Object *bub, *sh;
	Ecore_Animator *ani;
	unsigned int i;

	/* this will give you a window with an Evas canvas under the first engine available */
	game.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
	if (!game.ee)
	{
	    PEm("You got to have at least one evas engine built and linked up to ecore-evas for this example to run properly.");
	    edje_shutdown();
	    ecore_evas_shutdown();
	    return -1;
	}
	game.canvas = ecore_evas_get(game.ee);
	ecore_evas_title_set(game.ee, "LuaSL test harness");
	ecore_evas_show(game.ee);

	game.bg = evas_object_rectangle_add(game.canvas);
	evas_object_color_set(game.bg, 255, 255, 255, 255); /* white bg */
	evas_object_move(game.bg, 0, 0); /* at canvas' origin */
	evas_object_size_hint_weight_set(game.bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(game.bg, WIDTH, HEIGHT); /* covers full canvas */
	evas_object_show(game.bg);
	ecore_evas_object_associate(game.ee, game.bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);
	evas_object_focus_set(game.bg, EINA_TRUE);

	game.edje = edje_object_add(game.canvas);
	snprintf(buf, sizeof(buf), "%s/%s.edj", PACKAGE_DATA_DIR, "LuaSL");
	if (!edje_object_file_set(game.edje, buf, group))
	{
	    int err = edje_object_load_error_get(game.edje);
	    const char *errmsg = edje_load_error_str(err);
	    PEm("Could not load '%s' from %s: %s\n", group, buf, errmsg);

	    evas_object_del(game.edje);
	    ecore_evas_free(game.ee);
	    edje_shutdown();
	    ecore_evas_shutdown();
	    return -2;
	}
	evas_object_move(game.edje, 0, 0);
	evas_object_resize(game.edje, WIDTH, HEIGHT);
	evas_object_show(game.edje);

	snprintf(buf, sizeof(buf), "%s/images/bubble_sh.png", PACKAGE_DATA_DIR);
	for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
	{
	    sh = evas_object_image_filled_add(game.canvas);
	    evas_object_image_file_set(sh, buf, NULL);
	    evas_object_resize(sh, 64, 64);
	    evas_object_show(sh);
	    evas_object_data_set(game.bg, names[(i * 2) + 1], sh);
	}

	snprintf(buf, sizeof(buf), "%s/images/bubble.png", PACKAGE_DATA_DIR);
	for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
	{
	    bub = evas_object_image_filled_add(game.canvas);
	    evas_object_image_file_set(bub, buf, NULL);
	    evas_object_resize(bub, 64, 64);
	    evas_object_show(bub);
	    evas_object_data_set(game.bg, names[(i * 2)], bub);
	}
	ani = ecore_animator_add(anim, &game);
	evas_object_data_set(game.bg, "animator", ani);

	// Setup our callbacks.
	ecore_evas_callback_delete_request_set(game.ee, _on_delete);
	edje_object_signal_callback_add(game.edje, "*", "game_*", _edje_signal_cb, &game);

	// Do the compiles.
	compilerSetup(&game);
	snprintf(buf, sizeof(buf), "%s/Test sim/objects", PACKAGE_DATA_DIR);
	eina_file_dir_list(buf, EINA_TRUE, dirList_cb, &game);

//	ecore_main_loop_begin();

	ecore_animator_del(ani);
	ecore_evas_free(game.ee);
	edje_shutdown();
	ecore_evas_shutdown();
    }

    return 0;
}

