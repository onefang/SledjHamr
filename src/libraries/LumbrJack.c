/*  LumbrJack - a logging library that wraps Eina logging.

*/


#include <unistd.h>
#include "LumbrJack.h"


static char dateTime[DATE_TIME_LEN];
static Eina_Prefix *prefix = NULL;

int HamrTime(char *argv0, void *main, int logDom)
{
  Eina_Array *path;
  char *env, name[PATH_MAX], cwd[PATH_MAX], temp[PATH_MAX * 2];
  int i, len;

  if (!eina_init())
  {
    fprintf(stderr, "Can't load eina library, nothing else will work!\n");
    exit(0);
  }

  // Coz eina_file_split splits the string in place, instead of making a copy first.
  snprintf(temp, sizeof(temp), "%s", argv0);
  path = eina_file_split(temp);
  snprintf(name, sizeof(name), "%s", (char *) eina_array_data_get(path, eina_array_count(path) - 1));
  logDom = loggingStartup(name, logDom);
  eina_array_free(path);

  len = strlen(name);
  temp[len] = 0;
  cwd[len]  = 0;
  for (i = 0;  i < len;  i++)
  {
    temp[i] = toupper(name[i]);
    cwd[i]  = tolower(name[i]);
  }

  if (!(prefix = eina_prefix_new(argv0, main, temp, cwd, "checkme.txt", PACKAGE_BIN_DIR, PACKAGE_LIB_DIR, PACKAGE_DATA_DIR, PACKAGE_LOCALE_DIR)))
    PC("Can't find application prefix!");

//  PD("%s is installed in %s", name, eina_prefix_get(prefix));
//  PD("The binaries are in %s", eina_prefix_bin_get(prefix));
//  PD("The data files are in %s", eina_prefix_data_get(prefix));
//  PD("The libraries are in %s", eina_prefix_lib_get(prefix));
//  PD("The locale files are in %s", eina_prefix_locale_get(prefix));
// TODO - ecore_file_escape_name(const char *filename) might also be useful.
//        EFL doesn't seem to support links under Windows.  Though apparently Windows itself has something similar.
//  PD("The HOME directory is %s", eina_environment_home_get());
//  PD("The TMP directory is %s", eina_environment_tmp_get());

  getcwd(cwd, PATH_MAX);
  env = getenv("LUA_CPATH");
  if (!env)  env = "";
  sprintf(temp, "%s;%s/lib?.so;%s/?.so;%s/?.so", env, eina_prefix_lib_get(prefix), eina_prefix_lib_get(prefix), cwd);
  setenv("LUA_CPATH", temp, 1);

  env = getenv("LUA_PATH");
  if (!env)  env = "";
  sprintf(temp, "%s;%s/?.lua;%s/?.lua", env, eina_prefix_lib_get(prefix), cwd);
  setenv("LUA_PATH", temp, 1);

  return logDom;
}

const char *prefix_get()		{return eina_prefix_get(prefix);}
const char *prefix_bin_get()		{return eina_prefix_bin_get(prefix);}
const char *prefix_data_get()		{return eina_prefix_data_get(prefix);}
const char *prefix_lib_get()		{return eina_prefix_lib_get(prefix);}
const char *prefix_locale_get()		{return eina_prefix_locale_get(prefix);}

void pantsOff(int logDom)
{
  if (logDom >= 0)
    eina_log_domain_unregister(logDom);

  eina_prefix_free(prefix);

  eina_shutdown();
}

static void _logPrint(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args)
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
  eina_log_threads_enable();

  if (logDom < 0)
  {
    logDom = eina_log_domain_register(name, EINA_COLOR_ORANGE);
    if (logDom < 0)
    {
      EINA_LOG_CRIT("could not register log domain '%s'", name);
      return logDom;
    }
  }
  eina_log_level_set(EINA_LOG_LEVEL_DBG);
  eina_log_domain_level_set(name, EINA_LOG_LEVEL_DBG);
  eina_log_print_cb_set(_logPrint, stderr);

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
  eina_log_domain_level_set("EvasGL", EINA_LOG_LEVEL_WARN);
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
  strftime(date, DATE_TIME_LEN, "%Y-%m-%d %H:%M:%S\r", newTime);
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
