/* love world server - The central server that manages the virtual world.

Dedicated to my girl Boots, coz she means the world to me.

*/

#include "LumbrJack.h"		// Have to include this first, to turn on the Eo and beta API stuff.

#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "SledjHamr.h"
#include "love.h"


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
    Ecore_Con_Server	*serverLuaSL;
    Ecore_Con_Server	*server;
    Ecore_Con_Client	*client;	// TODO - Really should be a bunch of these.
    Eina_Hash		*scripts;
    const char		*address;
    int			port;
    boolean		ui;		// Wether we actually start up the UI.
} gameGlobals;

typedef struct _Lscript
{
    char		SID[PATH_MAX];
    char		fileName[PATH_MAX];
    struct timeval	startTime;
    float		compileTime;
    int			bugs, warnings;
    boolean		running;
} LoveScript;


int logDom = -1;	// Our logging domain.
static Eina_Strbuf *LuaSLStream;
static Eina_Strbuf *clientStream;
static int scriptCount = 0;
static int compiledCount = 0;
static float compileTime = 0.0;
static struct timeval startTime;
//static int timedEvent = 0;
static char *ownerKey = "12345678-1234-4321-abcd-0123456789ab";
static char *ownerName = "onefang rejected";

static const char *names[] =
{
     "bub1", "sh1",
     "bub2", "sh2",
     "bub3", "sh3",
};


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
	    // TODO - We are leaking these, coz we don't know when scripts get deleted in the script server.
	    //        On the other hand, the main use for this is a temporary hack that sends events to all scripts.
	    //        So that part will get a rewrite when we make it real later anyway.
	    LoveScript *me = calloc(1, sizeof(LoveScript));

	    scriptCount++;
	    gettimeofday(&me->startTime, NULL);
	    snprintf(me->SID, sizeof(me->SID), FAKE_UUID);
	    snprintf(me->fileName, sizeof(me->fileName), "%s/%s", path, name);
	    eina_hash_add(ourGlobals->scripts, me->SID, me);
	    sendForth(ourGlobals->serverLuaSL, me->SID, "compile(%s)", me->fileName);
	}
    }
}

static Eina_Bool _addLuaSL(void *data, int type, Ecore_Con_Event_Server_Add *ev)
{
  gameGlobals *ourGlobals = data;
  char buf[PATH_MAX];

  ourGlobals->serverLuaSL = ev->server;

  // Compile and run scripts.
  gettimeofday(&startTime, NULL);
  snprintf(buf, sizeof(buf), "%s/Test%%20sim", prefix_data_get());
  eina_file_dir_list(buf, EINA_TRUE, dirList_compile, data);

  return ECORE_CALLBACK_RENEW;
}


// Borrowed from toybox, though it's real slow.
// TODO - Replace this quick and dirty llGetNotecardLine() with something using mmap and cached files that assumes more lines will be read soon.
char *get_rawline(int fd, long *plen, char end)
{
  char c, *buf = NULL;
  long len = 0;

  for (;;) {
    if (1>read(fd, &c, 1)) break;
    if (!(len & 63)) buf=realloc(buf, len+65);
    if ((buf[len++]=c) == end) break;
  }
  if (buf) buf[len]=0;
  if (plen) *plen = len;

  return buf;
}


