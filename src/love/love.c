/* love world server - The central server that manages the virtual world.

Dedicated to my girl Boots, coz she means the world to me.

*/

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>

#include <unistd.h>

#include "LumbrJack.h"
#include "Runnr.h"


#define WIDTH  (512)
#define HEIGHT (384)


#define TABLE_WIDTH	7
#define TABLE_HEIGHT	42


typedef struct _gameGlobals
{
    Ecore_Evas		*ee;		// Our window.
    Evas		*canvas;	// The canvas for drawing directly onto.
    Evas_Object		*bg;		// Our background edje, also the game specific stuff.
    Evas_Object		*edje;		// The edje of the background.
    Ecore_Con_Server	*server;
    Eina_Hash		*scripts;
    const char		*address;
    int			port;
    boolean		ui;		// Wether we actually start up the UI.
} gameGlobals;

typedef struct _script
{
    char		SID[PATH_MAX];
    char		fileName[PATH_MAX];
    struct timeval	startTime;
    float		compileTime;
    int			bugs, warnings;
    boolean		running;
} script;


int logDom;	// Our logging domain.
static Eina_Strbuf *clientStream;
static int scriptCount = 0;
static int compiledCount = 0;
static float compileTime = 0.0;
static struct timeval startTime;
static int timedEvent = 0;
static char *ownerKey = "12345678-1234-4321-abcd-0123456789ab";
static char *ownerName = "onefang rejected";

static const char *names[] =
{
     "bub1", "sh1",
     "bub2", "sh2",
     "bub3", "sh3",
};


static float timeDiff(struct timeval *now, struct timeval *then)
{
    if (0 == gettimeofday(now, 0))
    {
	struct timeval thisTime = { 0, 0 };
	double  result = 0.0;

	thisTime.tv_sec = now->tv_sec;
	thisTime.tv_usec = now->tv_usec;
	if (thisTime.tv_usec < then->tv_usec)
	{
	    thisTime.tv_sec--;
	    thisTime.tv_usec += 1000000;
	}
	thisTime.tv_usec -= then->tv_usec;
	thisTime.tv_sec -= then->tv_sec;
	result = ((double) thisTime.tv_usec) / ((double) 1000000.0);
	result += thisTime.tv_sec;
	return result;
    }
    else
	return 0.0;
}


static void
_edje_signal_cb(void *data, Evas_Object *obj, const char  *emission, const char  *source)
{
//    gameGlobals *ourGlobals = data;
}

