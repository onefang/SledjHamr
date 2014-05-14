
#include "LuaSL.h"
#include "Runnr.h"


int logDom;	// Our logging domain.
static int CPUs = 4;
static Eina_Strbuf *clientStream;


static Eina_Bool _sleep_timer_cb(void *data)
{
    script *script = data;
    gameGlobals *ourGlobals = script->game;

//    PD("Waking up %s", script->name);
    sendToChannel(ourGlobals, script->SID, "return 0.0");
    return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _timer_timer_cb(void *data)
{
    script *script = data;
    gameGlobals *ourGlobals = script->game;

    PD("Timer for %s", script->name);
    sendToChannel(ourGlobals, script->SID, "events.timer()");
    return ECORE_CALLBACK_RENEW;
}

static script *findThem(gameGlobals *ourGlobals, const char *base, const char *text)
{
    char name[PATH_MAX];
    char *temp;

    strncpy(name, base, PATH_MAX);
    if ((temp = rindex(name, '/')))
	temp[1] = '\0';
    strcat(name, text);
    if ((temp = rindex(name, '"')))
	temp[0] = '\0';
    strcat(name, ".lsl");
    return eina_hash_find(ourGlobals->names, name);
}

static void resetScript(script *victim)
{
  gameGlobals *ourGlobals = victim->game;
  script *me;
  char buf[PATH_MAX];

//  PD("RESETTING %s", victim->name);
  sendToChannel(ourGlobals, victim->SID, "quit()");

  eina_hash_del(ourGlobals->scripts, victim->SID, NULL);
  eina_hash_del(ourGlobals->names, victim->fileName, NULL);

  // The old one will eventually die, create a new one.
  me = calloc(1, sizeof(script));
  gettimeofday(&me->startTime, NULL);
  strncpy(me->SID, victim->SID, sizeof(me->SID));
  strncpy(me->fileName, victim->fileName, sizeof(me->fileName));
  me->name = &me->fileName[sizeof(PACKAGE_DATA_DIR)];
  me->game = ourGlobals;
  me->client = victim->client;
  eina_hash_add(ourGlobals->scripts, me->SID, me);
  eina_hash_add(ourGlobals->names, me->fileName, me);
  sprintf(buf, "%s.lua.out", me->fileName);
  newProc(buf, TRUE, me);
}

void scriptSendBack(void * data)
{
    scriptMessage *message = data;
    gameGlobals *ourGlobals = message->script->game;

    if (!message->script)
    {
      PE("scriptSendBack script is NULL");
      return;
    }

    if (0 == strncmp(message->message, "llSleep(", 8))
	ecore_timer_add(atof(&(message->message)[8]), _sleep_timer_cb, message->script);
    else if (0 == strncmp(message->message, "llSetTimerEvent(", 16))
    {
	message->script->timerTime = atof(&(message->message)[16]);
	if (0.0 == message->script->timerTime)
	{
	    if (message->script->timer)
		ecore_timer_del(message->script->timer);
	    message->script->timer = NULL;
	}
	else
	    message->script->timer = ecore_timer_add(message->script->timerTime, _timer_timer_cb, message->script);
    }
    else if (0 == strncmp(message->message, "llSetScriptState(", 17))
    {
	script *them;

	if ((them = findThem(ourGlobals, message->script->fileName, &(message->message[18]))))
	{
	    char *temp = rindex(&(message->message[18]), ',');

	    if (temp)
	    {
		temp++;
		while (isspace(*temp))
		    temp++;
		if ('1' == *temp)
		    sendToChannel(ourGlobals, them->SID, "start()");
		else
		    sendToChannel(ourGlobals, them->SID, "stop()");
//		PD("Stopped %s", them->name);
	    }
	    else
		PE("Missing script state in llSetScriptState(%s, )", them->name);
	}
	else
	{
	    char *temp = rindex(&(message->message[18]), '"');

	    if (temp)
		*temp = '\0';
	    PE("Can't stop script, can't find %s", &(message->message[18]));
	}
    }
    else if (0 == strncmp(message->message, "llResetOtherScript(", 19))
    {
	script *them;

	if ((them = findThem(ourGlobals, message->script->fileName, &(message->message[20]))))
	    resetScript(them);
    }
    else if (0 == strncmp(message->message, "llResetScript(", 14))
	resetScript(message->script);
    else
	sendBack(message->script->client, message->script->SID, message->message);
    free(message);
}

static Eina_Bool _add(void *data, int type __UNUSED__, Ecore_Con_Event_Client_Add *ev)
{
    ecore_con_client_timeout_set(ev->client, 0);
    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _data(void *data, int type __UNUSED__, Ecore_Con_Event_Client_Data *ev)
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
	    ext[0] = '\0';
	    command = ext + 1;
	    if (0 == strncmp(command, "compile(", 8))
	    {
		char *temp;
		char *file;
		char *name;

		strcpy(buf, &command[8]);
		temp = buf;
		file = temp;
		while (')' != temp[0])
		    temp++;
		temp[0] = '\0';

		name = &file[sizeof(PACKAGE_DATA_DIR)];
		PD("Compiling %s, %s.", SID, name);
		if (compileLSL(ourGlobals, ev->client, SID, file, FALSE))
		{
		    script *me = calloc(1, sizeof(script));

		    gettimeofday(&me->startTime, NULL);
		    strncpy(me->SID, SID, sizeof(me->SID));
		    strncpy(me->fileName, file, sizeof(me->fileName));
		    me->name = &me->fileName[sizeof(PACKAGE_DATA_DIR)];
		    me->game = ourGlobals;
		    me->client = ev->client;
		    eina_hash_add(ourGlobals->scripts, me->SID, me);
		    eina_hash_add(ourGlobals->names, me->fileName, me);
		    sendBack(ev->client, SID, "compiled(true)");
		}
		else
		    sendBack(ev->client, SID, "compiled(false)");
	    }
	    else if (0 == strcmp(command, "run()"))
	    {
		script *me;
		char buf[PATH_MAX];

		me = eina_hash_find(ourGlobals->scripts, SID);
		if (me)
		{
		    sprintf(buf, "%s.lua.out", me->fileName);
		    newProc(buf, TRUE, me);
		}
	    }
	    else if (0 == strcmp(command, "exit()"))
	    {
		PD("Told to exit.");
		ecore_main_loop_quit();
	    }
	    else
	    {
		const char *status = NULL;

		status = sendToChannel(ourGlobals, SID, command);
		if (status)
		    PE("Error sending command %s to script %s : %s", command, SID, status);
	    }
	}

	// Get the next blob to check it.
	command = eina_strbuf_string_get(clientStream);
    }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _del(void *data, int type __UNUSED__, Ecore_Con_Event_Client_Del *ev)
{
//    gameGlobals *ourGlobals = data;

    if (ev->client)
    {
	PD("No more clients, exiting.");
	ecore_con_client_del(ev->client);
	ecore_main_loop_quit();
    }
    return ECORE_CALLBACK_RENEW;
}

