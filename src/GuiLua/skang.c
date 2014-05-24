#include "LumbrJack.h"
#include "GuiLua.h"


int logDom = -1;

EAPI_MAIN int elm_main(int argc, char **argv)
{
  logDom = HamrTime(argv[0], elm_main, logDom);
  GuiLuaDo(argc, argv, NULL, NULL);
  pantsOff(logDom);

  return 0;
}

ELM_MAIN()
