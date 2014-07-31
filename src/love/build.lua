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


removeFiles(dir, {'../../media/love.edj'})

runCommand('edje_cc', dir, 'edje_cc ' .. EDJE_FLAGS .. ' love.edc ../../media/love.edj')
compileFiles('../../love', dir, {'love'}, '')
