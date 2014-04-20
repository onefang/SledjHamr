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
  local build, err = loadfile(LOCALDIR .. '/' .. dir .. '/build.lua')
  if build then
    setfenv(build, getfenv(2))
    build(LOCALDIR .. '/' .. dir)
  else
    print("ERROR - " .. err)
  end
end

-- Likely this will fail, coz Lua likes to strip out environmont variables.
-- On the other hand, there's a more direct way to get to environment variables, it would fail to.
CFLAGOPTS = readCommand('echo "$CFLAGOPTS"')

LOCALDIR = readCommand('pwd')readCommand('pwd')
CFLAGS = '-g -Wall -I include -I ' .. LOCALDIR .. ' -I ../../libraries'
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'luajit')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'eo')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'eet')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-con')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-evas')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-file')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'edje')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'elementary')
CFLAGS = CFLAGS .. ' -DPACKAGE_BIN_DIR=\\"'  .. LOCALDIR .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_LIB_DIR=\\"'  .. LOCALDIR .. '\\"'
CFLAGS = CFLAGS .. ' -DPACKAGE_DATA_DIR=\\"' .. LOCALDIR .. '\\"'
CFLAGS = CFLAGS .. ' ' .. CFLAGOPTS

LDFLAGS = '-L ' .. LOCALDIR .. ' ' ..  pkgConfig('libs-only-L', 'luajit') .. ' -L lib -L /usr/lib -L /lib'
libs = pkgConfig('libs', 'elementary') .. ' ' .. pkgConfig('libs', 'luajit') .. ' -lpthread -lm'
LFLAGS = '-d'
EDJE_FLAGS = '-id images -fd fonts'


if 'nil' == type(args) then
  print('_______________ BUILDING lemon _______________')
  removeFiles(LOCALDIR .. '/libraries/lemon', {'*.o', 'lemon'})
  compileFiles('lemon', LOCALDIR .. '/libraries/lemon', {'lemon'})
  print('_______________ BUILDING Irrlicht _______________')
  runCommand('Irrlicht', 'libraries/irrlicht-1.8.1/source/Irrlicht', 'make')
  buildSub('LuaSL',	'LuaSL')
  buildSub('GuiLua',	'ClientHamr/GuiLua')
  buildSub('extantz',	'ClientHamr/extantz')
end
