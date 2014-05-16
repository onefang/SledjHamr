/*  LumbrJack - a logging library that wraps Eina logging.

*/


#include "LumbrJack.h"


static char dateTime[DATE_TIME_LEN];

static void _ggg_log_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args)
{
  FILE *f = data;
  char dt[DATE_TIME_LEN + 1];
  char fileTab[256], funcTab[256];

  getDateTime(NULL, dt, NULL);
  dt[19] = '\0';
  if (12 > strlen(file))
    snprintf(fileTab, sizeof(fileTab), "%s\t\t", file);
  else
    snprintf(fileTab, sizeof(fileTab), "%s\t", file);
  snprintf(funcTab, sizeof(funcTab), "\t%s", fnc);
  fprintf(f, "%s ", dt);
  if (f == stderr)
    eina_log_print_cb_stderr(d, level, fileTab, funcTab, line, fmt, data, args);
  else if (f == stdout)
    eina_log_print_cb_stdout(d, level, fileTab, funcTab, line, fmt, data, args);
  fflush(f);
}

int loggingStartup(char *name, int logDom)
{
  if (logDom < 0)
  {
    logDom = eina_log_domain_register(name, NULL);
    if (logDom < 0)
    {
      EINA_LOG_CRIT("could not register log domain '%s'", name);
      return logDom;
    }
  }
  eina_log_level_set(EINA_LOG_LEVEL_DBG);
  eina_log_domain_level_set(name, EINA_LOG_LEVEL_DBG);
  eina_log_print_cb_set(_ggg_log_print_cb, stderr);

  // Shut up the excess debugging shit from EFL.
  eina_log_domain_level_set("eo", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("eldbus", EINA_LOG_LEVEL_CRITICAL);
  eina_log_domain_level_set("eet", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("efreet_icon", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_audio", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_con", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_evas", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_input_evas", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_input_evas", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ecore_system_upower", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("eio", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("ephysics", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("evas", EINA_LOG_LEVEL_WARN);
  eina_log_domain_level_set("evas_main", EINA_LOG_LEVEL_ERR);

  return logDom;
}

char *getDateTime(struct tm **nowOut, char *dateOut, time_t *timeOut)
{
  struct tm *newTime;
  time_t  szClock;
  char *date = dateTime;

  // Get time in seconds
  time(&szClock);
  // Convert time to struct tm form
  newTime = localtime(&szClock);

  if (nowOut)
    *nowOut = newTime;
  if (dateOut)
    date = dateOut;
  if (timeOut)
    *timeOut = szClock;

  // format
  strftime(date, DATE_TIME_LEN, "%d/%m/%Y %H:%M:%S\r", newTime);
  return (dateTime);
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
