
#include "LuaSL.h"


static int CPUs = 4;
static Eina_Strbuf *clientStream;


static Eina_Bool _add(void *data, int type __UNUSED__, Ecore_Con_Event_Client_Add *ev)
{
    ecore_con_client_timeout_set(ev->client, 0);
    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _data(void *data, int type __UNUSED__, Ecore_Con_Event_Client_Data *ev)
{
    gameGlobals *game = data;
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
	ext = rindex(SID, '.');
	if (ext)
	{
	    ext[0] = '\0';
	    command = ext + 1;
	    if (0 == strcmp(command, "compile()"))
	    {
		PD("Compiling %s.", SID);
		if (compileLSL(game, SID, FALSE))
		    sendBack(game, ev->client, SID, "compiled(true)");
		else
		    sendBack(game, ev->client, SID, "compiled(false)");
	    }
	    else if (0 == strcmp(command, "start()"))
	    {
		char buf[PATH_MAX];

		sprintf(buf, "%s.lua.out", SID);
		newProc(buf, TRUE);
	    }
	    else if (0 == strcmp(command, "exit()"))
	    {
		PD("Told to exit.");
		ecore_main_loop_quit();
	    }
	    else
	    {
		const char *status = NULL;

		status = sendToChannel(SID, command, NULL, NULL);
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
    gameGlobals *game = data;

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
    gameGlobals game;
    int result = EXIT_FAILURE;

    memset(&game, 0, sizeof(gameGlobals));
    game.address = "127.0.01";
    game.port = 8211;

    if (eina_init())
    {
	loggingStartup(&game);
	if (ecore_con_init())
	{
	    if ((game.server = ecore_con_server_add(ECORE_CON_REMOTE_TCP, game.address, game.port, &game)))
	    {
		ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,  (Ecore_Event_Handler_Cb) _add,  &game);
		ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb) _data, &game);
		ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,  (Ecore_Event_Handler_Cb) _del,  &game);
		ecore_con_server_timeout_set(game.server, 0);
		ecore_con_server_client_limit_set(game.server, -1, 0);
		clientStream = eina_strbuf_new();

		if (edje_init())
		{
		    int i;

		    result = 0;
		    compilerSetup(&game);
		    luaprocInit();
		    for (i = 0; i < CPUs; i++)
		    {
			if ( sched_create_worker( ) != LUAPROC_SCHED_OK )
			    PEm("Error creating luaproc worker thread.");
		    }
		    ecore_main_loop_begin();

		    // TODO - this is what hangs the system, should change from raw pthreads to ecare threads.
		    sched_join_workerthreads();
		    edje_shutdown();
		}
		else
		    PCm("Failed to init edje!");
	    }
	    else
		PCm("Failed to add server!");
	    ecore_con_shutdown();
	}
	else
	    PCm("Failed to init ecore_con!");
    }
    else
	fprintf(stderr, "Failed to init eina!");

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

void runLuaFile(gameGlobals *game, const char *filename)
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
