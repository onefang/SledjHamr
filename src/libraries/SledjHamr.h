#ifndef _SLEDJHAMR_H_
#define _SLEDJHAMR_H_


#define EFL_API_OVERRIDE 1
/* Enable access to unstable EFL API that are still in beta */
#define EFL_BETA_API_SUPPORT 1
/* Enable access to unstable EFL EO API. */
#define EFL_EO_API_SUPPORT 1

#include <ctype.h>

#include <Elementary.h>


void HamrTime(void *elm_main, char *domain);

#endif
