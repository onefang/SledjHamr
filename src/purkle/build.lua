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

CFLAGS  = CFLAGS  .. ' -I../GuiLua'
LDFLAGS = '-L ' .. dir .. ' ' .. LDFLAGS
libs    = libs    .. ' -lGuiLua'

removeFiles(dir, {lib_d .. '/purkle.so'})

runCommand('C modules',		dir, 'gcc ' .. CFLAGS .. ' -fPIC -shared -o ' .. lib_d .. '/purkle.so purkle.c')
