#include "LuaSL.h"


// "01:03:52 01-01-1973\n\0"
#    define DATE_TIME_LEN	21


char    dateTime[DATE_TIME_LEN];


static
void _ggg_log_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args)
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

void loggingStartup(gameGlobals *game)
{
    game->logDom = eina_log_domain_register("LuaSL", NULL);
    if (game->logDom < 0)
    {
	EINA_LOG_CRIT("could not register log domain 'LuaSL'");
    }
    // TODO - should unregister this later.
    eina_log_level_set(EINA_LOG_LEVEL_DBG);
    eina_log_domain_level_set("LuaSL", EINA_LOG_LEVEL_DBG);
    eina_log_print_cb_set(_ggg_log_print_cb, stderr);
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

//fprintf(stderr, "       %ld . %ld\n", now->tv_sec, now->tv_usec);
//fprintf(stderr, "minus  %ld . %ld\n", then->tv_sec, then->tv_usec);

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
//fprintf(stderr, "equals %lf\n", result);
//fflush(stderr);
	return result;
    }
    else
	return 0.0;
}

