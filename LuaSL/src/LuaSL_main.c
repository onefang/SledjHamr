#include "LuaSL.h"

#define LUA_TEST	0

static int scriptCount;

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

static void dirList_cb(const char *name, const char *path, void *data)
{
    gameGlobals *game = data;
    char buf[PATH_MAX];
    char *ext = rindex(name, '.');

    if (ext)
    {
    if ((!LUASL_DEBUG) || (0 == scriptCount))
	if (0 == strcmp(ext, ".lsl"))
	{
	    scriptCount++;
	    snprintf(buf, sizeof(buf), "%s/%s", path, name);
	    if (compileLSL(game, buf, FALSE))
		PD("Against all odds, the compile of %s worked!  lol", buf);
	    else
		PE("The compile of %s failed!", buf);
	}
    }
}

#if LUA_TEST
static void dirListLua_cb(const char *name, const char *path, void *data)
{
    char buf[PATH_MAX];
    char *ext = rindex(name, '.');

    if (ext)
    {
	if (0 == strcmp(ext, ".lua"))
	{
	    scriptCount++;
	    snprintf(buf, sizeof(buf), "luac %s/%s 2>/dev/null", path, name);
	    system(buf);
	}
    }
}

#ifdef _WIN32
# define FMT_SIZE_T "%Iu"
#else
# define FMT_SIZE_T "%zu"
#endif

#define MAX_LUA_MEM (4 * (1024 * 1024))

#define _edje_lua2_error(L, err_code) _edje_lua2_error_full(__FILE__, __FUNCTION__, __LINE__, L, err_code)

/*
typedef struct _Edje_Lua_Alloc       Edje_Lua_Alloc;

struct _Edje_Lua_Alloc
{
   size_t max, cur;
};

static void *
_elua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    Edje_Lua_Alloc *ela = ud;
    void *ptr2 = NULL;

    if (ela)
    {
	ela->cur += nsize - osize;
	if (ela->cur > ela->max)
	{
	    printf("Lua memory limit of " FMT_SIZE_T " bytes reached (" FMT_SIZE_T  " allocated)", ela->max, ela->cur);
	}
	else if (nsize == 0)
	{
		free(ptr);
	}
	else
	{
	    ptr2 = realloc(ptr, nsize);
	    if (NULL == ptr2)
		printf("Lua cannot re-allocate " FMT_SIZE_T " bytes", nsize);
	}
    }
    else
	printf("Lua cannoct allocate memory, no Edje_Lua_Alloc");

    return ptr2;
}

static int panics = 0;
static int
_elua_custom_panic(lua_State *L)                   // Stack usage [-0, +0, m]
{
   // If we somehow manage to have multiple panics, it's likely due to being out
   // of memory in the following lua_tostring() call.
   panics++;
   if (panics)
     {
        printf("Lua PANICS!!!!!");
     }
   else
     {
        printf("Lua PANIC!!!!!: %s", lua_tostring(L, -1));  // Stack usage [-0, +0, m]
     }
   // The docs say that this will cause an exit(EXIT_FAILURE) if we return,
   // and that we we should long jump some where to avoid that.  This is only
   // called for things not called from a protected environment.  We always
   // use pcalls though, except for the library load calls.  If we can't load
   // the standard libraries, then perhaps a crash is the right thing.
   return 0;
}

static void
_edje_lua2_error_full(const char *file, const char *fnc, int line,
                      lua_State *L, int err_code)            // Stack usage [-0, +0, m]
{
   const char *err_type;

   switch (err_code)
     {
     case LUA_ERRRUN:
        err_type = "runtime";
        break;
     case LUA_ERRSYNTAX:
        err_type = "syntax";
        break;
     case LUA_ERRMEM:
        err_type = "memory allocation";
        break;
     case LUA_ERRERR:
        err_type = "error handler";
        break;
     default:
        err_type = "unknown";
        break;
     }
   printf("Lua %s error: %s\n", err_type, lua_tostring(L, -1));  // Stack usage [-0, +0, m]
}

static int errFunc(lua_State *L)
{
    int i = 0;
    lua_Debug ar;

    while (lua_getstack(L, i++, &ar))
    {
	if (lua_getinfo(L, "nSlu", &ar))
	{
	    if (NULL == ar.name)
		ar.name = "DUNNO";
	    printf("Lua error in the %s %s %s @ line %d in %s\n%s!", ar.what, ar.namewhat, ar.name, ar.currentline, ar.short_src, ar.source);
	}
    }
    return 0;
}
*/

static void runnerSetup(gameGlobals *game)
{
    luaprocInit();

    if ( sched_create_worker( ) != LUAPROC_SCHED_OK )
	PE("Error creating luaproc worker thread.");
    if ( sched_create_worker( ) != LUAPROC_SCHED_OK )
	PE("Error creating luaproc worker thread.");
    if ( sched_create_worker( ) != LUAPROC_SCHED_OK )
	PE("Error creating luaproc worker thread.");
    if ( sched_create_worker( ) != LUAPROC_SCHED_OK )
	PE("Error creating luaproc worker thread.");
}

static void runnerTearDown(gameGlobals *game)
{
// TODO - this is what hangs the system.
    sched_join_workerthreads();
}