static Eina_Bool _dataLuaSL(void *data, int type, Ecore_Con_Event_Server_Data *ev)
{
    gameGlobals *ourGlobals = data;

    char buf[PATH_MAX];
    char SID[PATH_MAX];
    const char *command;
    char *ext;

    eina_strbuf_append_length(LuaSLStream, ev->data, ev->size);
    command = eina_strbuf_string_get(LuaSLStream);
    while ((ext = index(command, '\n')))
    {
	int length = ext - command;

	strncpy(SID, command, length + 1);
	SID[length] = '\0';
	eina_strbuf_remove(LuaSLStream, 0, length + 1);
	ext = index(SID, '.');
	if (ext)
	{
	    LoveScript *me;

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
		sendForth(ourGlobals->serverLuaSL, SID, "run()");
	    }
	    else
	    {
		// Send back some random or fixed values for testing.
		if (0 == strcmp(command, "llGetKey()"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return \"%08lx-%04lx-%04lx-%04lx-%012lx\"", random(), random() % 0xFFFF, random() % 0xFFFF, random() % 0xFFFF, random());
		else if (0 == strcmp(command, "llGetOwner()"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return \"%s\"", ownerKey);
		else if (0 == strcmp(command, "llGetPermissionsKey()"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return \"%s\"", ownerKey);
		else if (0 == strncmp(command, "llRequestPermissions(", 21))
		    PI("Faked %s", command);
		else if (0 == strcmp(command, "llGetPos()"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return {x=128.0, y=128.0, z=128.0}");
		else if (0 == strcmp(command, "llGetRot()"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return {x=0.0, y=0.0, z=0.0, s=1.0}");
		else if (0 == strcmp(command, "llGetFreeMemory()"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return 654321");
		else if (0 == strcmp(command, "llGetObjectDesc()"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return \"\"");
		else if (0 == strncmp(command, "llGetAlpha(", 11))
		    sendForth(ourGlobals->serverLuaSL, SID, "return 1.0");
		else if (0 == strcmp(command, "llGetInventoryNumber(7)"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return 3");
		else if (0 == strcmp(command, "llGetLinkNumber()"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return 1");
		else if (0 == strcmp(command, "llGetInventoryName(7, 2)"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return \".readme\"");
		else if (0 == strcmp(command, "llGetInventoryName(7, 1)"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return \".POSITIONS\"");
		else if (0 == strcmp(command, "llGetInventoryName(7, 0)"))
		    sendForth(ourGlobals->serverLuaSL, SID, "return \".MENUITEMS\"");
		else if (0 == strncmp(command, "llListen(", 9))
		{
		    PI("Faked %s", command);
		    sendForth(ourGlobals->serverLuaSL, SID, "return %d", random());
		}
		else if (0 == strncmp(command, "llSameGroup(", 12))
		    sendForth(ourGlobals->serverLuaSL, SID, "return true");
		else if (0 == strncmp(command, "llKey2Name(", 11))
		{
		    char *temp;

		    strcpy(buf, &command[12]);
		    temp = buf;
		    while (')' != temp[0])
			temp++;
		    temp[0] = '\0';
		    if (0 == strcmp(buf, ownerKey))
		      temp = ownerName;
		    else
		      temp = "Unknown User";
		    // TODO - Sanitize the name, no telling what weird shit people put in their names.
		    snprintf(buf, sizeof(buf), "return \"%s\"", temp);
		    sendForth(ourGlobals->serverLuaSL, SID, buf);
		}
		// Send "back" stuff on to the one and only client.
		// TODO - All of these output functions should just use one thing to append stuff to either local or an IM tab.
		//        Love filtering out stuff that should not go there.
		//        Extantz registering any channel it wants to listen to, mostly for client side scripts.
		//        Extantz is then only responsible for the registered channels, it can do what it likes with them.
		//        Dialogs, notifications, and other stuff goes through some other functions.
		else if (0 == strncmp(command, "llOwnerSay(", 11))
		{
		    if (ourGlobals->client)  sendBack(ourGlobals->client, SID, command);
		    else PW("No where to send %s", command);
		}
		else if (0 == strncmp(command, "llWhisper(", 10))
		{
		    if (ourGlobals->client)  sendBack(ourGlobals->client, SID, command);
		    else PW("No where to send %s", command);
		}
		else if (0 == strncmp(command, "llRegionSay(", 12))
		{
		    if (ourGlobals->client)  sendBack(ourGlobals->client, SID, command);
		    else PW("No where to send %s", command);
		}
		else if (0 == strncmp(command, "llSay(", 6))
		{
		    if (ourGlobals->client)  sendBack(ourGlobals->client, SID, command);
		    else PW("No where to send %s", command);
		}
		else if (0 == strncmp(command, "llShout(", 8))
		{
		    if (ourGlobals->client)  sendBack(ourGlobals->client, SID, command);
		    else PW("No where to send %s", command);
		    // TODO - Temporary so we have a place to log stuff from LSL.
		    PD("SHOUTING %s", command);
		}
		else if (0 == strncmp(command, "llDialog(", 9))
		{
		    if (ourGlobals->client)  sendBack(ourGlobals->client, SID, command);
		    else PW("No where to send %s", command);
		}
		else if (0 == strncmp(command, "llMessageLinked(", 16))
		{
		    Eina_Iterator *scripts;
		    LoveScript *me;

		    // TODO - For now, just send it to everyone.
		    scripts = eina_hash_iterator_data_new(ourGlobals->scripts);
		    while(eina_iterator_next(scripts, (void **) &me))
		    {
			sendForth(ourGlobals->serverLuaSL, me->SID, "events.link_message%s", &command[15]);
		    }
		    eina_iterator_free(scripts);
		}
		else if (0 == strncmp(command, "llGetNotecardLine(", 18))
		{
		    char *notecard, *temp, *line, key[PATH_MAX];
		    int  lineNo, fd;

		    strcpy(buf, &command[19]);
		    notecard = buf;
		    temp = notecard;
		    while ('"' != temp[0])
			temp++;
		    temp[0] = '\0';
		    while (',' != temp[0])
			temp++;
		    while (' ' != temp[0])
			temp++;
		    line = temp;
		    while (')' != temp[0])
			temp++;
		    temp[0] = '\0';
		    lineNo = atoi(line);
		    snprintf(key, sizeof(key), "%s/Test%%20sim/onefang%%27s%%20test%%20bed/%s", prefix_data_get(), notecard);

		    fd = open(key, O_RDONLY);
		    if (-1 != fd)
		    {
			Eina_Iterator *scripts;
			LoveScript *me;
			long len;

			temp = NULL;
			do
			{
			    free(temp);
			    temp = get_rawline(fd, &len, '\n');
			    if (temp)
			    {
				if (temp[len - 1] == '\n')
				    temp[--len] = '\0';
			    }
			} while (temp && (0 < lineNo--));

			sprintf(key, FAKE_UUID);
			sendForth(ourGlobals->serverLuaSL, SID, "return \"%s\"", key);

			// TODO - For now, just send it to everyone.
			scripts = eina_hash_iterator_data_new(ourGlobals->scripts);
			while(eina_iterator_next(scripts, (void **) &me))
			{
			    if (temp)
			    {
			        char buf2[PATH_MAX];
				int i, j, len = strlen(temp);

				// Escape ' and \ characters.
				for (i = 0, j = 0; i <= len; i++)
				{
				    if ('\'' == temp[i])
					buf2[j++] = '\\';
				    if ('\\' == temp[i])
					buf2[j++] = '\\';
				    buf2[j++] = temp[i];
				}
				sendForth(ourGlobals->serverLuaSL, me->SID, "events.dataserver(\"%s\", '%s')", key, buf2);
			    }
			    else
				sendForth(ourGlobals->serverLuaSL, me->SID, "events.dataserver(\"%s\", \"EndOfFuckingAround\")", key);
			}
			eina_iterator_free(scripts);
			free(temp);

			close(fd);
		    }

		}
		else
		    PI("Script %s sent command %s", SID, command);
	    }
	}

	// Get the next blob to check it.
	command = eina_strbuf_string_get(LuaSLStream);
    }

    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _delLuaSL(void *data, int type, Ecore_Con_Event_Server_Del *ev)
{
  gameGlobals *ourGlobals = data;
  static int count = 0;

  ourGlobals->serverLuaSL = NULL;

  // Let it fail a couple of times during startup, then try to start our own script server.
  count++;
  if (1 < count)
  {
    char buf[PATH_MAX];

    PW("Failed to connect to a script server, starting our own.");
    sprintf(buf, "%s/LuaSL &", prefix_bin_get());
    system(buf);
    count = 0;
  }

  // TODO - May want to renew even if it's not running the GUI, but then we still need some sort of "shut down" signal, which we don't need during testing.
//  if (ourGlobals->ui)
    return ECORE_CALLBACK_RENEW;

//  ecore_main_loop_quit();

//  return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _addClient(void *data, int type, Ecore_Con_Event_Client_Add *ev)
{
  gameGlobals *ourGlobals = data;

  ourGlobals->client = ev->client;
  ecore_con_client_timeout_set(ev->client, 0);

  if (ourGlobals->client)
  {
    // TODO - Sending the currently hard coded ownerKey here, should actually deal with logging in / hypergrid TP style things instead.
    sendBack(ourGlobals->client, ownerKey, "loadSim('file://%s/Test%%20sim')", prefix_data_get());
  }

  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _dataClient(void *data, int type, Ecore_Con_Event_Client_Data *ev)
{
    gameGlobals *ourGlobals = data;
//    char buf[PATH_MAX];
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
	    ext[0] = '\0';
	    command = ext + 1;

	    if (0 == strncmp(command, "events.touch_start(", 19))
	    {
	        Eina_Iterator *scripts;
		LoveScript *me;

		// TODO - For now, just send it to everyone.
		scripts = eina_hash_iterator_data_new(ourGlobals->scripts);
		while(eina_iterator_next(scripts, (void **) &me))
		{
		    sendForth(ourGlobals->serverLuaSL, me->SID, "events.detectedKeys({\"%s\"})", ownerKey);
		    sendForth(ourGlobals->serverLuaSL, me->SID, "events.detectedNames({\"%s\"})", ownerName);
		    sendForth(ourGlobals->serverLuaSL, me->SID, "events.touch_start(1)");
		}
		eina_iterator_free(scripts);
	    }
	    else if (0 == strncmp(command, "events.listen(", 14))
	    {
	        Eina_Iterator *scripts;
		LoveScript *me;

		// TODO - For now, just send it to everyone.
		scripts = eina_hash_iterator_data_new(ourGlobals->scripts);
		while(eina_iterator_next(scripts, (void **) &me))
		{
		    sendForth(ourGlobals->serverLuaSL, me->SID, command);
		}
		eina_iterator_free(scripts);
	    }
	    else
	      PW("Unknown command from client - %s", command);
	}

	// Get the next blob to check it.
	command = eina_strbuf_string_get(clientStream);
    }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _delClient(void *data, int type, Ecore_Con_Event_Client_Del *ev)
{
    gameGlobals *ourGlobals = data;

    if (ev->client)
    {
	Eina_List const *clients;

	// This is only really for testing, normally it just runs 24/7, or until told not to.
	clients = ecore_con_server_clients_get(ourGlobals->server);
        if (0 == eina_list_count(clients))
        {
	    PD("No more clients, exiting.");
	    sendForth(ourGlobals->serverLuaSL, ownerKey, "exit()");
	    ecore_main_loop_quit();
	}
	else
	{
	    PD("Some %d more clients, exiting anyway.", eina_list_count(clients));
	    sendForth(ourGlobals->serverLuaSL, ownerKey, "exit()");
	    ecore_main_loop_quit();
	}
    }
    return ECORE_CALLBACK_RENEW;
}

int main(int argc, char **argv)
{
    gameGlobals ourGlobals;
    char *programName = argv[0];
    boolean badArgs = FALSE;
    int result = EXIT_FAILURE;

    memset(&ourGlobals, 0, sizeof(gameGlobals));
    ourGlobals.address = "127.0.0.1";
    ourGlobals.port = 8211;

    if (eina_init())
    {
	logDom = HamrTime(argv[0], main, logDom);
	ourGlobals.scripts = eina_hash_string_superfast_new(NULL);

	if (ecore_con_init())
	{
	    LuaSLStream = eina_strbuf_new();
	    clientStream = eina_strbuf_new();
	    reachOut("127.0.0.1", 8211, &ourGlobals, (Ecore_Event_Handler_Cb) _addLuaSL, (Ecore_Event_Handler_Cb) _dataLuaSL, (Ecore_Event_Handler_Cb) _delLuaSL);

		if ((ourGlobals.server = ecore_con_server_add(ECORE_CON_REMOTE_TCP, ourGlobals.address, ourGlobals.port + 1, &ourGlobals)))
		{
		    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,  (Ecore_Event_Handler_Cb) _addClient,  &ourGlobals);
		    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb) _dataClient, &ourGlobals);
		    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,  (Ecore_Event_Handler_Cb) _delClient,  &ourGlobals);
		    ecore_con_server_timeout_set(ourGlobals.server, 0);
		    ecore_con_server_client_limit_set(ourGlobals.server, -1, 0);


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
				snprintf(buf, sizeof(buf), "%s/%s.edj", prefix_data_get(), "love");
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

				snprintf(buf, sizeof(buf), "%s/bubble_sh.png", prefix_data_get());
				for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
				{
				    sh = evas_object_image_filled_add(ourGlobals.canvas);
				    evas_object_image_file_set(sh, buf, NULL);
				    evas_object_resize(sh, 64, 64);
				    evas_object_show(sh);
				    evas_object_data_set(ourGlobals.bg, names[(i * 2) + 1], sh);
				}

				snprintf(buf, sizeof(buf), "%s/bubble.png", prefix_data_get());
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
			    PD("Fell out of the main loop");

			    if (ourGlobals.server)       ecore_con_server_del(ourGlobals.server);
			    if (ourGlobals.serverLuaSL)  ecore_con_server_del(ourGlobals.serverLuaSL);

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
		    PC("Failed to add server!");

	    ecore_con_shutdown();
	}
	else
	    PC("Failed to init ecore_con!");

	eina_hash_free(ourGlobals.scripts);
	pantsOff(logDom);
    }
    else
	fprintf(stderr, "Failed to init eina!");

    PD("Falling out of main()");
    return result;
}
