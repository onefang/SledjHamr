/*  Runnr - a library that deals with running Lua scripts.
*/


#include "LumbrJack.h"
#include "Runnr.h"


static Ecore_Idle_Enterer	*enterer;
static Eina_Clist 		scripts;

static int _send(lua_State *L);
static int _receive(lua_State *L);
static void _cancel(void *data, Ecore_Thread *thread);


static const struct luaL_reg runnrFunctions[] =
{
  { "send", _send },
  { "receive", _receive },
  { NULL, NULL }
};


void dumpStack(lua_State *L, int i)
{
  int type = lua_type(L, i);
  const char *t = lua_typename(L, type);

  printf("Stack %d is %s", i, t);
  switch (type)
  {
    case LUA_TNONE		:  break;
    case LUA_TNIL		:  break;
    case LUA_TBOOLEAN		:  printf(" - %d", lua_toboolean(L, i));  break;
    case LUA_TNUMBER		:  printf(" - %f", lua_tonumber(L, i));  break;
    case LUA_TSTRING		:  printf(" - %s", lua_tostring(L, i));  break;
    case LUA_TFUNCTION		:  break;
    case LUA_TTHREAD		:  break;
    case LUA_TTABLE		:
    {
      int j;

      lua_getfield(L, i, "_NAME");
      j = lua_gettop(L);
      if (lua_isstring(L, j))
        printf(" - %s", lua_tostring(L, j));
      lua_pop(L, 1);
      break;
    }
    case LUA_TUSERDATA		:  break;
    case LUA_TLIGHTUSERDATA	:  break;
    default			:  printf("- unknown!");  break;
  }
  printf("\n");
}

static int traceBack(lua_State *L)
{
  lua_Debug ar;
  int i, top = lua_gettop(L), b = 1;

//  printf("Stack is %d deep\n", top);
//  for (i = 1; i <= top; i++)
//    dumpStack(L, i);

  if (top)
    printf("Lua error - %s\n", lua_tostring(L, 1));

  i = 0;
  while (lua_getstack(L, i++, &ar))
  {
    if (lua_getinfo(L, "nSlu", &ar))
    {
      if (NULL == ar.name)
	ar.name = "DUNNO";
      printf("  Lua backtrace %d - %s %s %s @ %s : %d\n", i, ar.what, ar.namewhat, ar.name, ar.short_src, ar.currentline);
      b = 0;
    }
    else
      printf("  Failed to get trace line!\n");
  }

  if (b)
    printf("  NO BACKTRACE!\n");

  return 0;
}

static void printLuaError(int err, char *string, lua_State *L)
{
  const char *err_type;

  switch (err)
  {
    case LUA_ERRRUN:	err_type = "runtime";		break;
    case LUA_ERRSYNTAX:	err_type = "syntax";		break;
    case LUA_ERRMEM:	err_type = "memory allocation";	break;
    case LUA_ERRERR:	err_type = "error handler";	break;
    default:		err_type = "unknown";		break;
  }
  printf("Error running - %s, \n  %s - %s\n", string, err_type, lua_tostring(L, -1));
}

static int panics = 0;
static int _panic(lua_State *L)                   // Stack usage [-0, +0, m]
{
  // If we somehow manage to have multiple panics, it's likely due to being out
  // of memory in the following lua_tostring() call.
  panics++;
  if (panics)
    printf("Lua PANICS!!!!!");
  else
    printf("Lua PANIC!!!!!: %s", lua_tostring(L, -1));  // Stack usage [-0, +0, m]
  // The docs say that this will cause an exit(EXIT_FAILURE) if we return,
  // and that we we should long jump some where to avoid that.  This is only
  // called for things not called from a protected environment.  We always
  // use pcalls though, except for the library load calls.  If we can't load
  // the standard libraries, then perhaps a crash is the right thing.
  //
  // The above is not true when we deal with the threaded stuff, that uses lua_resume().
  return 0;
}

void printScriptsStatus()
{
  int active, pending_total, pending_feedback, pending_short, available;

  active = ecore_thread_active_get();
  pending_total = ecore_thread_pending_total_get();
  pending_feedback = ecore_thread_pending_feedback_get();
  pending_short = ecore_thread_pending_get();
  available = ecore_thread_available_get();

  printf("Scripts - active %d available %d pending short jobs %d pending feedback jobs %d pending total %d\n", 
    active, available, pending_short, pending_feedback, pending_total);
}

