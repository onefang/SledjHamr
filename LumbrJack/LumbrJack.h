
#include <stdio.h>
#include <ctype.h>

#include <Eina.h>


#define PC(...) EINA_LOG_DOM_CRIT(ourGlobals->logDom, __VA_ARGS__)
#define PE(...) EINA_LOG_DOM_ERR(ourGlobals->logDom, __VA_ARGS__)
#define PW(...) EINA_LOG_DOM_WARN(ourGlobals->logDom, __VA_ARGS__)
#define PD(...) EINA_LOG_DOM_DBG(ourGlobals->logDom, __VA_ARGS__)
#define PI(...) EINA_LOG_DOM_INFO(ourGlobals->logDom, __VA_ARGS__)

#define PCm(...) EINA_LOG_DOM_CRIT(ourGlobals.logDom, __VA_ARGS__)
#define PEm(...) EINA_LOG_DOM_ERR(ourGlobals.logDom, __VA_ARGS__)
#define PWm(...) EINA_LOG_DOM_WARN(ourGlobals.logDom, __VA_ARGS__)
#define PDm(...) EINA_LOG_DOM_DBG(ourGlobals.logDom, __VA_ARGS__)
#define PIm(...) EINA_LOG_DOM_INFO(ourGlobals.logDom, __VA_ARGS__)

#define D()	PD("DEBUG")


// "01:03:52 01-01-1973\n\0"
#define DATE_TIME_LEN  21


#ifndef FALSE
// NEVER change this
typedef enum
{
  FALSE	= 0,
  TRUE	= 1
} boolean;
#endif


int loggingStartup(char *name, int logDom);
char *getDateTime(struct tm **nowOut, char *dateOut, time_t *tiemOut);