static
Eina_Bool anim(void *data)
{
    gameGlobals *ourGlobals = data;
    Evas_Object *bub, *sh;
    Evas_Coord x, y, w, h, vw, vh;
    double t, xx, yy, zz, r, fac;
    double lx, ly;
    unsigned int i;

    evas_output_viewport_get(ourGlobals->canvas, 0, 0, &vw, &vh);
    r = 48;
    t = ecore_loop_time_get();
    fac = 2.0 / (double)((sizeof(names) / sizeof(char *) / 2));
    evas_pointer_canvas_xy_get(ourGlobals->canvas, &x, &y);
    lx = x;
    ly = y;

    for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
    {
	bub = evas_object_data_get(ourGlobals->bg, names[i * 2]);
	sh = evas_object_data_get(ourGlobals->bg, names[(i * 2) + 1]);
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
_on_delete(Ecore_Evas *ee)
{
    ecore_main_loop_quit();
}

static void dirList_compile(const char *name, const char *path, void *data)
{
    gameGlobals *ourGlobals = data;

    char *ext = rindex(name, '.');

    if (ext)
    {
	if (0 == strcmp(ext, ".lsl"))
	{
	    script *me = calloc(1, sizeof(script));

	    scriptCount++;
	    gettimeofday(&me->startTime, NULL);
	    snprintf(me->SID, sizeof(me->SID), "%08lx-%04lx-%04lx-%04lx-%012lx", random(), random() % 0xFFFF, random() % 0xFFFF, random() % 0xFFFF, random());
	    snprintf(me->fileName, sizeof(me->fileName), "%s/%s", path, name);
	    eina_hash_add(ourGlobals->scripts, me->SID, me);
	    sendForth(ourGlobals->server, me->SID, "compile(%s)", me->fileName);
	}
    }
}

static Eina_Bool _timer_cb(void *data)
{
    gameGlobals *ourGlobals = data;
    Eina_Iterator *scripts;
    script *me;
    boolean exit = FALSE;

    scripts = eina_hash_iterator_data_new(ourGlobals->scripts);
    while(eina_iterator_next(scripts, (void **) &me))
    {
	switch (timedEvent)
	{
	    case 5 :
	    {
		sendForth(ourGlobals->server, me->SID, "events.detectedKeys({\"%s\"})", ownerKey);
		sendForth(ourGlobals->server, me->SID, "events.detectedNames({\"%s\"})", ownerName);
		sendForth(ourGlobals->server, me->SID, "events.touch_start(1)");
		break;
	    }
	    case 9 :
	    {
		sendForth(ourGlobals->server, me->SID, "quit()");
		break;
	    }
	    case 11 :
	    {
		exit = TRUE;
		break;
	    }
	}
    }
    timedEvent++;

    if (exit)
    {
	sendForth(ourGlobals->server, ownerKey, "exit()");
	ecore_main_loop_quit();
	return ECORE_CALLBACK_CANCEL;
    }
    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _add(void *data, int type, Ecore_Con_Event_Server_Add *ev)
{
    gameGlobals *ourGlobals = data;
    char buf[PATH_MAX];

    ourGlobals->server = ev->server;
    gettimeofday(&startTime, NULL);
    snprintf(buf, sizeof(buf), "%s/Test sim/objects", PACKAGE_DATA_DIR);
    eina_file_dir_list(buf, EINA_TRUE, dirList_compile, ourGlobals);
    // Wait awhile, then start sending events for testing.
    ecore_timer_add(0.5, _timer_cb, ourGlobals);
    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _data(void *data, int type, Ecore_Con_Event_Server_Data *ev)
{
    gameGlobals *ourGlobals = data;

    char buf[PATH_MAX];
    char SID[PATH_MAX];
    const char *command;
    char *ext;

    eina_strbuf_append_length(clientStream, ev->data, ev->size);
    command = eina_strbuf_string_get(clientStream);
    while ((ext = index(command, '\n')))
    {
	int length = ext - command;

	strncpy(SID, command, length + 1);
	SID[length] = '\0';
	eina_strbuf_remove(clientStream, 0, length + 1);
	ext = index(SID, '.');
	if (ext)
	{
	    script *me;

	    ext[0] = '\0';
	    command = ext + 1;
	    me = eina_hash_find(ourGlobals->scripts, SID);
	    if (0 == strncmp(command, "compilerWarning(", 16))
	    {
		char *temp;
		char *line;
		char *column;
		char *text;

		strcpy(buf, &command[16]);
		temp = buf;
		line = temp;
		while (',' != temp[0])
		    temp++;
		temp[0] = '\0';
		column = ++temp;
		while (',' != temp[0])
		    temp++;
		temp[0] = '\0';
		text = ++temp;
		while (')' != temp[0])
		    temp++;
		temp[0] = '\0';
		PW("%s @ line %s, column %s.", text, line, column);
		if (me)
		    me->warnings++;
	    }
	    else if (0 == strncmp(command, "compilerError(", 14))
	    {
		char *temp;
		char *line;
		char *column;
		char *text;

		strcpy(buf, &command[14]);
		temp = buf;
		line = temp;
		while (',' != temp[0])
		    temp++;
		temp[0] = '\0';
		column = ++temp;
		while (',' != temp[0])
		    temp++;
		temp[0] = '\0';
		text = ++temp;
		while (')' != temp[0])
		    temp++;
		temp[0] = '\0';
		PE("%s @ line %s, column %s.", text, line, column);
		if (me)
		    me->bugs++;
	    }
	    else if (0 == strcmp(command, "compiled(false)"))
		PE("The compile of %s failed!", SID);
	    else if (0 == strcmp(command, "compiled(true)"))
	    {
		if (me)
		{
		    struct timeval now;

		    me->compileTime = timeDiff(&now, &me->startTime);
		    me->running = TRUE;
		    compiledCount++;
		    compileTime += me->compileTime;
//		    PD("Average compile speed is %f scripts per second", compiledCount / compileTime);
		    if (compiledCount == scriptCount)
			PD("TOTAL compile speed is %f scripts per second", compiledCount / timeDiff(&now, &startTime));
		}
//		PD("The compile of %s worked, running it now.", SID);
		sendForth(ourGlobals->server, SID, "run()");
	    }
	    else
	    {
		// Send back some random or fixed values for testing.
		if (0 == strcmp(command, "llGetKey()"))
		    sendForth(ourGlobals->server, SID, "return \"%08lx-%04lx-%04lx-%04lx-%012lx\"", random(), random() % 0xFFFF, random() % 0xFFFF, random() % 0xFFFF, random());
		else if (0 == strcmp(command, "llGetOwner()"))
		    sendForth(ourGlobals->server, SID, "return \"%s\"", ownerKey);
		else if (0 == strcmp(command, "llGetPos()"))
		    sendForth(ourGlobals->server, SID, "return {x=128.0, y=128.0, z=128.0}");
		else if (0 == strcmp(command, "llGetRot()"))
		    sendForth(ourGlobals->server, SID, "return {x=0.0, y=0.0, z=0.0, s=1.0}");
		else if (0 == strcmp(command, "llGetObjectDesc()"))
		    sendForth(ourGlobals->server, SID, "return \"\"");
		else if (0 == strncmp(command, "llGetAlpha(", 11))
		    sendForth(ourGlobals->server, SID, "return 1.0");
		else if (0 == strcmp(command, "llGetInventoryNumber(7)"))
		    sendForth(ourGlobals->server, SID, "return 3");
		else if (0 == strcmp(command, "llGetInventoryName(7, 2)"))
		    sendForth(ourGlobals->server, SID, "return \".readme\"");
		else if (0 == strcmp(command, "llGetInventoryName(7, 1)"))
		    sendForth(ourGlobals->server, SID, "return \".POSITIONS\"");
		else if (0 == strcmp(command, "llGetInventoryName(7, 0)"))
		    sendForth(ourGlobals->server, SID, "return \".MENUITEMS\"");
		else
		    PI("Script %s sent command %s", SID, command);
	    }
	}

	// Get the next blob to check it.
	command = eina_strbuf_string_get(clientStream);
    }

    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _del(void *data, int type, Ecore_Con_Event_Server_Del *ev)
{
    gameGlobals *ourGlobals = data;

    if (ev->server)
    {
	ourGlobals->server = NULL;
	ecore_con_server_del(ev->server);
	if (!ourGlobals->ui)
	    ecore_main_loop_quit();
    }

    return ECORE_CALLBACK_RENEW;
}

int main(int argc, char **argv)
{
    /* put here any init specific to this app like parsing args etc. */
    gameGlobals ourGlobals;
    char *programName = argv[0];
    boolean badArgs = FALSE;
    int result = EXIT_FAILURE;
    char buf[PATH_MAX];

    sprintf(buf, "%s/LuaSL &", PACKAGE_BIN_DIR);
    system(buf);
    sleep(1);

    memset(&ourGlobals, 0, sizeof(gameGlobals));
    ourGlobals.address = "127.0.0.1";
    ourGlobals.port = 8211;

    if (eina_init())
    {
	logDom = loggingStartup("love", logDom);
	ourGlobals.scripts = eina_hash_string_superfast_new(NULL);

	if (ecore_con_init())
	{
	    if ((ourGlobals.server = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, ourGlobals.address, ourGlobals.port, &ourGlobals)))
	    {
		ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,  (Ecore_Event_Handler_Cb) _add,  &ourGlobals);
		ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, (Ecore_Event_Handler_Cb) _data, &ourGlobals);
		ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,  (Ecore_Event_Handler_Cb) _del,  &ourGlobals);
		clientStream = eina_strbuf_new();

		if (ecore_evas_init())
                {
		    if (edje_init())
		    {
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
					ourGlobals.ui = TRUE;
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
		    //    else if ((ourGlobals.config) && (ourGlobals.data))
			{
			    unsigned int i;
			    Evas_Object *bub, *sh;
			    Ecore_Animator *ani;
			    char *group = "main";
			    char buf[PATH_MAX];

			    if (ourGlobals.ui)
			    {
				/* this will give you a window with an Evas canvas under the first engine available */
				ourGlobals.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
				if (!ourGlobals.ee)
				{
				    PE("You got to have at least one evas engine built and linked up to ecore-evas for this example to run properly.");
				    edje_shutdown();
				    ecore_evas_shutdown();
				    return -1;
				}
				ourGlobals.canvas = ecore_evas_get(ourGlobals.ee);
				ecore_evas_title_set(ourGlobals.ee, "love test harness (snickers)");
				ecore_evas_show(ourGlobals.ee);

				ourGlobals.bg = evas_object_rectangle_add(ourGlobals.canvas);
				evas_object_color_set(ourGlobals.bg, 255, 255, 255, 255); /* white bg */
				evas_object_move(ourGlobals.bg, 0, 0); /* at canvas' origin */
				evas_object_size_hint_weight_set(ourGlobals.bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				evas_object_resize(ourGlobals.bg, WIDTH, HEIGHT); /* covers full canvas */
				evas_object_show(ourGlobals.bg);
				ecore_evas_object_associate(ourGlobals.ee, ourGlobals.bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);
				evas_object_focus_set(ourGlobals.bg, EINA_TRUE);

				ourGlobals.edje = edje_object_add(ourGlobals.canvas);
				snprintf(buf, sizeof(buf), "%s/%s.edj", PACKAGE_DATA_DIR, "love");
				if (!edje_object_file_set(ourGlobals.edje, buf, group))
				{
				    int err = edje_object_load_error_get(ourGlobals.edje);
				    const char *errmsg = edje_load_error_str(err);
				    PE("Could not load '%s' from %s: %s\n", group, buf, errmsg);

				    evas_object_del(ourGlobals.edje);
				    ecore_evas_free(ourGlobals.ee);
				    edje_shutdown();
				    ecore_evas_shutdown();
				    return -2;
				}
				evas_object_move(ourGlobals.edje, 0, 0);
				evas_object_resize(ourGlobals.edje, WIDTH, HEIGHT);
				evas_object_show(ourGlobals.edje);

				snprintf(buf, sizeof(buf), "%s/bubble_sh.png", PACKAGE_DATA_DIR);
				for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
				{
				    sh = evas_object_image_filled_add(ourGlobals.canvas);
				    evas_object_image_file_set(sh, buf, NULL);
				    evas_object_resize(sh, 64, 64);
				    evas_object_show(sh);
				    evas_object_data_set(ourGlobals.bg, names[(i * 2) + 1], sh);
				}

				snprintf(buf, sizeof(buf), "%s/bubble.png", PACKAGE_DATA_DIR);
				for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
				{
				    bub = evas_object_image_filled_add(ourGlobals.canvas);
				    evas_object_image_file_set(bub, buf, NULL);
				    evas_object_resize(bub, 64, 64);
				    evas_object_show(bub);
				    evas_object_data_set(ourGlobals.bg, names[(i * 2)], bub);
				}
				ani = ecore_animator_add(anim, &ourGlobals);
				evas_object_data_set(ourGlobals.bg, "animator", ani);

				// Setup our callbacks.
				ecore_evas_callback_delete_request_set(ourGlobals.ee, _on_delete);
				edje_object_signal_callback_add(ourGlobals.edje, "*", "game_*", _edje_signal_cb, &ourGlobals);
			    }

			    ecore_main_loop_begin();
			    if (ourGlobals.ui)
			    {
				ecore_animator_del(ani);
				ecore_evas_free(ourGlobals.ee);
			    }
			}

			edje_shutdown();
		    }
		    else
			PC("Failed to init edje!");
		    ecore_evas_shutdown();
		}
		else
		    PC("Failed to init ecore_evas!");
	    }
	    else
		PC("Failed to connect to server!");
	    ecore_con_shutdown();
	}
	else
	    PC("Failed to init ecore_con!");
    }
    else
	fprintf(stderr, "Failed to init eina!");

    return result;
}
