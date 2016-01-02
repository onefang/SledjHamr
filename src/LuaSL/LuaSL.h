#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define __UNUSED__
#endif

#include <Eet.h>
#include <Ecore_File.h>

typedef struct _gameGlobals gameGlobals;	// Define this here, so LuaSL_threads.h can use it.

#include "LumbrJack.h"
#include "Runnr.h"
#include "SledjHamr.h"


struct _gameGlobals
{
    Eina_Hash		*names;
    const char		*address;
    int			port;
};


#include "LuaSL_LSL_tree.h"
