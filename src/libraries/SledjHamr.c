// This might become nails, since at the moment it's only got comms stuff in it.

#include <unistd.h>
#include "LumbrJack.h"
#include "SledjHamr.h"


struct _message
{
  Eina_Clist	node;
  char		*message;
};


void *addMessage(Eina_Clist *list, size_t size, const char *message, ...)
{
    va_list args;
    char buf[PATH_MAX];
    int length = 0;
    struct _message *result;

    va_start(args, message);
    length += vsprintf(&buf[length], message, args);
    va_end(args);
    result = calloc(1, size + length + 1);
    eina_clist_element_init(&(result->node));
    eina_clist_add_tail(list, &(result->node));
    result->message = ((char *) result) + size;
    strcpy(result->message, buf);

    return result;
}

static boolean checkConnection(Connection *conn, char *func, connType wanted, boolean isLocal)
{
  boolean result = TRUE;

  if ((conn->type != CT_CLIENT) && (conn->type != CT_SERVER))
  {
    result = FALSE;
    PE("CONNECTION OBJECT in %s() is of unknown type %d", func, (int) conn->type);
  }
  else if (conn->type != wanted)
  {
    result = FALSE;
    switch (wanted)
    {
      case CT_CLIENT :
	if (conn->type == CT_SERVER)
          PE("INVALID CONNECTION OBJECT in %s(), it might be a server object!", func);
	else
          PE("INVALID CONNECTION OBJECT in %s(), type is %d!", func, (int) conn->type);
	if (conn->conn.client.myServer == NULL)
	  PE("CONNECTION OBJECT in %s() is a local client, but should be a remote client mirror!", func);
        break;

      case CT_SERVER :
	if (conn->type == CT_CLIENT)
	  PE("INVALID CONNECTION OBJECT in %s(), it might be a client object!", func);
	else
	  PE("INVALID CONNECTION OBJECT in %s(), type is %d!", func, (int) conn->type);
	if (isLocal)
	{
	  if (conn->conn.server.clients == NULL)
	    PE("CONNECTION OBJECT in %s() is a remote server mirror, but should be a local server!", func);
	}
	else
	{
	  if (conn->conn.server.clients != NULL)
	    PE("CONNECTION OBJECT in %s() is a local server, but should be a remote server mirror!", func);
	}
        break;

      default :
	PE("CONNECTION OBJECT in %s(), silly coder asked for an unknown type!", func);
	break;
    }

    if (NULL == conn->name)
    {
      result = FALSE;
      PE("CONNECTION OBJECT in %s() has no name!", func);
    }
  }

//if (result)  PD("%s(\"%s\")", func, conn->name);

  return result;
}

void send2(Connection *conn, const char *SID, const char *message, ...)
{
    va_list args;
    char buf[PATH_MAX * 2];
    int length = strlen(SID);

    strncpy(buf, SID, length);
    buf[length++] = '.';
    va_start(args, message);
    length += vsprintf(&buf[length], message, args);
    va_end(args);
    buf[length++] = '\n';
    buf[length] = '\0';

// TODO - Should check if this is always gonna be local?  Likely not.
    if (checkConnection(conn, "send2", conn->type, FALSE))
    {
	switch (conn->type)
	{
	    case CT_CLIENT :
//		PD("vvv send2(%*s", length, buf);
		ecore_con_client_send(conn->conn.client.client, strndup(buf, length), length);
		ecore_con_client_flush(conn->conn.client.client);
		break;

	    case CT_SERVER :
//		PD("^^^ send2(%*s", length, buf);
		ecore_con_server_send(conn->conn.server.server, strndup(buf, length), length);
		ecore_con_server_flush(conn->conn.server.server);
		break;

	    default :
		PE("send2() unable to send to partially bogus Connection object!");
		break;
	}
    }
    else
      PE("send2() unable to send to bogus Connection object!");
}

