#include "SledjHamr.h"
#include "LumbrJack.h"
#include "Runnr.h"
#include "winFang.h"
#include "GuiLua.h"


EAPI_MAIN int elm_main(int argc, char **argv)
{
  HamrTime(elm_main, "GuiLua");
  GuiLuaDo(argc, argv, NULL);

  return 0;
}

ELM_MAIN()
