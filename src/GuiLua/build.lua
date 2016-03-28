#!/usr/bin/env luajit

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

removeFiles(dir, {'test_c.so', 'GuiLua.o', lib_d .. '/libGuiLua.so', bin_d .. '/skang'})

runCommand('C libraries',	dir, 'gcc ' .. CFLAGS .. ' -fPIC -c GuiLua.c')
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -shared -Wl,-soname,libGuiLua.so -o ' .. lib_d .. '/libGuiLua.so GuiLua.o')
runCommand('C apps',		dir, 'gcc ' .. CFLAGS .. ' -Wl,-export-dynamic -o ' .. bin_d .. '/skang skang.c ' .. LDFLAGS .. ' -lGuiLua ' .. libs)
runCommand('C modules',		dir, 'gcc ' .. CFLAGS .. ' -fPIC -shared -o test_c.so test_c.c')
