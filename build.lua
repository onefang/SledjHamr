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

compileFiles = function (name, dir, files, extras)
  local objects = ''
  print('\n' .. name)
  for i, v in ipairs(files) do
    print('  ' .. v)
    os.execute('cd ' .. dir .. '; gcc ' .. CFLAGS .. ' -c -o ' .. v .. '.o ' .. v .. '.c')
    objects = objects .. ' ' .. v .. '.o'
  end
  os.execute('cd ' .. dir .. '; gcc ' .. CFLAGS .. ' -o ' .. name .. ' ' .. objects .. ' ' .. extras .. ' ' .. LDFLAGS .. ' ' .. libs)
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


workingDir = readCommand('pwd')
baseDir = workingDir
if 'number' == type(args) then
  for i = 1, args do
    baseDir = string.gsub(baseDir, '(.*)/.-$', '%1')
  end
end

bin_d    = baseDir
lib_d    = baseDir .. '/lib'
data_d   = baseDir .. '/media'
locale_d = baseDir .. '/locale'

-- Likely this will fail, coz Lua likes to strip out environmont variables.
-- On the other hand, there's a more direct way to get to environment variables, it would fail to.
CFLAGOPTS = readCommand('echo "$CFLAGOPTS"')

CFLAGS = '-g -Wall -I ' .. baseDir .. '/src/libraries'
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'luajit')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'eo')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'eet')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-con')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-evas')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'ecore-file')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'edje')
CFLAGS = CFLAGS .. ' ' .. pkgConfig('cflags', 'elementary')
CFLAGS = CFLAGS .. ' ' .. CFLAGOPTS

LDFLAGS = '-L ' .. baseDir .. '/lib ' .. pkgConfig('libs-only-L', 'luajit') .. ' -L /usr/lib -L /lib'
libs = '-lLumbrJack -lRunnr -lSledjHamr ' .. pkgConfig('libs', 'elementary') .. ' ' .. pkgConfig('libs', 'luajit') .. ' -lpthread -lm -Wl,-rpath,' .. baseDir .. '/lib'
LFLAGS = '-d'
EDJE_FLAGS = '-id ' .. baseDir .. '/media -fd ' .. baseDir .. '/media'


if 'nil' == type(args) then
  -- Building this passes my "holding breath" test, if it can compile while I'm holding my breath, no need for make files.
  print('_______________ BUILDING lemon _______________')
  removeFiles('libraries/lemon', {'*.o', 'lemon'})
  compileFiles('lemon', 'libraries/lemon', {'lemon'}, '')
  print('_______________ BUILDING Irrlicht _______________')
  -- Irrlicht is an external project that comes with make files anyway, and doesn't otherwise pass the test.
  runCommand('Irrlicht','libraries/irrlicht-1.8.1/source/Irrlicht', 'make')
  buildSub('libraries',	'src/libraries')
  buildSub('LuaSL',	'src/LuaSL')
  buildSub('GuiLua',	'src/GuiLua')
  buildSub('extantz',	'src/extantz')
end
