#ifndef _SLEDJHAMR_H_
#define _SLEDJHAMR_H_


#include <stdlib.h>

#include <Ecore.h>
#include <Ecore_Con.h>


Ecore_Con_Server *reachOut(char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del);
void sendBack(Ecore_Con_Client *client, const char *SID, const char *message, ...);
void sendForth(Ecore_Con_Server *server, const char *SID, const char *message, ...);

#endif