void doLuaString(lua_State *L, char *string, char *module)
{
  int _T, _A, err;

  lua_pushcfunction(L, traceBack);
  _T = lua_gettop(L);

  if (luaL_loadstring(L, string))
  {
    const char *err = lua_tostring(L, 1);

    printf("Error parsing - %s, ERROR %s", string, err);
  }
  else
  {
    _A = lua_gettop(L);
    if (module)
    {
      lua_getfield(L, LUA_REGISTRYINDEX, module);

      // Consistancy would be good, just sayin'.
      if (0 == lua_setfenv(L, _A))
      {
        printf("Error setting environment for - %s", string);
        return;
      }
    }

//printf("doLuaString(%s)\n", string);
    if ((err = lua_pcall(L, 0, LUA_MULTRET, _T)))
      printLuaError(err, string, L);
  }
}

static void _stopScript(script *s)
{
  scriptMessage *sm0, *sm1;

//printf("^^^^^^^^^^^^^^^^^^^_stop(, %s)\n", s->name);
  if (s->L)		lua_close(s->L);	s->L = NULL;
  if (s->timer)	ecore_timer_del(s->timer);	s->timer = NULL;
  EINA_CLIST_FOR_EACH_ENTRY_SAFE(sm0, sm1, &(s->messages), scriptMessage, node)
  {
    eina_clist_remove(&(sm0->node));
    free(sm0);
  }
  s->status = RUNNR_NOT_STARTED;
}

static void _workerFunction(void *data, Ecore_Thread *thread)
{
  script *s = data;
  scriptMessage *msg = NULL;
  const char *message = NULL;

  takeScript(s);
//  if (RUNNR_FINISHED == s->status)
//  {
//    releaseScript(s);
//    return;
//  }

  // The documentation is not clear on which thread is which inside and out,
  // but states that at least for some they are different.
  // So store the internal one as well.
#if THREADIT
  s->me = thread;
#endif

  if (RUNNR_RESET == s->status)
    _stopScript(s);

  if (RUNNR_READY == s->status)
  {
//printf("_workerFunction()  READY %s\n", s->name);
    if ((msg = (scriptMessage *) eina_clist_head(&(s->messages))))
    {
      eina_clist_remove(&(msg->node));
      message = msg->message;
      if (s->L)
        s->status = RUNNR_RUNNING;
      else
        s->status = RUNNR_NOT_STARTED;
    }
  }

  if (RUNNR_NOT_STARTED == s->status)
  {
    int err;

//printf("_workerFunction()  STARTING %s\n", s->name);
    s->status = RUNNR_RUNNING;
    s->L = luaL_newstate();	// Sets a standard allocator and panic function.

    lua_atpanic(s->L, _panic);
    // TODO - Set our allocator here.
    luaL_openlibs(s->L);
    luaL_register(s->L, "Runnr", runnrFunctions);

    // Store the script struct in its own Lua state,
    lua_pushlightuserdata(s->L, s);
    lua_setfield(s->L, LUA_REGISTRYINDEX, "_SELF");

    err = luaL_loadfile(s->L, s->binName);
    if (err != 0)
      s->status = RUNNR_FINISHED;
    gettimeofday(&s->startTime, NULL);
  }

  releaseScript(s);

  if (RUNNR_RUNNING == s->status)
  {
    int stat;

//printf("_workerFunction()  RUNNING %s %s\n", s->name, message);
    // Resume running the script.
    // lua_resume() needs a Lua thread, and the initial Lua state is a thread.
    // Other Lua threads have their own state, but share the environment with the initial state.
    // In theory lua_resume() is a pcall.
    if (message)
      lua_pushstring(s->L, message);
    stat = lua_resume(s->L, (message) ? 1 : 0);
    free(msg);

    takeScript(s);
    // If the script finished.
    if (stat == 0)
      s->status = RUNNR_FINISHED;
    else if (stat != LUA_YIELD)
    {
      printf("lua_resume error at %s\n", s->name);
      printLuaError(stat, s->name, s->L);
      s->status = RUNNR_FINISHED;
    }
    else
    {
      if (eina_clist_count(&s->messages) == 0)
        s->status = RUNNR_WAIT;
      else
        s->status = RUNNR_READY;
    }
    releaseScript(s);
  }

  takeScript(s);
  // Start again from the top when Ecore_Thread has a spare thread ready, unless the script finished.
  if (RUNNR_FINISHED == s->status)
  {
//printf("_workerFunction()  FINISHED %s\n", s->name);
#if THREADIT
    ecore_thread_cancel(thread);
#else
    _cancel(s, NULL);
#endif
  }
  else if (RUNNR_WAIT == s->status)
  {
;//printf("_workerFunction()  WAIT    %s\n", s->name);
  }
#if THREADIT
  else if (RUNNR_READY == s->status)
    ecore_thread_reschedule(thread);
#endif
  releaseScript(s);
}

