// This might become nails, since at the moment it's only got comms stuff in it.

#include <unistd.h>
#include "LumbrJack.h"
#include "SledjHamr.h"


struct _conct
{
  char *address;
  int port;
  void *pointer;
  Ecore_Event_Handler_Cb addCb, dataCb, delCb;
  Ecore_Event_Handler *add, *data, *del;
};

struct _message
{
  Eina_Clist	node;
  char		*message;
};


static Eina_Bool _add(void *data, int type, Ecore_Con_Event_Server_Del *ev)
{
  struct _conct *this = data;

  if (this->addCb)
    this->addCb(this->pointer, type, ev);
  if (this->dataCb)
    ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, this->dataCb, this->pointer);

  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _delTimer(void *data)
{
  struct _conct *this = data;

  reachOut(this->address, this->port, this->pointer, this->addCb, this->dataCb, this->delCb);
  return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _del(void *data, int type, Ecore_Con_Event_Server_Del *ev)
{
  struct _conct *this = data;

  printf("FAILED connection to server %s:%d, trying again in a second!\n", this->address, this->port);
  ecore_event_handler_del(this->add);
  ecore_event_handler_del(this->del);

  if (this->delCb)
  {
    if (ECORE_CALLBACK_RENEW == this->delCb(this->pointer, type, ev))
      ecore_timer_add(1.0, _delTimer, this);
  }

  if (ev->server)  ecore_con_server_del(ev->server);
  // TODO - Hmm, I think this is where this should be freed, but it causes a seggie in reachOut's while loop.
  //        Which is odd, so leave it commented for now and investigate later.
//  free(this);

  return ECORE_CALLBACK_CANCEL;
}

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
    buf[length] = '\0';
//    printf("sendBack(%s)", buf);
    ecore_con_client_send(client, buf, length);
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
    buf[length] = '\0';
//    printf("sendForth(%s)", buf);
    ecore_con_server_send(server, buf, length);
    ecore_con_server_flush(server);
}




static Eina_Bool parseStream(void *data, int type, void *evData, int evSize, void *ev)
{
    Connection *connection = data;
    char SID[PATH_MAX];
    const char *command;
    char *ext;

    if (NULL == connection->stream)
      connection->stream = eina_strbuf_new();

    eina_strbuf_append_length(connection->stream, evData, evSize);
    command = eina_strbuf_string_get(connection->stream);
    while ((ext = index(command, '\n')))
    {
	int length = ext - command;

	strncpy(SID, command, length + 1);
	SID[length] = '\0';
	eina_strbuf_remove(connection->stream, 0, length + 1);
	ext = index(SID, '.');
	if (ext)
	{
	    ext[0] = '\0';
	    command = ext + 1;
	    ext = index(SID, '(');
	    if (ext)
	    {
		streamParser func = eina_hash_find(connection->commands, command);
//PW("COMMAND - %s", command);

		ext[0] = '\0';

		// Need a callback if we can't find the command.
		if (NULL == func)
		    func = connection->unknownCommand;
		if (func)
		    func(data, connection, SID, (char *) command, ext + 1);
            }
	}

	// Get the next blob to check it.
	command = eina_strbuf_string_get(connection->stream);
    }

    if (connection->_data)
      connection->_data(data, type, ev);

    return ECORE_CALLBACK_RENEW;
}

Eina_Bool parseClientStream(void *data, int type, Ecore_Con_Event_Client_Data *ev)
{
  // data is the server connection, but we want the client one.
  return parseStream(ecore_con_client_data_get(ev->client), type, ev->data, ev->size, ev);
}

Eina_Bool parseServerStream(void *data, int type, Ecore_Con_Event_Server_Data *ev)
{
    return parseStream(data, type, ev->data, ev->size, ev);
}


Eina_Bool clientAdd(void *data, int type, Ecore_Con_Event_Client_Add *ev)
{
    Connection *conn, *connection = data;

    ecore_con_client_timeout_set(ev->client, 0);
    conn = calloc(1, sizeof(Connection));
    conn->type = CT_CLIENT;
    conn->conn.client.client = ev->client;
    conn->conn.client.myServer = connection;
    conn->name = strdup(connection->name);
    conn->address = strdup(connection->address);
    conn->port = connection->port;
    conn->pointer = connection->pointer;
    conn->commands = eina_hash_string_superfast_new(NULL);
    ecore_con_client_data_set(ev->client, conn);

    if (connection->_add)
      return connection->_add(connection->pointer, type, ev);

    return ECORE_CALLBACK_RENEW;
}

Eina_Bool clientDel(void *data, int type, Ecore_Con_Event_Client_Del *ev)
{
    Connection *connection = data;

    if (ev->client)
    {
	Eina_List const *clients;

	// This is only really for testing, normally it just runs 24/7, or until told not to.
	clients = ecore_con_server_clients_get(connection->conn.server.server);
        if (0 == eina_list_count(clients))
	    printf("No more clients, exiting.");
	else
	    printf("Some %d more clients, exiting anyway.", eina_list_count(clients));

// TODO - Probably should just keep running, both servers, and go away when all clients are gone for testing.

	if (connection->_del)
	    connection->_del(connection->pointer, type, ev);

	ecore_con_client_del(ev->client);
	ecore_main_loop_quit();
    }

    return ECORE_CALLBACK_RENEW;
}

