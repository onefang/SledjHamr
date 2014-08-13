// This might become nails, since at the moment it's only got comms stuff in it.

#include <unistd.h>
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