int main(int argc, char **argv)
{
  gameGlobals ourGlobals;
  char *env, cwd[PATH_MAX], temp[PATH_MAX * 2];
  int result = EXIT_FAILURE;

  // Sigh, Elm has this great thing for dealing with bin, lib, and data directories, but this is not an Elm app,
  // And Elm is too heavy for just that little bit.
  // So just duplicate a bit of what we need here.  Sorta.
  getcwd(cwd, PATH_MAX);
  env = getenv("LUA_CPATH");
  if (!env)  env = "";
  sprintf(temp, "%s;%s/lib?.so;%s/?.so;%s/?.so", env, PACKAGE_LIB_DIR, PACKAGE_LIB_DIR, cwd);
  setenv("LUA_CPATH", temp, 1);

  env = getenv("LUA_PATH");
  if (!env)  env = "";
  sprintf(temp, "%s;%s/?.lua;%s/?.lua", env, PACKAGE_LIB_DIR, cwd);
  setenv("LUA_PATH", temp, 1);

    memset(&ourGlobals, 0, sizeof(gameGlobals));
    ourGlobals.address = "127.0.0.1";
    ourGlobals.port = 8211;

    if (eina_init())
    {
	logDom = loggingStartup("LuaSL", logDom);
	ourGlobals.scripts = eina_hash_string_superfast_new(NULL);
	ourGlobals.names = eina_hash_string_superfast_new(NULL);
	if (ecore_init())
	{
	    if (ecore_con_init())
	    {
		if ((ourGlobals.server = ecore_con_server_add(ECORE_CON_REMOTE_TCP, ourGlobals.address, ourGlobals.port, &ourGlobals)))
		{
		    int i;
		    Eina_Iterator *scripts;
		    script *me;

		    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,  (Ecore_Event_Handler_Cb) _add,  &ourGlobals);
		    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb) _data, &ourGlobals);
		    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,  (Ecore_Event_Handler_Cb) _del,  &ourGlobals);
		    ecore_con_server_timeout_set(ourGlobals.server, 0);
		    ecore_con_server_client_limit_set(ourGlobals.server, -1, 0);
//		    ecore_con_server_timeout_set(ourGlobals.server, 10);
//		    ecore_con_server_client_limit_set(ourGlobals.server, 3, 0);
		    clientStream = eina_strbuf_new();

		    result = 0;
		    compilerSetup(&ourGlobals);
		    luaprocInit();
		    for (i = 0; i < CPUs; i++)
		    {
			if ( sched_create_worker( ) != LUAPROC_SCHED_OK )
			PE("Error creating luaproc worker thread.");
		    }
		    ecore_main_loop_begin();
		    PD("Fell out of the main loop.");

		    scripts = eina_hash_iterator_data_new(ourGlobals.scripts);
		    while(eina_iterator_next(scripts, (void **) &me))
		    {
			const char *status = NULL;

			status = sendToChannel(&ourGlobals, me->SID, "quit()");
			if (status)
			    PE("Error sending command quit() to script %s : %s", me->SID, status);
		    }

		    PD("Finished quitting scripts.");
		    // TODO - This is what hangs the system, should change from raw pthreads to ecore threads.
		    //        Or perhaps just run the main loop for a bit longer so all the scripts can quit?
		    sched_join_workerthreads();
		}
		else
		    PC("Failed to add server!");
		ecore_con_shutdown();
	    }
	    else
		PC("Failed to init ecore_con!");
	    ecore_shutdown();
	}
	else
	    PC("Failed to init ecore!");
    }
    else
	fprintf(stderr, "Failed to init eina!");

    PD("Falling out of main()");
    return result;
}


