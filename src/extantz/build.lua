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

removeFiles(dir, {'crappisspuke.o', 'CDemo.o', 'extantzCamera.o', 'gears.o', 'ephysics_demo.o', 'Evas_3D_demo.o', '../../media/extantz.edj'})
removeFiles(dir, {'../../extantz', 'camera.o', 'winFang.o', 'chat.o', 'files.o', 'woMan.o'})

runCommand('edje_cc',		dir, 'edje_cc ' .. EDJE_FLAGS .. ' extantz.edc ../../media/extantz.edj')
runCommand('Irrlicht files',	dir, 'g++ ' .. CFLAGS .. ' -ffast-math -c crappisspuke.cpp -o crappisspuke.o ' .. LDFLAGS)
runCommand(nil,			dir, 'g++ ' .. CFLAGS .. ' -ffast-math -c CDemo.cpp -o CDemo.o ' .. LDFLAGS)
runCommand(nil,			dir, 'g++ ' .. CFLAGS .. ' -ffast-math -c extantzCamera.cpp -o extantzCamera.o ' .. LDFLAGS)
runCommand('C libraries',	dir, 'gcc ' .. CFLAGS .. ' -fPIC -c winFang.c')
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -shared -Wl,-soname,libwinFang.so -o ' .. lib_d .. '/libwinFang.so winFang.o')
compileFiles('../../extantz',	dir, {'gears', 'ephysics_demo', 'camera', 'Evas_3D_demo', 'chat', 'files', 'woMan', 'extantz'}, 'crappisspuke.o CDemo.o extantzCamera.o -lwinFang')
