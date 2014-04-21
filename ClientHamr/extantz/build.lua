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

CFLAGS  = CFLAGS  .. ' -I../../libraries/irrlicht-1.8.1/include -I/usr/X11R6/include'
LDFLAGS = LDFLAGS .. ' -L../../libraries/irrlicht-1.8.1/lib/Linux'
libs    = libs    .. ' -lIrrlicht -lGL -lbz2'

removeFiles(dir, {'extantz', 'crappisspuke.o', 'CDemo.o', 'extantzCamera.o', 'extantz.edj'})

runCommand('edje_cc',		dir, 'edje_cc ' .. EDJE_FLAGS .. ' extantz.edc extantz.edj')
runCommand('Irrlicht files',	dir, 'g++ ' .. CFLAGS .. ' -O3 -ffast-math -c crappisspuke.cpp -o crappisspuke.o ' .. LDFLAGS)
runCommand(nil,			dir, 'g++ ' .. CFLAGS .. ' -O3 -ffast-math -c CDemo.cpp -o CDemo.o ' .. LDFLAGS)
runCommand('extantz',		dir, 'g++ ' .. CFLAGS .. ' -O3 -ffast-math -c extantzCamera.cpp -o extantzCamera.o ' .. LDFLAGS)
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' extantz.c crappisspuke.o CDemo.o extantzCamera.o -o extantz ' .. LDFLAGS .. ' ' .. libs .. ' && strip extantz')
