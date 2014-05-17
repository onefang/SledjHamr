#ifndef _SLEDJHAMR_H_
#define _SLEDJHAMR_H_


#define EFL_API_OVERRIDE 1
/* Enable access to unstable EFL API that are still in beta */
#define EFL_BETA_API_SUPPORT 1
/* Enable access to unstable EFL EO API. */
#define EFL_EO_API_SUPPORT 1

#include <stdlib.h>

#include <Ecore.h>
#include <Ecore_Con.h>


#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(*array))

Ecore_Con_Server *reachOut(char *address, int port, void *data, Ecore_Event_Handler_Cb _add, Ecore_Event_Handler_Cb _data, Ecore_Event_Handler_Cb _del);
void sendBack(Ecore_Con_Client *client, const char *SID, const char *message, ...);
void sendForth(Ecore_Con_Server *server, const char *SID, const char *message, ...);

#endif
