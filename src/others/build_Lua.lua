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

baseDir = '/usr/local/'

cloneGit('luajit-2.0', '.', 'luajit.org/git', '')
runCommand('LuaJIT', 'luajit-2.0', 'make clean')
runCommand(nil, 'luajit-2.0', 'make amalg PREFIX=' .. baseDir)
runCommand(nil, 'luajit-2.0', 'sudo make install PREFIX=' .. baseDir)
-- This link prevents linking to LuaJIT when it works, the Ubuntu one supplies a broken link instead, which also "works".
runCommand(nil, 'luajit-2.0', 'sudo rm ' .. baseDir .. '/lib/libluajit-5.1.so')