static void _notify(void *data, Ecore_Thread *thread, void *message)
{
  script *s = data;

  if (s->send2server)  s->send2server(s, message);
  free(message);
}

static void _cancel(void *data, Ecore_Thread *thread)
{
  script *s = data;

  takeScript(s);
  _stopScript(s);
  s->status = RUNNR_FINISHED;
  eina_clist_remove(&(s->node));
  if (s->SID[0])	ecore_thread_global_data_del(s->SID);	s->SID[0] = 0;
//printf("^^^^^^^^^^^^^^^^^^^_del(, %s)\n", s->name);
  // TODO - Perhaps have our own deletion callback to pass back?
  releaseScript(s);
#if THREADIT
  eina_lock_free(&s->mutex);
#endif
  free(s);
}

#if THREADIT
static void _end(void *data, Ecore_Thread *thread)
{
}
#endif

static Eina_Bool _enterer(void *data)
{
  script *s, *s1;

  EINA_CLIST_FOR_EACH_ENTRY_SAFE(s, s1, &(scripts), script, node)
  {
    takeScript(s);
    if ((RUNNR_WAIT == s->status) && (eina_clist_count(&s->messages)))
    {
      s->status = RUNNR_READY;
#if THREADIT
      ecore_thread_feedback_run(_workerFunction, _notify, _end, _cancel, s, EINA_FALSE);
#else
      _workerFunction(s, NULL);
#endif
    }
    if ((RUNNR_RESET == s->status) || (RUNNR_READY == s->status) || (RUNNR_RUNNING == s->status) || (RUNNR_NOT_STARTED == s->status))
    {
#if THREADIT
      ecore_thread_feedback_run(_workerFunction, _notify, _end, _cancel, s, EINA_FALSE);
#else
      _workerFunction(s, NULL);
#endif
    }
    releaseScript(s);
  }

  return ECORE_CALLBACK_RENEW;
}

script *scriptAdd(char *file, char *SID, RunnrServerCb send2server, void *data)
{
  script *result;

  if (!enterer)
  {
    eina_clist_init(&(scripts));
    enterer = ecore_idler_add(_enterer, NULL);
  }

  result = calloc(1, sizeof(script));
  gettimeofday(&result->startTime, NULL);
  strcpy(result->SID, SID);
  result->status = RUNNR_NOT_STARTED;
  result->data = data;
  result->send2server = send2server;

  strncpy(result->fileName, file, sizeof(result->fileName));
  result->name = &result->fileName[strlen(prefix_data_get())];
  sprintf(result->binName, "%s.lua.out", result->fileName);

#if THREADIT
  eina_lock_new(&result->mutex);
#endif
  eina_clist_init(&(result->messages));
  ecore_thread_global_data_add(result->SID, result, NULL, EINA_FALSE);

  eina_clist_add_tail(&(scripts), &(result->node));

  return result;
}

static int luaWriter(lua_State *L, const void* p, size_t sz, void* ud)
{
    FILE *out = ud;
    int result = 0;

    if (sz != fwrite(p, 1, sz, out))
	result = -1;
    return result;
}

// TODO - This didn't help the compile time much, perhaps move the rest of the compiling stage into this thread as a callback?
static void _compileNotify(void *data, Ecore_Thread *thread, void *message)
{
  LuaCompiler *compiler = data;

  if (compiler->cb)  compiler->cb(compiler);
}

