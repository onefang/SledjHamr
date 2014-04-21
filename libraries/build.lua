#!/usr/bin/env lua

local dir = ...

if 'nil' == type(dir) then
  local build, err = loadfile('../build.lua')
  if build then
    setfenv(build, getfenv(2))
    build('')
  else
    print("ERROR - " .. err)
  end
  dir = workingDir
end

LDFLAGS = '-L ' .. dir .. ' ' .. LDFLAGS

removeFiles(dir, {'LumbrJack.o', 'libLumbrJack.so', 'Runnr.o', 'libRunnr.so'})

runCommand('C libraries',	dir, 'gcc ' .. CFLAGS .. ' -fPIC -c LumbrJack.c')
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -shared -Wl,-soname,libLumbrJack.so -o libLumbrJack.so LumbrJack.o')

runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -fPIC -c Runnr.c')
runCommand(nil,			dir, 'gcc ' .. CFLAGS .. ' -shared -Wl,-soname,libRunnr.so -o libRunnr.so Runnr.o')
