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

cloneHG('prosody', '.', 'hg.prosody.im/trunk', '')
runCommand(nil, 'prosody', './configure --ostype="debian" && make')