static void _compileThread(void *data, Ecore_Thread *thread)
{
  LuaCompiler *compiler = data;
  char name[PATH_MAX];
  lua_State *L;
  FILE *out;
  int err;

  strcpy(name, compiler->luaName);
  if ((L = luaL_newstate()))
  {
    luaL_openlibs(L);
    // This ends up pushing a function onto the stack.  The function is the compiled code.
    err = luaL_loadfile(L, name);
    if (err)
    {
      compiler->bugCount++;
      if (LUA_ERRSYNTAX == err)
	printf("Lua syntax error in %s: %s\n", name, lua_tostring(L, -1));
      else if (LUA_ERRFILE == err)
	printf("Lua compile file error in %s: %s\n", name, lua_tostring(L, -1));
      else if (LUA_ERRMEM == err)
	printf("Lua compile memory allocation error in %s: %s\n", name, lua_tostring(L, -1));
    }
    else
    {
      // Write the compiled code to a file.
      strcat(name, ".out");
      out = fopen(name, "w");
      if (out)
      {
	err = lua_dump(L, luaWriter, out);
	if (err)
	{
	  compiler->bugCount++;
	  printf("Lua compile file error writing to %s\n", name);
	}
	fclose(out);
      }
      else
      {
	compiler->bugCount++;
	printf("CRITICAL! Unable to open file %s for writing!\n", name);
      }
    }
  }
  else
  {
    compiler->bugCount++;
    printf("Can't create a new Lua state!\n");
  }

  ecore_thread_feedback(thread, compiler);
}

void compileScript(LuaCompiler *compiler)
{
  ecore_thread_feedback_run(_compileThread, _compileNotify, NULL, NULL, compiler, EINA_FALSE);
}

// Assumes the scripts mutex is taken already.
void runScript(script *s)
{
#if THREADIT
  if ((RUNNR_NOT_STARTED == s->status) || (RUNNR_FINISHED == s->status))
#endif
  {
#if THREADIT
    ecore_thread_feedback_run(_workerFunction, _notify, _end, _cancel, s, EINA_FALSE);
#else
    _workerFunction(s, NULL);
#endif
  }
}

void resetScript(script *s)
{
  takeScript(s);
  s->status = RUNNR_RESET;
  releaseScript(s);
}

script *getScript(char *SID)
{
  script *result = ecore_thread_global_data_find(SID);

  if (result)
  {
    takeScript(result);
    if (RUNNR_FINISHED == result->status)
    {
      releaseScript(result);
      result = NULL;
    }
  }
  return result;
}

void takeScript(script *s)
{
#if THREADIT
  Eina_Lock_Result result = eina_lock_take(&s->mutex);
  if (EINA_LOCK_DEADLOCK == result)  printf("Script %s IS DEADLOCKED!\n", s->name);
  if (EINA_LOCK_FAIL     == result)  printf("Script %s LOCK FAILED!\n",   s->name);
#endif
}

void releaseScript(script *s)
{
#if THREADIT
  eina_lock_release(&s->mutex);
#endif
}

void send2script(const char *SID, const char *message)
{
  if (SID)
  {
    script *s = ecore_thread_global_data_find(SID);

    if (s)
    {
      takeScript(s);
      if (RUNNR_FINISHED != s->status)
      {
        scriptMessage *sm = NULL;

        if ((sm = malloc(sizeof(scriptMessage))))
        {
          runnrStatus stat;

          sm->s = s;
          strcpy((char *) sm->message, message);
          eina_clist_add_tail(&(s->messages), &(sm->node));

	  stat = s->status;
	  s->status = RUNNR_READY;
	  if (RUNNR_WAIT == stat)
#if THREADIT
            ecore_thread_feedback_run(_workerFunction, _notify, _end, _cancel, s, EINA_FALSE);
#else
	    _workerFunction(s, NULL);
#endif
        }
      }
      releaseScript(s);
    }
  }
}

static script *_getSelf(lua_State *L)
{
  script *s;

  lua_getfield(L, LUA_REGISTRYINDEX, "_SELF");
  s = (script *) lua_touserdata(L, -1);
  lua_pop(L, 1);

  return s;
}