static void runLuaFile(gameGlobals *game, const char *filename)
{
    newProc(filename, TRUE);

// TODO, should set up our panic and errfunc as below.  Plus the other TODO stuff.

/*
// TODO - hack up LuaJIT so that we can limit memory per state.
//    lua_setallocf(L, _elua_alloc, &ela);  // LuaJIT uses a heavily hacked up dlmalloc.  Seems that standard realloc is not so thread safe?
    lua_atpanic(L, _elua_custom_panic);
// TODO - Sandbox out what this opens.  See lib_init.c  from LuaJIT.
// Just noticed this in the LuaJIT docs - "To change or extend the list of standard libraries to load, copy src/lib_init.c to your project and modify it accordingly. Make sure the jit library is loaded or the JIT compiler will not be activated."
    luaL_openlibs(L);

    lua_pushcfunction(L, errFunc);
    ...
    if ((err = lua_pcall(L, 0, 0, -2)))
	_edje_lua2_error(L, err);
*/
}

#endif

int
main(int argc, char **argv)
{
    /* put here any init specific to this app like parsing args etc. */
    gameGlobals game;
    char *programName = argv[0];
    boolean badArgs = FALSE;

    if (!ecore_evas_init())
	return EXIT_FAILURE;

    if (!edje_init())
    {
	ecore_evas_shutdown();
	return EXIT_FAILURE;
    }

    memset(&game, 0, sizeof(gameGlobals));

    loggingStartup(&game);

    // get the arguments passed in
    while (--argc > 0 && *++argv != '\0')
    {
	if (*argv[0] == '-')
	{
	    // point to the characters after the '-' sign
	    char *s = argv[0] + 1;

	    switch (*s)
	    {
		case 'u':
		{
		    game.ui = TRUE;
		    break;
		}
		default:
		    badArgs = TRUE;
	    }
	}
	else
	    badArgs = TRUE;
    }


    if (badArgs)
    {
	// display the program usage to the user as they have it wrong 
	printf("Usage: %s [-u]\n", programName);
	printf("   -u: Show the test UI.\n");
    }
    else
//    else if ((game.config) && (game.data))
    {
	unsigned int i;
	Evas_Object *bub, *sh;
	Ecore_Animator *ani;
	char *group = "main";
	char buf[PATH_MAX];
	struct timeval lastTime2;
	struct timeval thisTime2;
	float diff;
#if LUA_TEST
	unsigned int lslCount;
	float diff0;
#endif

	if (game.ui)
	{
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
	}

	// Do the compiles.
	scriptCount = 0;
	gettimeofday(&lastTime2, 0);
	compilerSetup(&game);
#if LUA_TEST
	runnerSetup(&game);
#endif
	snprintf(buf, sizeof(buf), "%s/Test sim/objects", PACKAGE_DATA_DIR);
	eina_file_dir_list(buf, EINA_TRUE, dirList_cb, &game);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("Compiling %d LSL scripts took %f seconds, that's %f scripts per second.\n", scriptCount, diff, scriptCount / diff);

#if LUA_TEST
	lslCount = scriptCount;
	diff0 = diff;
	scriptCount = 0;
	gettimeofday(&lastTime2, 0);
	snprintf(buf, sizeof(buf), "%s/testLua", PACKAGE_DATA_DIR);
	eina_file_dir_list(buf, EINA_TRUE, dirListLua_cb, &game);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("Compiling %d Lua scripts took %f seconds, that's %f scripts per second.\n\n", scriptCount, diff, scriptCount / diff);

	printf("Combined estimate of compiling speed is %f scripts per second.\n", 1 / ((diff0 / lslCount) + (diff / scriptCount)));

	gettimeofday(&lastTime2, 0);
	snprintf(buf, sizeof(buf), "lua luaprocTest0.lua");
	system(buf);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("%s TOOK %f seconds......................................................................................................\n", buf, diff);

	gettimeofday(&lastTime2, 0);
	snprintf(buf, sizeof(buf), "%s/testLua/luaprocTest0_C.lua", PACKAGE_DATA_DIR);
	runLuaFile(&game, buf);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("Running that last one locally TOOK %f seconds.\n",diff);

	gettimeofday(&lastTime2, 0);
	snprintf(buf, sizeof(buf), "lua luaprocTest1.lua");
	system(buf);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("%s TOOK %f seconds.\n", buf, diff);

	gettimeofday(&lastTime2, 0);
	snprintf(buf, sizeof(buf), "../../libraries/luajit-2.0/src/luajit luaprocTest1.lua");
	system(buf);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("%s TOOK %f seconds.\n", buf, diff);

	gettimeofday(&lastTime2, 0);
	snprintf(buf, sizeof(buf), "lua luaprocTest2.lua");
	system(buf);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("%s TOOK %f seconds.\n", buf, diff);

	gettimeofday(&lastTime2, 0);
	snprintf(buf, sizeof(buf), "../../libraries/luajit-2.0/src/luajit luaprocTest2.lua");
	system(buf);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("%s TOOK %f seconds.\n", buf, diff);

	gettimeofday(&lastTime2, 0);
	snprintf(buf, sizeof(buf), "%s/testLua/luaprocTest2_C.lua", PACKAGE_DATA_DIR);
	runLuaFile(&game, buf);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("Running that last one locally TOOK %f seconds.\n",diff);
#endif

	if (game.ui)
	{
	    ecore_main_loop_begin();
	    ecore_animator_del(ani);
	    ecore_evas_free(game.ee);
	}

#if LUA_TEST
	runnerTearDown(&game);
	diff = timeDiff(&thisTime2, &lastTime2);
	printf("Running that last one locally TOOK %f seconds.\n",diff);
#endif
	edje_shutdown();
	ecore_evas_shutdown();
    }

    return 0;
}

