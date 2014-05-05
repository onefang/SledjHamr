#ifndef _LUMBRJACK_H_
#define _LUMBRJACK_H_


#include <stdio.h>
#include <ctype.h>

#include <Eina.h>


#define PC(...) EINA_LOG_DOM_CRIT(logDom, __VA_ARGS__)
#define PE(...) EINA_LOG_DOM_ERR(logDom, __VA_ARGS__)
#define PW(...) EINA_LOG_DOM_WARN(logDom, __VA_ARGS__)
#define PD(...) EINA_LOG_DOM_DBG(logDom, __VA_ARGS__)
#define PI(...) EINA_LOG_DOM_INFO(logDom, __VA_ARGS__)

//#define D()	PD("DEBUG")


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

#endif
