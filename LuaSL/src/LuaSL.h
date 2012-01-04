//#include <Elementary.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define __UNUSED__
#endif

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>
#include <stdio.h>
#include <ctype.h>
       
#define WIDTH  (1024)
#define HEIGHT (768)

#define PC(...) EINA_LOG_DOM_CRIT(game->logDom, __VA_ARGS__)
#define PE(...) EINA_LOG_DOM_ERR(game->logDom, __VA_ARGS__)
#define PW(...) EINA_LOG_DOM_WARN(game->logDom, __VA_ARGS__)
#define PD(...) EINA_LOG_DOM_DBG(game->logDom, __VA_ARGS__)
#define PI(...) EINA_LOG_DOM_INFO(game->logDom, __VA_ARGS__)

#define PCm(...) EINA_LOG_DOM_CRIT(game.logDom, __VA_ARGS__)
#define PEm(...) EINA_LOG_DOM_ERR(game.logDom, __VA_ARGS__)
#define PWm(...) EINA_LOG_DOM_WARN(game.logDom, __VA_ARGS__)
#define PDm(...) EINA_LOG_DOM_DBG(game.logDom, __VA_ARGS__)
#define PIm(...) EINA_LOG_DOM_INFO(game.logDom, __VA_ARGS__)

#define D()	PD("DEBUG")

// "01:03:52 01-01-1973\n\0"
#define DATE_TIME_LEN			21

#define TABLE_WIDTH	7
#define TABLE_HEIGHT	42

#ifndef FALSE
// NEVER change this
typedef enum
{
    FALSE	= 0, 
    TRUE	= 1
} boolean;
#endif

typedef struct
{
    Ecore_Evas  *ee;		// Our window.
    Evas        *canvas;	// The canvas for drawing directly onto.
    Evas_Object *bg;		// Our background edje, also the game specific stuff.
    Evas_Object *edje;		// The edje of the background.
    int logDom;
} gameGlobals;

typedef void (*doSomething) (gameGlobals *game, unsigned char key);

void loggingStartup(gameGlobals *game);
char *getDateTime(struct tm **nowOut, char *dateOut, time_t *tiemOut);
float timeDiff(struct timeval *now, struct timeval *then);


