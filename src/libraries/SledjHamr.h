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
};

struct _ConnClient
{
  Ecore_Con_Client *client;
  Connection *myServer;
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
  Ecore_Event_Handler *add, *data, *del;
  streamParser unknownCommand;
};


Ecore_Con_Server *reachOut(char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del);
void *addMessage(Eina_Clist *list, size_t size, const char *message, ...);
void sendBack(Ecore_Con_Client *client, const char *SID, const char *message, ...);
void sendForth(Ecore_Con_Server *server, const char *SID, const char *message, ...);

Connection *openArms(char *name, const char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del, streamParser _parser);

#endif
