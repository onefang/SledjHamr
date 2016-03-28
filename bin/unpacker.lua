#!/usr/bin/env luajit

local args = ...
local tmpFile = os.tmpname()

readCommand = function (command)
  os.execute(command .. ' >' .. tmpFile)
  local tf = io.open(tmpFile, 'r')
  local result = tf:read()
  tf:close()
  return result
end

function scanDir(directory)
  local i, t, popen = 0, {}, io.popen
--  for filename in popen('dir "'..directory..'" /b'):lines() do
  for filename in popen('ls -AF "'..directory..'"'):lines() do
    i = i + 1
    t[i] = filename
  end
  return t
end


--workingDir = readCommand('pwd')
--baseDir = workingDir
--baseDir = string.gsub(baseDir, '(.*)/.-$', '%1')

--bin_d    = baseDir .. '/bin'
--lib_d    = baseDir .. '/lib'
--data_d   = baseDir .. '/media'
--locale_d = baseDir .. '/locale'
home_d   = readCommand('echo "$HOME"')


unpackers =
{
  iar		= function (src, dst)  return('tar -xzf ' .. src .. ' -C ' .. dst)  end,
  oar		= function (src, dst)  return('tar -xzf ' .. src .. ' -C ' .. dst)  end,
  rar		= function (src, dst)  return('unrar x '  .. src .. ' '    .. dst)  end,
  tar_bz2	= function (src, dst)  return('tar -xzf ' .. src .. ' -C ' .. dst)  end,
  tar_gz	= function (src, dst)  return('tar -xzf ' .. src .. ' -C ' .. dst)  end,
  tgz		= function (src, dst)  return('tar -xzf ' .. src .. ' -C ' .. dst)  end,
  zip		= function (src, dst)  return('unzip '    .. src .. ' -d ' .. dst)  end,
}

--[[ TODO
Make it accept arguments - file dir
  file = archive file, or directory to scan
  dir  = output directory

If it's an OAR, or other sim defining format, or a model format, then create .omg files, stuff into .cache/converted/ or inventory/converted.
  Or should conversion be a separate tool?
    The problem is that we need to link back to the original archive, or carry other info like the archive type with us from here.
    So might as well start the process by creating basic .omg files, with the info we have here, before we lose that info.

]]

print('Searching for archives in ' .. home_d .. '/.SledjHamr', ' -> ', ' unpack into ' .. home_d .. '/.SledjHamr/.cache/unpacked/')

for k, v in pairs(scanDir(home_d .. '/.SledjHamr')) do

  -- First find if there's one of the special flags at the end, and strip it off.
  t = string.sub(v, -1, -1)
  v = string.sub(v, 1, -2)
  f = true
      if '@' == t then t = t
  elseif '*' == t then t = t
  elseif '/' == t then f = false
  elseif '=' == t then t = t
  elseif '>' == t then t = t
  elseif '|' == t then t = t
  else   v = v .. t;   t = ' ';
  end

  -- Figure out what sort of file it is.
  name, ext = string.match(v, "(.*)%.(.*)$")
  if f and nil ~= ext then
    name1, tar = string.match(name, "(.*)%.(.*)$")
    if 'tar' == tar then
      name = name1
      ext  = tar .. '_' .. ext
    end

    ext = string.lower(ext)
    u = unpackers[ext]
    if nil ~= u then
      src = home_d .. '/.SledjHamr/' .. v
      dst = home_d .. '/.SledjHamr/.cache/unpacked/' .. name

      os.execute('rm -fr ' .. dst)
      os.execute('mkdir -p ' .. dst)
      print('un' .. string.upper(ext) .. 'ing ', '"' .. v .. '"', ' -> ', '"' .. name .. '"')
          if '@' == t then print('  ' .. v, ' is a  soft link.')
      elseif '*' == t then print('  ' .. v, ' is an executable.')
      elseif '/' == t then print('  ' .. v, ' is a  directory.');
      elseif '=' == t then print('  ' .. v, ' is a  socket.')
      elseif '>' == t then print('  ' .. v, ' is a  door.')
      elseif '|' == t then print('  ' .. v, ' is a  FIFO.')
--      else                 print('  ' .. v, ' is an ordinary file.')
      end
      os.execute(u(src, dst) .. ' >/dev/null')
    end
  end
end