static int _send(lua_State *L)
{
  script *self = _getSelf(L);
  const char *SID = NULL, *message = luaL_checkstring(L, 2);

  if (lua_isstring(L, 1))
    SID = lua_tostring(L, 1);

  if (SID)
    send2script(SID, message);
  else
  {
    takeScript(self);
#if THREADIT
    ecore_thread_feedback(self->me, strdup(message));
#else
    _notify(self, NULL, strdup(message));
#endif
    releaseScript(self);
  }

  return 0;
}

static int _receive(lua_State *L)
{
  script *self = _getSelf(L);

  takeScript(self);
  self->status = RUNNR_WAIT;
  releaseScript(self);
  return lua_yield(L, 0);
}


// These are what the various symbols are for each type -
//  int		%
//  num		#
//  str		$
//  bool	!
//  C func	&
//  lightuserdata	*
//  table.field	@  Expects an integer and a string.
//  nil		~
//  table       {} Starts and stops filling up a new table.
//              (  Just syntax sugar for call.
//  call        )  Expects an integer, the number of results left after the call.
// TODO: Still to do, if we ever use them -
//  table	Some way to specify an arbitrary table, though this sucks, coz C can't deal with them easily.
//  stack	=  Get a value from the stack, expects a stack index.
//  userdata	+
//  thread	^

static char *_push_name(lua_State *L, char *q, int *idx)  // Stack usage [-0, +1, e or m]
{
  char *p = q;
  char temp = '\0';

  // A simplistic scan through an identifier, it's wrong, but it's quick,
  // and we don't mind that it's wrong, coz this is only internal.
  while (isalnum((int)*q))
    q++;
  temp = *q;
  *q = '\0';
  if (*idx > 0)
    lua_getfield(L, *idx, p);    // Stack usage [-0, +1, e]
  else
  {
    if (p != q)
      lua_pushstring(L, p);       // Stack usage [-0, +1, m]
    else
    {
      lua_pushnumber(L, (lua_Number) (0 - (*idx)));
      (*idx)--;
    }
  }
  *q = temp;

  return q;
}

/* It's the callers job to stash things safely before returning from the Lua to C function call.
 * Coz things like strings might go away after the stack is freed.
 */
int pull_lua(lua_State *L, int i, char *params, ...)         // Stack usage -
                                                             // if i is a table
                                                             //   [-n, +n, e]
                                                             // else
                                                             //   [-0, +0, -]
{
   va_list vl;
   char *f = strdup(params);
   char *p = f;
   int n = 0, j = i, count = 0;
   Eina_Bool table = EINA_FALSE;

   if (!f) return -1;
   va_start(vl, params);

   if (lua_istable(L, i))                                                // Stack usage [-0, +0, -]
     {
        j = -1;
        table = EINA_TRUE;
     }

   while (*p)
     {
        char *q;
        Eina_Bool get = EINA_TRUE;

        while (isspace((int)*p))
           p++;
        q = p + 1;
        switch (*p)
          {
             case '%':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_isnumber(L, j))                                // Stack usage [-0, +0, -]
                    {
                       int *v = va_arg(vl, int *);
                       *v = lua_tointeger(L, j);                         // Stack usage [-0, +0, -]
                       n++;
                    }
                  break;
               }
             case '#':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_isnumber(L, j))                                // Stack usage [-0, +0, -]
                    {
                       double *v = va_arg(vl, double *);
                       *v = lua_tonumber(L, j);                          // Stack usage [-0, +0, -]
                       n++;
                    }
                  break;
               }
             case '$':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_isstring(L, j))                                // Stack usage [-0, +0, -]
                    {
                       char **v = va_arg(vl, char **);

                       // We could strdup the string, but that causes leaks.
                       // The problem is that the caller doesn't know if we allocated or not,
                       // since the incoming pointer could already be pointing to a default value.
                       // Lua says the string is valid until it's popped off the stack,
                       // and this is used only in calls to C functions from Lua.
                       // So just document that it's the callers job to stash it safely if needed after returning.
                       *v = (char *) lua_tostring(L, j);
                       n++;
                    }
                  break;
               }
             case '!':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_isboolean(L, j))                               // Stack usage [-0, +0, -]
                    {
                       int *v = va_arg(vl, int *);
                       *v = lua_toboolean(L, j);                         // Stack usage [-0, +0, -]
                       n++;
                    }
                  break;
               }
             case '*':
               {
                  if (table)  q = _push_name(L, q, &i);                     // Stack usage [-0, +1, e]
                  if (lua_islightuserdata(L, j))                               // Stack usage [-0, +0, -]
                    {
                       void **v = va_arg(vl, void **);
                       *v = lua_touserdata(L, j);                         // Stack usage [-0, +0, -]
                       n++;
                    }
                  break;
               }
             default:
               {
                  get = EINA_FALSE;
                  break;
               }
          }

        if (get)
          {
             if (table)
               {
                  // If this is a table, then we pushed a value on the stack, pop it off.
                  lua_pop(L, 1);                                         // Stack usage [-n, +0, -]
               }
            else
                j++;
            count++;
          }
        p = q;
     }

   va_end(vl);
   free(f);
   if (count > n)
      n = 0;
   else if (table)
     n = 1;
   return n;
}

