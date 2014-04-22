#include "GuiLua.h"


EAPI_MAIN int elm_main(int argc, char **argv)
{
  elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
  elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
  elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
  elm_app_info_set(elm_main, "GuiLua", "skang.lua");

  GuiLuaDo(argc, argv);

  return 0;
}

ELM_MAIN()
