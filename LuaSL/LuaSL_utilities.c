#include "LuaSL.h"


void sendBack(gameGlobals *ourGlobals, Ecore_Con_Client *client, const char *SID, const char *message, ...)
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

void sendForth(gameGlobals *ourGlobals, const char *SID, const char *message, ...)
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
    ecore_con_server_send(ourGlobals->server, buf, strlen(buf));
    ecore_con_server_flush(ourGlobals->server);
}

float timeDiff(struct timeval *now, struct timeval *then)
{
    if (0 == gettimeofday(now, 0))
    {
	struct timeval thisTime = { 0, 0 };
	double  result = 0.0;

	thisTime.tv_sec = now->tv_sec;
	thisTime.tv_usec = now->tv_usec;
	if (thisTime.tv_usec < then->tv_usec)
	{
	    thisTime.tv_sec--;
	    thisTime.tv_usec += 1000000;
	}
	thisTime.tv_usec -= then->tv_usec;
	thisTime.tv_sec -= then->tv_sec;
	result = ((double) thisTime.tv_usec) / ((double) 1000000.0);
	result += thisTime.tv_sec;
	return result;
    }
    else
	return 0.0;
}
