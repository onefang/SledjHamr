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

local tools = {"buildsystem", "libnsutils", "nsgenbind"}
local libs  = {"libwapcaplet", "libparserutils", "libhubbub",
		"libcss",
		"libdom",

		"libnsbmp",
		"libnsgif",
		"librosprite",
		"libsvgtiny",
		"libutf8proc",
	      }

instDir = dir .. '/netsurf/inst'
exports = 'export PKG_CONFIG_PATH=' .. instDir .. '/lib/pkgconfig::;' ..
'export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:' .. instDir .. '/lib;' ..
'export PATH=${PATH}:' ..instDir .. '/bin;' ..
'export PREFIX=' .. instDir .. '; '

runCommand(nil, '.', 'rm -rf netsurf;  mkdir -p netsurf/inst')

for i, v in ipairs(tools) do
  cloneGit(v, 'netsurf', 'git.netsurf-browser.org', '')
  runCommand('NetSurf tool - ' .. v, 'netsurf', exports .. 'cd ' .. v .. '; make install')
end

for i, v in ipairs(libs) do
  cloneGit(v, 'netsurf', 'git.netsurf-browser.org', '')
  runCommand('NetSurf lib - ' .. v, 'netsurf', exports .. 'cd ' .. v .. '; make install')
end

local v = 'netsurf'
cloneGit(v, 'netsurf', 'git.netsurf-browser.org', '')
runCommand('NetSurf', 'netsurf', exports .. 'cd ' .. v .. '; make && cp nsgtk ../inst/bin/netsurf')
