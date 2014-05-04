#!/usr/bin/env lua

local dir = ...

if 'nil' == type(dir) then
  local build, err = loadfile('../../build.lua')
  if build then
    setfenv(build, getfenv(2))
    build(2)
  else
    print("ERROR - " .. err)
  end
  dir = workingDir
end

LDFLAGS = '-L ' .. dir .. ' ' .. LDFLAGS

removeFiles(dir, {'LumbrJack.o', lib_d .. '/libLumbrJack.so', 'Runnr.o', lib_d .. '/libRunnr.so', 'SledjHamr.o', lib_d .. '/libSledjHamr.so', 'winFang.o', lib_d .. '/libwinFang.so'})

runCommand('C libraries',	dir, 'gcc ' .. CFLAGS .. ' -fPIC -c LumbrJack.c')
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -shared -Wl,-soname,libLumbrJack.so -o ' .. lib_d .. '/libLumbrJack.so LumbrJack.o')

runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -fPIC -c Runnr.c')
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -shared -Wl,-soname,libRunnr.so -o ' .. lib_d .. '/libRunnr.so Runnr.o')

-- For Elm apps, these are all centrally controlled in libSledjHamr.
CFLAGS = CFLAGS .. ' -DPACKAGE_BIN_DIR=\\"'    .. bin_d    .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_LIB_DIR=\\"'    .. lib_d    .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_DATA_DIR=\\"'   .. data_d   .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_LOCALE_DIR=\\"' .. locale_d .. '\\"'

runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -fPIC -c SledjHamr.c')
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -shared -Wl,-soname,libSledjHamr.so -o ' .. lib_d .. '/libSledjHamr.so SledjHamr.o')

runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -fPIC -c winFang.c')
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -shared -Wl,-soname,libwinFang.so -o ' .. lib_d .. '/libwinFang.so winFang.o')
