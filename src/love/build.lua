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


removeFiles(dir, {'../../love', '*.o', '../../media/love.edj'})

runCommand('edje_cc', dir, 'edje_cc ' .. EDJE_FLAGS .. ' love.edc ../../media/love.edj')

-- While SledHamr.c does this, we can't use that here, coz love is not an Elm app.
CFLAGS = CFLAGS .. ' -DPACKAGE_BIN_DIR=\\"'    .. bin_d    .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_LIB_DIR=\\"'    .. lib_d    .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_DATA_DIR=\\"'   .. data_d   .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_LOCALE_DIR=\\"' .. locale_d .. '\\"'

compileFiles('../../love', dir, {'love'}, '')
