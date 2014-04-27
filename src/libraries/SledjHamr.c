#include "SledjHamr.h"

void HamrTime(void *elm_main, char *domain)
{
  elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
  elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
  elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
  elm_app_compile_locale_set(PACKAGE_LOCALE_DIR);
  // Do this after the above calls, but before changing the working directory, or screwing with argv[0].
  // It tries to set up the package paths depending on where the executable is, so things are relocatable.
  // First argument is the elm_main() function that Elementary starts us from.
  // Second argument should be a lower case string used as the "domain", which is different from the log domain.
  //   It's used lower case as part of the data directory path.
  //     So, if prefix is /usr/local, then the system data dir is /usr/local/share,
  //     and this apps data dir is /usr/local/share/"domain".
  //   It's used upper case as part of environment variables to override directory paths at run time.
  //     So "DOMAIN"_PREFIX, "DOMAIN"_BIN_DIR, "DOMAIN"_LIB_DIR, "DOMAIN"_DATA_DIR, and "DOMAIN"_LOCALE_DIR
  // Third argument is the name of a file it can check for to make sure it found the correct path.
  //  This file is looked for in the data dir.
  elm_app_info_set(elm_main, domain, "checkme.txt");
  // Once this is all setup, the code can do -
  // elm_app_prefix_dir_get();  // or bin, lib, data, locale.
}
