
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
