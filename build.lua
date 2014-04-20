#!/usr/bin/env lua

local args = ...
local tmpFile = os.tmpname()

readCommand = function (command)
  os.execute(command .. ' >' .. tmpFile)
  local tf = io.open(tmpFile, 'r')
  local result = tf:read()
  tf:close()
  return result
end

pkgConfig = function (what, name)
  return readCommand('pkg-config --' .. what .. ' ' .. name)
end

removeFiles = function (dir, files)
  print('clean')
  for i, v in ipairs(files) do
    os.execute('rm -f ' .. dir .. '/' .. v)
  end
end

runCommand = function (name, dir, command)
  if name then print('\n' .. name) end
  os.execute('cd ' .. dir .. '; ' .. command)
end

compileFiles = function (name, dir, files)
  local objects = ''
  print('\n' .. name)
  for i, v in ipairs(files) do
    print('  ' .. v)
    os.execute('cd ' .. dir .. '; gcc ' .. CFLAGS .. ' -c -o ' .. v .. '.o ' .. v .. '.c')
    objects = objects .. ' ' .. v .. '.o'
  end
  os.execute('cd ' .. dir .. '; gcc ' .. CFLAGS .. ' -o ' .. name .. ' ' .. objects .. ' ' .. LDFLAGS .. ' ' .. libs)
end

local buildSub = function (name, dir)
  print('_______________ BUILDING ' .. name .. ' _______________')
  local build, err = loadfile(dir .. '/build.lua')
  if build then
    setfenv(build, getfenv(2))
    build(workingDir .. '/' .. dir)
  else
    print("ERROR - " .. err)
  end
end

-- Likely this will fail, coz Lua likes to strip out environmont variables.
-- On the other hand, there's a more direct way to get to environment variables, it would fail to.
CFLAGOPTS = readCommand('echo "$CFLAGOPTS"')

workingDir = readCommand('pwd')
-- TODO - -I ../../libraries will be wrong for somethings, but right now those things don't care.
CFLAGS = '-g -Wall -I include -I ../../libraries'
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'luajit')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'eo')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'eet')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-con')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-evas')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-file')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'edje')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'elementary')
-- TODO - The workingDir part of these strings gets set differently depending on who starts the build.
--        Which is a problem for the PACKAGE_*_DIR defines.
--        On the other hand, that part needs to be rethought anyway, coz otherwise they are locked to the build place.
CFLAGS = CFLAGS .. ' -DPACKAGE_BIN_DIR=\\"'  .. workingDir .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_LIB_DIR=\\"'  .. workingDir .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_DATA_DIR=\\"' .. workingDir .. '\\"'
CFLAGS = CFLAGS .. ' ' .. CFLAGOPTS

LDFLAGS = pkgConfig('libs-only-L', 'luajit') .. ' -L lib -L /usr/lib -L /lib'
libs = pkgConfig('libs', 'elementary') .. ' ' .. pkgConfig('libs', 'luajit') .. ' -lpthread -lm'
LFLAGS = '-d'
EDJE_FLAGS = '-id images -fd fonts'


if 'nil' == type(args) then
  -- Building this passes my "holding breath" test, if it can compile while I'm holding my breath, no need for make files.
  print('_______________ BUILDING lemon _______________')
  removeFiles('libraries/lemon', {'*.o', 'lemon'})
  compileFiles('lemon', 'libraries/lemon', {'lemon'})
  print('_______________ BUILDING Irrlicht _______________')
  -- Irrlicht is an external project that comes with make files anyway, and doesn't otherwise pass the test.
  runCommand('Irrlicht', 'libraries/irrlicht-1.8.1/source/Irrlicht', 'make')
  buildSub('LumbrJack',	'LumbrJack')
  buildSub('Runnr',	'Runnr')
  buildSub('LuaSL',	'LuaSL')
  buildSub('GuiLua',	'ClientHamr/GuiLua')
  buildSub('extantz',	'ClientHamr/extantz')
end