static Eina_Bool parseStream(void *data, int type, void *evData, int evSize, void *ev)
{
    Connection *conn = data;
    char SID[PATH_MAX];
    const char *command;
    char *ext;

    if (NULL == conn->stream)
      conn->stream = eina_strbuf_new();

    eina_strbuf_append_length(conn->stream, evData, evSize);
    command = eina_strbuf_string_get(conn->stream);
    while ((ext = index(command, '\n')))
    {
	int length = ext - command;

	strncpy(SID, command, length + 1);
	SID[length] = '\0';
	eina_strbuf_remove(conn->stream, 0, length + 1);
	ext = index(SID, '.');
	if (ext)
	{
	    ext[0] = '\0';
	    command = ext + 1;
	    ext = index(command, '(');
	    if (NULL == ext)
	      ext = index(command, ' ');
	    if (ext)
	    {
		streamParser func = eina_hash_find(conn->commands, command);

		ext[0] = '\0';
		// Need a callback if we can't find the command.
		if (NULL == func)
		    func = conn->unknownCommand;
		if (func)
		    func(conn->pointer, conn, SID, (char *) command, ext + 1);
		else
		    PE("parseStream() No function found for command %s!", command);
            }
	}

	// Get the next blob to check it.
	command = eina_strbuf_string_get(conn->stream);
    }

    if (conn->_data)
      conn->_data(conn->pointer, type, ev);

    return ECORE_CALLBACK_RENEW;
}

static Eina_Bool parseClientStream(void *data, int type, Ecore_Con_Event_Client_Data *ev)
{
  Connection *conn = /*data*/  ecore_con_client_data_get(ev->client);

  if (checkConnection(conn, "parseClientStream", CT_CLIENT, FALSE))
    return parseStream(conn, type, ev->data, ev->size, ev);
  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool parseServerStream(void *data, int type, Ecore_Con_Event_Server_Data *ev)
{
  Connection *conn = /*data*/  ecore_con_server_data_get(ev->server);

  if (checkConnection(conn, "parseServerStream", CT_SERVER, FALSE))
    return parseStream(conn, type, ev->data, ev->size, ev);

  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool clientAdd(void *data, int type, Ecore_Con_Event_Client_Add *ev)
{
  Connection *conn, *connection = data;

  if (checkConnection(connection, "clientAdd", CT_SERVER, TRUE))
  {
    ecore_con_client_timeout_set(ev->client, 0);
    conn = calloc(1, sizeof(Connection));
    conn->type = CT_CLIENT;
    conn->conn.client.client = ev->client;
    conn->conn.client.myServer = connection;
    conn->conn.client.server = malloc(sizeof(Eina_Clist));
    eina_clist_element_init(conn->conn.client.server);
    eina_clist_add_tail(connection->conn.server.clients, conn->conn.client.server);
    conn->name = strdup(connection->name);
    conn->address = strdup(connection->address);
    conn->port = connection->port;
    conn->pointer = connection->pointer;
    conn->_add = connection->_add;
    conn->_data = connection->_data;
    conn->_del = connection->_del;
    conn->unknownCommand = connection->unknownCommand;
    conn->commands = eina_hash_string_superfast_new(NULL);
    ecore_con_client_data_set(ev->client, conn);

    if (connection->_add)
      return connection->_add(connection->pointer, type, ev);

  }

  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool clientDel(void *data, int type, Ecore_Con_Event_Client_Del *ev)
{
    Connection *conn = data;

  if (checkConnection(conn, "clientDel", CT_SERVER, TRUE))
  {
    if (ev->client)
    {
	Eina_List const *clients;

	if (conn->_del)
	    conn->_del(conn->pointer, type, ev);

	ecore_con_client_del(ev->client);


	// This is only really for testing, normally it just runs 24/7, or until told not to.
	// The "- 1" is coz this server is still counted.
	clients = ecore_con_server_clients_get(conn->conn.server.server) - 1;
        if (0 == eina_list_count(clients))
	    PI("No more clients for %s, exiting.", conn->name);
	else
	    PW("Some (%d) more clients for %s, exiting anyway.", eina_list_count(clients), conn->name);

	// TODO - the Connection free function should take care of all of this, and we should call it here.  ish.
	eina_clist_remove(conn->conn.client.server);
	free(conn->conn.client.server);
	conn->conn.client.server = NULL;

// TODO - Probably should just keep running, both servers, and go away when all clients are gone for testing.
	ecore_main_loop_quit();
    }
  }

  free(conn);
  return ECORE_CALLBACK_RENEW;
}

Connection *openArms(char *name, const char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del, streamParser _parser)
{
    Connection *conn = calloc(1, sizeof(Connection));
    Ecore_Con_Server *server;

    conn->type = CT_SERVER;
    conn->conn.server.clients = malloc(sizeof(Eina_Clist));
    eina_clist_init(conn->conn.server.clients);
    conn->name = strdup(name);
    conn->address = strdup(address);
    conn->port = port;
    conn->pointer = data;
    conn->_add = _add;
    conn->_data = _data;
    conn->_del = _del;
    conn->unknownCommand = _parser;
    conn->commands = eina_hash_string_superfast_new(NULL);

    conn->add  = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,  (Ecore_Event_Handler_Cb) clientAdd,   conn);
    conn->data = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb) parseClientStream, conn);
    conn->del  = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,  (Ecore_Event_Handler_Cb) clientDel,   conn);