Connection *openArms(char *name, const char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del, streamParser _parser)
{
    Connection *conn = calloc(1, sizeof(Connection));
    Ecore_Con_Server *server;

    conn->type = CT_SERVER;
//    conn->conn.server.serverCommand = ;
    conn->name = strdup(name);
    conn->address = strdup(address);
    conn->port = port;

    conn->pointer = data;
    conn->_add = _add;
    conn->_data = _data;
    conn->_del = _del;
    conn->unknownCommand = _parser;
    conn->commands = eina_hash_string_superfast_new(NULL);

    if ((server = ecore_con_server_add(ECORE_CON_REMOTE_TCP, address, port, data)))
    {
      conn->conn.server.server = server;

	conn->add  = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,  (Ecore_Event_Handler_Cb) clientAdd,   conn);
	conn->add  = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb) parseClientStream, conn);
	conn->add  = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,  (Ecore_Event_Handler_Cb) clientDel,   conn);
	ecore_con_server_timeout_set(server, 0);
	ecore_con_server_client_limit_set(server, -1, 0);
	ecore_con_server_data_set(server, conn);
//	ecore_con_server_timeout_set(server, 10);
//	ecore_con_server_client_limit_set(server, 3, 0);
    }
    else
    {
      free(conn->address);
      free(conn);
      conn = NULL;
    }

    return conn;
}

Eina_Bool serverAdd(void *data, int type, Ecore_Con_Event_Server_Add *ev)
{
  Connection *connection = data;

  connection->conn.server.hackyCount++;

  // Alledgedly this checks for real conections, but given that the first time it's called, there's no real connection, this is failing somehow.
  //if (ecore_con_server_connected_get(ev->server))
  if (connection->conn.server.hackyCount <= 1)
    printf("Bogus server ignored.");
  else
  {
    printf("Connected to %s server.", connection->name);
    connection->conn.server.server = ev->server;
    // In case the server crashed, clear out any waiting data.
    if (connection->stream)
      eina_strbuf_reset(connection->stream);

    if (connection->_add)
      connection->_add(data, type, ev);
  }

  return ECORE_CALLBACK_RENEW;
}

Eina_Bool serverDel(void *data, int type, Ecore_Con_Event_Server_Del *ev)
{
  Connection *connection = data;

  connection->conn.server.server = NULL;

//  connection->server.hackyCount--;
  // Let it fail a couple of times during startup, then try to start our own script server.
  connection->conn.server.count++;
  if (1 < connection->conn.server.count)
  {
    char buf[PATH_MAX];

    printf("Failed to connect to a %s server, starting our own.", connection->name);
    // TODO - Should use Ecore_Exe for this sort of thing.
    sprintf(buf, "%s/%s &", prefix_bin_get(), connection->conn.server.serverCommand);
    system(buf);
    connection->conn.server.count = 0;
    // TODO - There's also the question of what to do if the connection failed.
    //        Did the server crash, or was it just the connection?
    //        Probably gonna need some smarts, for now we just restart all the scripts.
    //        Which I think gets done on server add.
  }

  if (connection->_del)
    connection->_del(data, type, ev);


  // TODO - May want to renew even if it's not running the GUI, but then we still need some sort of "shut down" signal, which we don't need during testing.
//  if (ourGlobals->ui)
    return ECORE_CALLBACK_RENEW;

//  ecore_main_loop_quit();

//  return ECORE_CALLBACK_CANCEL;



/* Extantz does this instead of the above returns -

  if (ourGlobals->running)
    return ECORE_CALLBACK_RENEW;
  return ECORE_CALLBACK_CANCEL;
*/
}



#if 1
Ecore_Con_Server *reachOut(char *address, int port, void *data, Ecore_Event_Handler_Cb _addCb, Ecore_Event_Handler_Cb _dataCb, Ecore_Event_Handler_Cb _delCb)
{
  Ecore_Con_Server *server = NULL;
  struct _conct *this = malloc(sizeof(struct _conct));
  int count = 0;

  this->address = address;
  this->port = port;
  this->pointer = data;
  this->addCb = _addCb;
  this->dataCb = _dataCb;
  this->delCb = _delCb;
  // This loop is overkill I think.
  while ((!server) && (10 > count))
  {
    if ((server = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, address, port, this->pointer)))
    {
      this->add = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, (Ecore_Event_Handler_Cb) _add, this);
      this->del = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, (Ecore_Event_Handler_Cb) _del, this);
    }
    count++;
  }

  if (!server)
    printf("Failed to connect to server %s:%d!\n", this->address, this->port);

  return server;
}
#else
Connection *reachOut(char *name, char *command, char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del)
{
  Connection *conn = calloc(1, sizeof(Connection));
  Ecore_Con_Server *server = NULL;
  int count = 0;

  conn->type = CT_SERVER;
  conn->conn.server.serverCommand = strdup(command);
  conn->name = strdup(name);
  conn->address = strdup(address);
  conn->port = port;
  conn->pointer = data;
  conn-> = _add;
  conn-> = _data;
  conn-> = _del;
  conn->commands = eina_hash_string_superfast_new(NULL);

  // This loop is overkill I think.
  while ((!server) && (10 > count))
  {
    if ((server = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, address, port, data)))
    {
      conn->add  = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,  (Ecore_Event_Handler_Cb) serverAdd,   conn);
      conn->data = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, (Ecore_Event_Handler_Cb) parseServerStream, conn);
      conn->del  = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,  (Ecore_Event_Handler_Cb) serverDel,   conn);
      ecore_con_server_data_set(server, conn);
    }
    count++;
  }

  if (!server)
    printf("Failed to connect to the %s server %s:%d!\n", name, address, port);

  return conn;
}
#endif
