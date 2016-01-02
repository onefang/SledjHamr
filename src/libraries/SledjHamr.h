#ifndef _SLEDJHAMR_H_
#define _SLEDJHAMR_H_


#include <stdlib.h>

#include <Ecore.h>
#include <Ecore_Con.h>


typedef struct _Connection	Connection;

typedef Eina_Bool (* streamParser)(void *data, Connection *connection, char *SID, char *command, char *arguments);

typedef enum
{
  CT_CLIENT,
  CT_SERVER
} connType;

struct _ConnServer
{
  Ecore_Con_Server *server;
  char *serverCommand;
  int count, hackyCount;
  // A list of connected remote clients.
  //   A NULL list means this is a remote server stored in a local clients Connection.
  //   An empty list means this is a local server, with no clients.
  //   An actual list means this is a local server, with connected remote clients.
  Eina_Clist	*clients;	// HEAD element.
  Ecore_Exe	*serverHandle;	// For running the server.
  pid_t		pid;
};

struct _ConnClient
{
  Ecore_Con_Client *client;
  // If this is a local  client, then myServer is a server Connection representing the remote server, and the server list entry element can be NULL.
  // If this is a remote client, then myServer is NULL, and this Connection is stored in a list in the local server's Connection.
  Connection *myServer;
  Eina_Clist	*server;	// Entry element.
};

struct _Connection
{
  connType type;
  union
  {
    struct _ConnServer		server;
    struct _ConnClient		client;
  } conn;
  char		*name; // For log entries and such.
  char		*address;
  int		port;
  Eina_Strbuf	*stream;
  Eina_Hash	*commands;
//		    streamParser *func;
  //  Callbacks.
  void *pointer;
  Ecore_Event_Handler_Cb _add, _data, _del;
  Ecore_Event_Handler *add, *data, *del, *died;
  streamParser unknownCommand;

  int		stage;	// Stage of creation for the Connection.
};


void *addMessage(Eina_Clist *list, size_t size, const char *message, ...);
void sendBack(Connection *conn, const char *SID, const char *message, ...);
void sendForth(Connection *conn, const char *SID, const char *message, ...);

void send2(Connection *conn, const char *SID, const char *message, ...);
Connection *openArms(char *name, const char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del, streamParser _parser);
Connection *reachOut(char *name, char *command, char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del, streamParser _parser);

#endif
