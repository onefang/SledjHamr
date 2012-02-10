
#include "LuaSL.h"


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

void runnerSetup(gameGlobals *game)
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

void runLuaFile(gameGlobals *game, const char *filename)
{
//    PD("Starting %s", filename);
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

void runnerTearDown(gameGlobals *game)
{
// TODO - this is what hangs the system.
    sched_join_workerthreads();
}


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
 *     luaproc.send() cals that C function after taking a single message from the stack, and block waits as usual if the C call cannot deliver.
 *   Don't think there is C to receive messages, luaproc seems to be lacking entirely in C side API.
 *   Edje messages might have to be used instead, or some hybrid.
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
 * Object inventory "cache".
 *
 *   This code currently pretends that there is a local file based sim object store available.
 *   I think it would be a good idea to abuse the OpenSim cache system to produce that file based object store.
 *   It will help with the "damn OpenSim's asset database has to be a bottomless pit" monster design flaw.
 *
*/
