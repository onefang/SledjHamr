#include "SledjHamr.h"

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
