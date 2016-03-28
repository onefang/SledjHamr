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

runCommand('libg3d', 'mimesh/libg3d-0.0.8', 'make clean')
runCommand(nil, 'mimesh/libg3d-0.0.8', './configure && make')
runCommand('g3dviewer', 'mimesh/g3dviewer-0.2.99.4', './configure && make')
runCommand(nil, 'mimesh/g3dviewer-0.2.99.4', 'make clean')