//  conn->died = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _serverDied, conn);

    if ((server = ecore_con_server_add(ECORE_CON_REMOTE_TCP, address, port, conn)))
    {
      conn->conn.server.server = server;
      ecore_con_server_timeout_set(server, 0);
      ecore_con_server_client_limit_set(server, -1, 0);
//      ecore_con_server_timeout_set(server, 10);
//      ecore_con_server_client_limit_set(server, 3, 0);
        PI("ACTUALLY created the %s server %s:%d.", name, address, port);
    }
    else
    {
      // TODO - Connection needs a generic free function.  Only reason we are getting away with this is during initial testing, the Connections last the entire run anyway.
      free(conn->address);
      free(conn);
      conn = NULL;
    }

    return conn;
}

static Eina_Bool serverAdd(void *data, int type, Ecore_Con_Event_Server_Add *ev)
{
  Connection *conn = data;

  if (checkConnection(conn, "serverAdd", CT_SERVER, FALSE))
  {
    conn->conn.server.hackyCount++;
    conn->stage++;

    if (conn->name)
      PI("serverAdd()^^^^^^^^^^^^^^^^^^^^^^^Connected to %s server.", conn->name);
    else
      PW("serverAdd()^^^^^^^^^^^^^^^^^^^^^^^Connected to UNKNOWN server.");

    // In case the server crashed, clear out any waiting data.
    if (conn->stream)
      eina_strbuf_reset(conn->stream);
    if (conn->_add)
      conn->_add(conn->pointer, type, ev);
  }

  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool serverDel(void *data, int type, Ecore_Con_Event_Server_Del *ev)
{
  Eina_Bool result = ECORE_CALLBACK_RENEW;
  Connection *conn = data;

  if (checkConnection(conn, "serverDel", CT_SERVER, FALSE))
  {
    conn->conn.server.server = NULL;
    conn->stage = -3;
    if (conn->_del)
      result = conn->_del(conn->pointer, type, ev);
  }

  return result;
}

static Eina_Bool _serverDied(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
  Connection *conn = data;
//  Ecore_Exe_Event_Data *dataFromProcess = (Ecore_Exe_Event_Data *)event;

  conn->stage = -3;
  conn->conn.server.serverHandle = NULL;
  conn->conn.server.pid = 0;

  return ECORE_CALLBACK_DONE;
}

// TODO - instead of a timer, try to make this event driven.  Use jobs, or idler, or something.
static Eina_Bool _reachOutTimer(void *data)
{
  Eina_Bool result = ECORE_CALLBACK_RENEW;
  Connection *conn = data;
  Ecore_Con_Server *server = NULL;

  switch (conn->stage)
  {
    // TODO - Seems Ecore_con now has trouble with my try first, then start method, so start first, then try.  Fix this, or do something else.
    case -3 :
      PW("Failed to connect to a %s server, starting our own.", conn->name);
      conn->conn.server.serverHandle = ecore_exe_pipe_run(conn->conn.server.serverCommand, ECORE_EXE_NONE /*| ECORE_EXE_TERM_WITH_PARENT*/, conn);
      if (conn->conn.server.serverHandle)
      {
        conn->conn.server.pid = ecore_exe_pid_get(conn->conn.server.serverHandle);
        if (conn->conn.server.pid == -1)
          PE("Could not retrive the PID!");
      }
      else
        PE("Could not create server process %s!", conn->conn.server.serverCommand);

      // TODO - There's also the question of what to do if the connection failed.
      //        Did the server crash, or was it just the connection?
      //          Also, I'm assuming the connection failed here, rather than went away after running for some time.  Should differentiate.
      //        Probably gonna need some smarts, for now we just restart all the scripts.
      //        Which I think gets done on server add.
      break;

// TODO - Alternate strategy : Keep track of if we started a server, then keep pinging it's port until it answers,
//          with a timeout until we kill the server and start again.

    case -2 :  // Give the server some time to start up.
    case -1 :  // Give the server some time to start up.
      // Check if the server is still running here, if not, reset stage to previous -3 (taking into account the increment at the end).
      if (conn->conn.server.pid)
        PI("Waiting for %s server to start from command \"%s\"", conn->name, conn->conn.server.serverCommand);
      else
        conn->stage = -4;
      break;

    case 0 :
      PI("Attempting to connect to the %s server %s:%d.", conn->name, conn->address, conn->port);
      // This should only return NULL if something goes wrong with the setup, 
      // you wont know if the connection worked until you get the add callback,
      // or you get the del calback if it failed.
      if ((server = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, conn->address, conn->port, conn)))
        PD("MAYBE connecting to the %s server %s:%d.", conn->name, conn->address, conn->port);
      else
        PE("FAILED to create the connection to the %s server %s:%d!", conn->name, conn->address, conn->port);
      conn->conn.server.server = server;
      break;

    case 1 :  // This stage is the serverAdd callback.
      break;

    case 2 :  // Give the server a chance to die.
      break;

    default :
	if (5 < conn->stage)
	  conn->stage = 2;  // loop back to nothing.
//      result = ECORE_CALLBACK_CANCEL;
      break;
  }
  conn->stage++;

  return result;
}

Connection *reachOut(char *name, char *command, char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del, streamParser _parser)
{
  Connection *conn = calloc(1, sizeof(Connection));

  conn->type = CT_SERVER;
  conn->conn.server.serverCommand = strdup(command);
  // Sure, this is essentially a NOP, but lets be explicit here, this is a remote server, so no list of clients, not just an empty list.
  conn->conn.server.clients = NULL;
  conn->name = strdup(name);
  conn->address = strdup(address);
  conn->port = port;
  conn->pointer = data;
  conn->_add = _add;
  conn->_data = _data;
  conn->_del = _del;
  conn->unknownCommand = _parser;
  conn->commands = eina_hash_string_superfast_new(NULL);

  conn->add  = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,  (Ecore_Event_Handler_Cb) serverAdd,   conn);
  conn->data = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, (Ecore_Event_Handler_Cb) parseServerStream, conn);
  conn->del  = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,  (Ecore_Event_Handler_Cb) serverDel,   conn);
  conn->died = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _serverDied, conn);

  ecore_timer_add(1.0, _reachOutTimer, conn);

  return conn;
}
