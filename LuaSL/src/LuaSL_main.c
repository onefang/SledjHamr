
#include "LuaSL.h"


Eina_Strbuf *clientStream;


Eina_Bool _add(void *data, int type __UNUSED__, Ecore_Con_Event_Client_Add *ev)
{
    ecore_con_client_timeout_set(ev->client, 0);
    return ECORE_CALLBACK_RENEW;
}

Eina_Bool _del(void *data, int type __UNUSED__, Ecore_Con_Event_Client_Del *ev)
{
    gameGlobals *game = data;

    if (ev->client)
    {
	PD("No more clients, exiting.");
	ecore_con_client_del(ev->client);
	ecore_main_loop_quit();
    }
    return ECORE_CALLBACK_RENEW;
}

Eina_Bool _data(void *data, int type __UNUSED__, Ecore_Con_Event_Client_Data *ev)
{
    gameGlobals *game = data;
    char SID[PATH_MAX];
    const char *command;
    char *ext;

    eina_strbuf_append_length(clientStream, ev->data, ev->size);
    command = eina_strbuf_string_get(clientStream);
    while ((ext = index(command, '\n')))
    {
	int length = ext - command;

	strncpy(SID, command, length + 1);
	SID[length] = '\0';
	eina_strbuf_remove(clientStream, 0, length + 1);
	ext = rindex(SID, '.');
	if (ext)
	{
	    ext[0] = '\0';
	    command = ext + 1;
	    if (0 == strcmp(command, "compile()"))
	    {
		if (compileLSL(game, SID, FALSE))
		    PD("The compile of %s worked.", SID);
		else
		    PE("The compile of %s failed!", SID);
	    }
	    else if (0 == strcmp(command, "start()"))
	    {
		runLuaFile(game, SID);
	    }
	    else if (0 == strcmp(command, "exit()"))
	    {
		PD("Told to exit.");
		ecore_main_loop_quit();
	    }
	    else
	    {
		char temp[PATH_MAX];
		const char *status = NULL;

		snprintf(temp, sizeof(temp), "%s.events", SID);
		status = sendToChannel(temp, command, NULL, NULL);
		if (status)
		    PE("Error sending command %s to script %s : %s", command, temp, status);
	    }
	}

	// Get the next blob to check it.
	command = eina_strbuf_string_get(clientStream);
    }

   return ECORE_CALLBACK_RENEW;
}

int main(int argc, char **argv)
{
    gameGlobals game;
    int result = EXIT_FAILURE;

    memset(&game, 0, sizeof(gameGlobals));
    game.address = "127.0.01";
    game.port = 8211;

    if (eina_init())
    {
	loggingStartup(&game);
	if (ecore_con_init())
	{
	    if ((game.server = ecore_con_server_add(ECORE_CON_REMOTE_TCP, game.address, game.port, &game)))
	    {
		ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,  (Ecore_Event_Handler_Cb) _add,  &game);
		ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,  (Ecore_Event_Handler_Cb) _del,  &game);
		ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, (Ecore_Event_Handler_Cb) _data, &game);
		ecore_con_server_timeout_set(game.server, 0);
		ecore_con_server_client_limit_set(game.server, -1, 0);
		clientStream = eina_strbuf_new();

		if (edje_init())
		{
		    result = 0;
		    compilerSetup(&game);
		    runnerSetup(&game);
		    ecore_main_loop_begin();
		    runnerTearDown(&game);
		    edje_shutdown();
		}
		else
		    PCm("Failed to init edje!");
	    }
	    else
		PCm("Failed to add server!");
	    ecore_con_shutdown();
	}
	else
	    PCm("Failed to init ecore_con!");
    }
    else
	fprintf(stderr, "Failed to init eina!");

    return result;
}