int push_lua(lua_State *L, char *params, ...)       // Stack usage [-0, +n, em]
{
  va_list vl;
  char *f = strdup(params);
  char *p = f;
  int n = 0, table = 0, i = -1, needTrace = 0, _T;

  if (!f) return -1;

  // Scan ahead looking for ), so we know to put the traceBack function on the stack first.
  while (*p)
  {
    p++;
    if ('0' == *p)
    {
      lua_pushcfunction(L, traceBack);
      _T = lua_gettop(L);
      needTrace = 1;
      break;
    }
  }
  p = f;

  va_start(vl, params);

  while (*p)
  {
    char *q;
    Eina_Bool set = EINA_TRUE;

    while (isspace((int)*p))
      p++;
    q = p + 1;
    switch (*p)
    {
      case '%':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushinteger(L, va_arg(vl, int));    // Stack usage [-0, +1, -]
        break;
      }
      case '#':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushnumber(L, va_arg(vl, double));  // Stack usage [-0, +1, -]
        break;
      }
      case '$':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        char *t = va_arg(vl, char *);
//printf("push_lua %s string %s\n", p, t);
        lua_pushstring(L, t);  // Stack usage [-0, +1, m]
        break;
      }
      case '!':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushboolean(L, va_arg(vl, int));    // Stack usage [-0, +1, -]
        break;
      }
      case '=':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushvalue(L, va_arg(vl, int));      // Stack usage [-0, +1, -]
        break;
      }
      case '@':
      {
        int   tabl = va_arg(vl, int);
        char *field = va_arg(vl, char *);

        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_getfield(L, tabl, field);           // Stack usage [-0, +1, e]
        break;
      }
      case '*':
      {
        if (table)  q = _push_name(L, q, &i);     // Stack usage [-0, +1, m]
        lua_pushlightuserdata(L, va_arg(vl, void *)); // Stack usage [-0, +1, m]
        break;
      }
      case '&':
      {
        if (table)  q = _push_name(L, q, &i);     // Stack usage [-0, +1, m]
        lua_pushcfunction(L, va_arg(vl, void *)); // Stack usage [-0, +1, m]
        break;
      }
      case '~':
      {
        if (table)  q = _push_name(L, q, &i);   // Stack usage [-0, +1, m]
        lua_pushnil(L);                         // Stack usage [-0, +1, -]
        break;
      }
      case '(':		// Just syntax sugar.
      {
        set = EINA_FALSE;
        break;
      }
      case ')':
      {
        int err;

        if ((err = lua_pcall(L, n - 1, va_arg(vl, int), _T)))
          printLuaError(err, params, L);
        n = 0;
        set = EINA_FALSE;
        break;
      }
      case '{':
      {
        lua_newtable(L);
        table++;
        n++;
        set = EINA_FALSE;
        break;
      }
      case '}':
      {
        table--;
        set = EINA_FALSE;
        break;
      }
      default:
      {
        set = EINA_FALSE;
        break;
      }
    }

    if (set)
    {
      if (table > 0)
        lua_settable(L, -3);                         // Stack usage [-2, +0, e]
      else
        n++;
    }
    p = q;
  }

  va_end(vl);
  if (needTrace)
    lua_remove(L, _T);
  free(f);
  return n;
}