/*  Stuff to be merged in later.

#ifdef _WIN32
# define FMT_SIZE_T "%Iu"
#else
# define FMT_SIZE_T "%zu"
#endif

#define MAX_LUA_MEM (64 * 1024))  // LSL Mono is 64KB, edje Lua is 4MB. (4 * (1024 * 1024))

#define _edje_lua2_error(L, err_code) _edje_lua2_error_full(__FILE__, __FUNCTION__, __LINE__, L, err_code)


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

void runLuaFile(gameGlobals *ourGlobals, const char *filename)
{
    PD("Starting %s", filename);
    newProc(filename, TRUE);

// TODO, should set up our panic and errfunc as below.  Plus the other TODO stuff.


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
}
*/




/* What we need to do, and how we might do it.
 *
 * Get configuration info.
 *
 *   Some of the configuration info we need is tucked away in OpenSim .ini files.  So we have to read that.
 *   Eet is probably not so useful here, as we would have to write a UI tool, and there's no UI otherwise.
 *
 * Multi task!
 *
 *   Luaproc starts up X worker threads.  Each one takes the next ready Lua state and runs it until it blocks waiting for a channel message, or yields.
 *   The current mainloop waits for events and commands from the message channel, executes them, then goes back to waiting.
 *   So that is fine in general, so long as the LSL event handlers actually return in a reasonable time.
 *   We need to be able to yield at suitable places, or be able to force a yield.  Both without slowing things down too much.
 *
 * Watchdog thread.
 *
 *   It's easy enough to have a watchdog thread wake up every now and then to check if any given Lua state has been hogging it's CPU for too long.
 *   The hard part is forcing Lua states to yield cleanly, without slowing performance too much.
 *
 * Identifying scripts. - OpenSim/Region/ScriptEngine/Interfaces/IScriptInstance.cs
 *
 *   We need to be able to identify scripts so that messages can get to the right ones.  Also the objects they are in.
 *   And do it all in a way that OpenSim is happy with.
 *   Copies of the same script in the same prim would have the same asset UUID, but different name, though they would run independently.
 *   Copies of the same script in different prims could have the same asset UUID AND the same name.
 *   OpenSim seems to be using a UUID to identify single scripts, and a uint to identify single prims, when sending events for either case.
 *   The UUID is from the script item in the prims inventory.
 *   There is also an asset UUID (the one printed out on the console at script startup time) that points to the source code in the prim.
 *     Which will be identical to the asset UUID for the multiple copies of the same script.
 *
 *   script assetID
 *     UUID of the script source in the grids asset database, also the script source in the prim.
 *
 *   script itemID
 *     UUID of this instance of the running script.
 *     UUID of the scripts binary in the prims inventory.
 *     This is the one used to identify the running script.
 *
 *   prim uint localID
 *     Some sort of number representing the prim the script is running in.
 *     Events are sometimes sent to this.
 *
 *   path/filename
 *     An invention of LuaSL, coz we store stuff as files.
 *
 *   OpenSim says "compile this assetID for this itemID, in this prim uint"
 *     Current infrastructure does not allow easy sending of the script source, but we don't have ROBUST code to get it either.
 *     ROBUST is the way to go though, coz we can sneakily start to suck other stuff, like prim contents across when needed.
 *       Though that sort of thing needs access to the local sim databases to lookup the prim and it's other contents.  sigh
 *       I think that new script and notecard contents get new assetIDs anyway, so keeping an eye on assets.create_time or asset_access_time wont help much.
 *
 *   OpenSim says "start / stop this itemID"
 *     Already catered for.
 *
 *   What does OpenSim REALLY do?
 *
 *     Region/Framework/Scenes/Scene.Inventory.cs - CapsUpdateTaskInventoryScriptAsset(IClientAPI remoteClient, UUID itemId, UUID primId, bool isScriptRunning, byte[] data)
 *         remoteClient
 *         itemID - UUID of the script source.
 *         primID - UUID of the prim it is in.
 *         isScriptRunning
 *         data   - the script source code.
 *       Called when a user saves the script.  itemID stays the same, but we get a new assetID, for the new source code asset.
 *       Looks up the item in the prim.
 *       AssetBase asset = CreateAsset(item.Name, item.Description, (sbyte)AssetType.LSLText, data, remoteClient.AgentId);
 *       AssetService.Store(asset);
 *       stashes the new assetID in the item
 *       updates the item in the prim
 *       if (isScriptRunning)
 *         part.Inventory.RemoveScriptInstance(item.ItemID, false);
 *         part.Inventory.CreateScriptInstance(item.ItemID, 0, false, DefaultScriptEngine, 0);
 *         errors = part.Inventory.GetScriptErrors(item.ItemID);
 *
 *     CreateScriptInstance() is generally called to start scripts, part.ParentGroup.ResumeScripts(); is usually called after CreateScriptInstance()
 *
 *     Region/Framework/Scenes/SceneObjectPartInventory.cs - CreateScriptInstance(UUID itemId, int startParam, bool postOnRez, string engine, int stateSource)
 *       looks up the itemID, then calls the real one -
 *     Region/Framework/Scenes/SceneObjectPartInventory.cs - CreateScriptInstance(TaskInventoryItem item, int startParam, bool postOnRez, string engine, int stateSource)
 *       get the asset from the asset database using the items assetID
 *       restores script state if needed
 *       converts asset.data to a string called script
 *       m_part.ParentGroup.Scene.EventManager.TriggerRezScript(m_part.LocalId, item.ItemID, script, startParam, postOnRez, engine, stateSource);
 *       QUIRK - if it's a sim border crossing, TriggerRezScript() gets called with empty script source.
 *
 *     Region/ScriptEngine/XEngine/XEngine.cs - AddRegion(Scene scene)
 *       m_log.InfoFormat("[XEngine] Initializing scripts in region {0}", scene.RegionInfo.RegionName);
 *       gets the script config info, which is the same damn stuff for each sim.  Pffft
 *       Think it relies on the scenes event manager to call OnRezScript() -
 *         m_Scene.EventManager.OnRezScript += OnRezScript;
 *
 *     Region/Framework/Scenes/EventManager.cs - TriggerRezScript(uint localID, UUID itemID, string script, int startParam, bool postOnRez, string engine, int stateSource)
 *       Loops through Scene.EventManager.OnRezScript calling them.
 *
 *     Region/ScriptEngine/XEngine/XEngine.cs - OnRezScript(uint localID, UUID itemID, string script, int startParam, bool postOnRez, string engine, int stateSource)
 *       Looks at the script source to figure out if it's an XEngine script.
 *       Either queues the script for later, or does it direct.
 *       Region/ScriptEngine/XEngine/XEngine.cs - DoOnRezScript() is passed an array holding -
 *           localID is a uint that represents the containing prim in the current scene
 *           itemID is the UUID of the script in the prims contents
 *           script is the script source code.
 *           startParam is the scripts startParam
 *           postOnRez
 *           stateSource is an integer saying how we where started, used to trigger the appropriate startup events.
 *         uses localID to look up the prim in the scene, then looks inside that for the itemID to find the assetID.
 *         m_Compiler.PerformScriptCompile(script, assetID.ToString(), item.OwnerID, out assembly, out linemap);
 *           Which is in Region/ScriptEngine/Shared/CodeTools/Compiler.cs
 *         instance = new ScriptInstance(this, part, itemID, assetID, assembly, m_AppDomains[appDomain], part.ParentGroup.RootPart.Name, item.Name, startParam, postOnRez, stateSource, m_MaxScriptQueue);
 *           Region/ScriptEngine/Shared/Instance/ScriptInstance.cs - ScriptInstance(IScriptEngine engine, SceneObjectPart part, UUID itemID, UUID assetID, string assembly, AppDomain dom, string primName, string scriptName, int startParam, bool postOnRez, StateSource stateSource, int maxScriptQueue)
 *             inits all the APIs
 *             loads in any saved state if it can find one
 *         m_log.DebugFormat("[XEngine] Loaded script {0}.{1}, script UUID {2}, prim UUID {3} @ {4}.{5}", part.ParentGroup.RootPart.Name, item.Name, assetID, part.UUID, part.ParentGroup.RootPart.AbsolutePosition, part.ParentGroup.Scene.RegionInfo.RegionName);
 *
 *   Soooo, when a script is saved -
 *     the new source is saved in the asset database
 *     The script item in the prim gets the new assetID
 *     if the script is running -
 *       remove the old script instance (item.ItemID)
 *       create a new one (item.ItemID)
 *         get the source code from the asset database (item.assetID)
 *         restore script state
 *         TriggerRezOnScript()
 *           Loop through all those that are interested, incuding XEngine.onRezScript()
 ***           check the first line to see if it's an XEngine script
 *               sooner or later passes it to XEngine.DoOnRezScript()
 *                 looks up localID to get the prim
 *                 looks inside prim to get the script from itemID
 *                 gets the assetID from the script item
 *                 compiles the script
 *                 creates the script instance
 *                   loads up the APIs
 *                   restores any script state
 *                 calls instance.Init() which is Region/ScriptEngine/Shared/Instance/ScriptInstance.cs - Init()
 *                   passes the usual startup events to the script.
 *     part.ParentGroup.ResumeScripts()
 *
 *   At the *** marked point, LuaSL.onRezScript should -
 *     check the first line to see if it's an LuaSL script
 *       looks up localID to get the prim
 *       looks inside prim to get the script from itemID
 *       gets the assetID from the script item
 *       filename encode the sim name, object name, and script name
 *         replace anything less than 0x21, DEL " * / : < > ? \ | + [ ] - , . ( ) $ % # @ from - http://en.wikipedia.org/wiki/Filename plus a few more
 *         THEN reduce to 254 characters
 *           NOTE the object names might be identical, disambiguate them.
 *       write the script to a file - /script/engine/path/sim_name/objects/object_name/script_name
 *       send the itemID.compile(/script/engine/path/sim_name/objects/object_name/script_name) message to the script engine's socket
 *
 *
 * Object inventory "cache".
 *
 *   This code currently pretends that there is a local file based sim object store available.
 *   I think it would be a good idea to abuse the OpenSim cache system to produce that file based object store.
 *   It will help with the "damn OpenSim's asset database has to be a bottomless pit" monster design flaw.
 *   Prim contents must all be unique names anyway, and there are SOME constraints on contents names, so probably don't have to do much to convert an item name to a legal file name.
 *     Oops, names can have directory slashes in them.  lol
 *   On the other hand, sim objects CAN have the same name.
 *
 *   So we got sim directories, with an objects directory inside it, with object directories inside that.  The object directories have object files in them.  This is all like the test setup that is here.
 *   We need metadata.  Sim metadata, object metadata, and object contents metadata.  That can be done with a "foo.omg" file at each level.
 *     sim/index.omg - the list of object name.UUIDs, their X,Y,Z location, size, and rotation.
 *     sim/objects/objectName_UUID/index.omg - the list of contents names, item UUIDs, asset UUIDs, and types.
 *     sim/objects/objectName/subObjectName - the list of ITS contents names, item UUIDs, asset UUIDs, and types.
 *
 * Script start, stop, reset. - OpenSim/Region/ScriptEngine/Interfaces/IScriptEngine.cs
 *
 *   Scripts and users have to be able to start, stop, and reset scripts.
 *   Users have to be able to see the start / stopped status of scripts from the viewer.
 *   Which means if the script does it, we have to tell OpenSim.
 *   Naturally, if OpenSim does it, it has to tell us.
 *   Should be able to do both by passing textual Lua function calls between OpenSim and LuaSL.
 *
 * Event handling, llDetect*() functions.
 *
 *   OpenSim will generate events at random times and send us relevant information for llDetect*() functions and the handler calls.
 *   These should come through the scripts main loop eventually.
 *
 * Send messages from / to OpenSim and ROBUST.
 *
 *   ROBUST uses HTTP for the communications, and some sort of XML, probably XML-RPC.
 *   OpenSim has some sort of generic mechanism for talking to script engines in C#.  I want to turn that into a socket based, pass textual Lua function calls, type system.
 *     That assumes C# has some sort of semi decent introspection or reflection system.
 *     After a minimum of research, I have come to the conclusion that C# has suitable introspection, and will go ahead with my plans, leaving that problem to the C# coders.
 *     Looks like OpenSim is already using a bit of C# introspection for ll*() functions anyway.
 *     The scripts main loop can already deal with incoming commands as a string with a Lua function call in it.
 *
 * Send messages from / to Lua or C, and wait or not wait.
 *
 *   Luaproc channels are distinct from Lua states, but some Lua state has to create them before they can be used.
 *     On the other hand, looks like broadcasting messages is not really catered for, it's first come first served.
 *   luaproc.send() can send multiple messages to a single channel.  It blocks if no one is listening.
 *     This was done to simplify things for the luaproc programmers, who suggest creating more Lua states to deal with asynchronous message sending.
 *   luaprog.receive() gets a channel message.  It can block waiting, or not block.
 *   I already hacked up C code to send and not block.  I might have broken the luaproc.send() ability to send multiple messages.
 *     Before I hacked it up, actual message sending was done by copying the contents of the sending Lua states stack to the receiver states stack.
 *     This is the simple method for the luaproc programmers, as both states are in the context of a luaproc call, so both stacks are available.
 *     My hacked up version either takes one message from the sender, or is passed one from C.  The C call just returns if there is no one waiting on that channel.
 *     luaproc.send() calls that C function after taking a single message from the stack, and block waits as usual if the C call cannot deliver.
 *   Don't think there is C to receive messages, luaproc seems to be lacking entirely in C side API.
 *   NOTE - Sending from C means that the message goes nowhere if no one is waiting for it.
 *     SOOOO, we may need to queue messages to.
 *     Just chuck them in a FIFO per channel, and destroy the FIFO when the channel get's destroyed.
 *     See if I can create the SID channel in C before I start the Lua state running.
 *   Edje messages might have to be used instead, or some hybrid.
 *
 *   Main loop is waiting on messages, and that's the main driver.  Luaproc is fine with that.  Good for events.
 *   End of event handler -
 *     just wait for the next event.
 *   Stop a script from LSL -
 *     gotta find it's SID from it's name, and the prim UUID
 *     send the message
 *     wait for it to get the message - BUT we don't really want to wait.
 *   Stop a script from OpenSim -
 *     we should have it's SID from OpenSim, just send the message from C, no need to wait.
 *   Start a script -
 *     if it's stopped, it's already waiting for the message.
 *     if it's not stopped, then we don't care.  BUT then we might be waiting for it to get the message if starting it from LSL.
 *   Reset a script -
 *     probably should be done from C anyway, and can reuse the libraries like luaproc likes to do.
 *     ask C to reset it.
 *   LSL calls a function we have to hand to OpenSim -
 *     send the message to C, wait.
 *     C eventually sends a message back.
 *   Sleep -
 *     tell C it's waiting for the wake up message.
 *     wait for the wake up message.
 *
 *   C needs -
 *     Lua call for stop script.
 *       get the SID from the name, and the prim UUID.
 *       send the stop message to the SID.
 *       send something to OpenSim so it knows.
 *       return to Lua.
 *     Lua call for start script.
 *       get the SID from the name, and the prim UUID.
 *       send the start message to the SID.
 *       send something to OpenSim so it knows.
 *       return to Lua.
 *     Lua call for reset other script.
 *       get the SID from the name, and the prim UUID.
 *       figure out which Lua state it is.
 *       fall through to "reset this script", only with the script set to the found one.
 *     Lua call for reset this script.
 *       get luaproc to close this Lua state
 *       reload the script file
 *       start it again, reusing the previous Lua state, or which ever one luaproc wants to use.
 *     Lua call for sending a function to OpenSim.
 *       Lua first strings up the function call and args, with SID.
 *       C packs it off to opensim.
 *       C puts Lua state on the "waiting for message" queue if a return value is needed.
 *       OpenSim sends back the return value, business as usual.
 *     Lua call for sleep.
 *       setup an ecore timer callback
 *       put the Lua state into "waiting for message" queue.
 *       ecore timer callback sends the wake up message.
 *
 * Time and timers, plus deal with time dilation.
 *
 *   Various LSL functions deal with time, that's no problem.
 *   Some might deal with time dilation, which means dealing with that info through OpenSim.
 *   There is a timer event, which should be done through ecore timers and whatever message system we end up with.
 *   Finally, a sleep call, which can be done with ecore timer and messages to.
 *
 * Deal directly with MySQL and SQLite databases.
 *
 *   The script engine can run on the same computer as the sim server, that's how OpenSim does stuff.  So we can directly access the database the sim server has, which gives us access to sim object metadata.
 *   Changing that metadata might require us to inform OpenSim about the changes.  It's entirely possible that different changes do or do not need that.
 *   Esskyuehl may be suitable, though it's still in the prototype stage.
 *
 * Serialise the script state, send it somewhere.
 *
 *   Lua can generally serialise itself as as a string of code to be executed at the destination.  There might be some C side state that needs to be take care of as well.  We shall see.
 *
 * Email, HTTP, XML-RPC?
 *
 *   LSL has functions for using these as communications methods.  We should implement them ourselves eventually, but use the existing OpenSim methods for now.
 *   Note that doing it ourselves may cause issues with OpenSim doing it for some other script engine.
 *   Azy might be suitable, but it's also in prototype.
 *
*/
