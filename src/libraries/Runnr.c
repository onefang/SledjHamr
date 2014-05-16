/*  Runnr - a library that deals with running Lua scripts.

*/


#include "Runnr.h"


void dumpStack(lua_State *L, int i)
{
  int type = lua_type(L, i);

  switch (type)
  {
    case LUA_TNONE		:  printf("Stack %d is empty\n", i);  break;
    case LUA_TNIL		:  printf("Stack %d is a nil\n", i);  break;
    case LUA_TBOOLEAN		:  printf("Stack %d is a boolean - %d\n", i, lua_toboolean(L, i));  break;
    case LUA_TNUMBER		:  printf("Stack %d is a number\n - %f", i, lua_tonumber(L, i));  break;
    case LUA_TSTRING		:  printf("Stack %d is a string - %s\n", i, lua_tostring(L, i));  break;
    case LUA_TFUNCTION		:  printf("Stack %d is a function\n", i);  break;
    case LUA_TTHREAD		:  printf("Stack %d is a thread\n", i);  break;
    case LUA_TTABLE		:
    {
      int j;

      printf("Stack %d is a table", i);
      lua_getfield(L, i, "_NAME");
      j = lua_gettop(L);
      if (lua_isstring(L, j))
        printf(" - %s", lua_tostring(L, j));
      lua_pop(L, 1);
      printf("\n");
      break;
    }
    case LUA_TUSERDATA		:  printf("Stack %d is a userdata\n", i);  break;
    case LUA_TLIGHTUSERDATA	:  printf("Stack %d is a light userdata\n", i);  break;
    default			:  printf("Stack %d is unknown\n", i);  break;
  }
}

static int traceBack(lua_State *L)
{
  const char *msg = "";
  int top = lua_gettop(L);
//  int i;

//  printf("Stack is %d deep\n", top);
//  for (i = 1; i <= top; i++)
//    dumpStack(L, i);

  if (top)
    msg = lua_tostring(L, 1);
  lua_getglobal(L, "debug");
  push_lua(L, "@ ( )", lua_gettop(L), "traceback", 1);
  lua_pushstring(L, "\n");
  lua_pushstring(L, msg);
  lua_concat(L, 3);

  return 1;
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

    if ((err = lua_pcall(L, 0, LUA_MULTRET, _T)))
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
      printf("Error running - %s, \n%s - %s", string, err_type, lua_tostring(L, -1));
    }
  }
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
                       size_t len;
                       char *temp = (char *) lua_tolstring(L, j, &len);  // Stack usage [-0, +0, m]

                       len++;  // Cater for the null at the end.
                       *v = malloc(len);
                       if (*v)
                         {
                            memcpy(*v, temp, len);
                            n++;
                         }
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
  int n = 0, table = 0, i = -1;

  if (!f) return -1;

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
        lua_call(L, n - 1, va_arg(vl, int));
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
  free(f);
  return n;
}

void sendBack(Ecore_Con_Client *client, const char *SID, const char *message, ...)
{
    va_list args;
    char buf[PATH_MAX];
    int length = strlen(SID);

    strncpy(buf, SID, length);
    buf[length++] = '.';
    va_start(args, message);
    length += vsprintf(&buf[length], message, args);
    va_end(args);
    buf[length++] = '\n';
    buf[length++] = '\0';
    ecore_con_client_send(client, buf, strlen(buf));
    ecore_con_client_flush(client);
}

void sendForth(Ecore_Con_Server	*server, const char *SID, const char *message, ...)
{
    va_list args;
    char buf[PATH_MAX];
    int length = strlen(SID);

    strncpy(buf, SID, length);
    buf[length++] = '.';
    va_start(args, message);
    length += vsprintf(&buf[length], message, args);
    va_end(args);
    buf[length++] = '\n';
    buf[length++] = '\0';
    ecore_con_server_send(server, buf, strlen(buf));
    ecore_con_server_flush(server);
}
