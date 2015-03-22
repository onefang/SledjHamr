--[[ TODO - This should be in C, but so far development has been quite rapid doing it in Lua.

C will let us -
  Actually do the widget stuff.
  Slap meta tables on all value types.
    Which lets us put the meta table on the variable, instead of on the table, which I think is cleaner.
  Figure out the directory separator.
  Network stuff.   No need to look at Lua socket stuff, we have Ecore_Con.
  Database stuff.  No need to look at Lua SQL stuff, we have esskyuehl.  Maybe.

  Actually, we could have the best of both worlds, since it is currently a C / Lua hybrid.  B-)
]]


--[[ Skang package

In here should live all the internals of matrix-RAD that don't
specifically relate to widgets.  This would include the "window" though.

skang.module(Evas)
skang.module(Elementary)
skang.skang('some/skang/file.skang')

This package is also what "apps" that use the system should "inherit"
from, in the same way matrix-RAD apps did.  Skang "apps" could be Lua
modules.  They could also be C code, like the extantz modules are likely
to be.  Skang "apps" would automatically be associated with skang files
of the same name.

For a .skang file, the skang command (written in C) would strip off the
first line, add the two implied lines, then run it as Lua.  The
skang.load() command would do the same.  So that skang C comand would
just pass the file name to skang.load() in this library.  B-)

The old skang argument types are -

{"name",   "java.lang.String"},
{"action", "java.lang.String"},
{"type",   "java.lang.String"},
{"data",   "java.lang.String"},
{"URL",    "java.lang.String"},
{"file",   "java.lang.String"},
{"method", "java.lang.String"},
{"lx",     "java.lang.String"},
{"ly",     "java.lang.String"},
{"lw",     "java.lang.String"},
{"lh",     "java.lang.String"},
{"normal", "java.lang.String"},
{"ghost",  "java.lang.String"},
{"active", "java.lang.String"},
{"toggle", "java.lang.String"},
{"boolean","java.lang.Boolean"},
{"number", "java.lang.Integer"},
{"int",    "java.lang.Integer"},
{"x",      "java.lang.Integer"},
{"y",      "java.lang.Integer"},
{"w",      "java.lang.Integer"},
{"h",      "java.lang.Integer"},
{"r",      "java.lang.Integer"},
{"g",      "java.lang.Integer"},
{"b",      "java.lang.Integer"},
{"alpha",  "java.lang.Integer"},
{"acl",    "net.matrix_rad.security.ACL"},
]]


-- Wrapping the entire module in do .. end helps if people just join a bunch of modules together, which apparently is popular.
-- By virtue of the fact we are stuffing our result into package.loaded[], just plain running this works as "loading the module".
do	-- Only I'm not gonna indent this.

mainSkin = {}

-- TODO - This needs to be expanded a bit to cover things like 1.42
local versions = {
  '0%.0',  'unwritten',	'Just a stub, no code at all, or completely non-existant.',
  '0%.1',  'prototype',	'Stuff that has only been prototyped, or partly written.  There is some code, and it may even work, but it is not even close to the finished product.',
  '%d%.3', 'written',	'Stuff that has already been written.  It may not be perfect, but it is considered an aproximation of the finished product.',
  '%d%.5', 'alpha',	'Version being tested by official alpha testers.',
  '%d%.9', 'beta',	'Version passed alpha testing, but not ready for final release.',
  '1%.0',  'final',	'Version ready for final release, fully tested.',
  '3%.0',  'poetry',	'Near perfection has been acheived.',
  '5%.0',  'nirvana',	'Perfection has been acheived.',
  '9%.0',  'bible',	'This is the Whord of Ghod.',
}

-- Trying to capture best practices here for creating modules, especially since module() is broken and deprecated.
-- TODO - Should parse in license type to.
moduleBegin = function (name, author, copyright, version, timestamp, skin, isLua)
  local _M = {}	-- This is what we return to require().
  local level = 2

  if 'nil' == type(isLua) then isLua = true end

  package.loaded[name] = _M	-- Stuff the result into where require() can find it, instead of returning it at the end.
			-- Returning it at the end does the same thing.
			-- This is so that we can have all the module stuff at the top, in this function.
			-- Should do this before any further require(), so that circular references don't blow out.
  debug.getregistry()[name] = _M	-- Stuff the result in the C registry.

  -- Save the callers environment.
  local savedEnvironment
  if isLua then
    savedEnvironment = getfenv(level)
  else
    -- While the above works fine for test_c, it doesn't for GuiLua.  Odd.
    savedEnvironment = getfenv(1)
  end

  -- Clone the environment into _M, so the module can access everything as usual after the setfenv() below.
  --[[ TODO - Check if this also clones _G or _ENV.  And see if it leaks stuff in either direction.
	local _G = _G	-- Only sets a local _G for this function.
	_M._G = _G	-- This clone loop might do this, but we don't want to be able to access the old _G from outside via this leak.
	In Lua 5.1 at least, _G was special.  In 5.2, _ENV sorta replaces setfenv(), but no idea if this clone loop stomps on that.
  ]]
  for k, v in pairs(savedEnvironment) do
    _M[k] = v
  end

  _M._M = _M		-- So that references to _M below the setfenv() actually go to the real _M.
  --[[ TODO - Check if it exists before doing this, or could override standard Lua stuff. ]]
  _M[name] = _M		-- So that the module can refer to itself internally.
  _M._NAME = name
  _M._PACKAGE = string.gsub(_M._NAME, "[^.]*$", "")	-- Strip the name down to the package name.
  _M.isLua = isLua

  -- Parse in an entire copyright message, and strip that down into bits, to put back together.
  local date, owner = string.match(copyright, '[Cc]opyright (%d%d%d%d) (.*)')
  _M.AUTHOR = author or owner
  _M.COPYRIGHT = 'Copyright ' .. date .. ' ' .. _M.AUTHOR
  -- Translate the version number into a version string.
  local versionName, versionDesc = ' ', ''
  for i = 1, #versions / 3 do
    if 1 == string.find(version, versions[i]) then
      versionName = ' ' .. versions[i + 1] .. ' '
      versionDesc = versions[i + 2]
      break
    end
  end
  _M.VERSION = version .. versionName .. timestamp
  _M.VERSION_DESC = versionDesc

  -- If there is a .skang file, read that in and override the passed in skin.
  -- TODO - At this point it would be nice if we knew where the module came from, so we can search for the skin file in THAT directory.
  local f = io.open(name .. '.skang')
  if f then
    skin = f:read('*l')
    if '#' == string.sub(skin, 1, 1) then skin = '' end
    skin = skin .. f:read('*a')
    f:close()
  end
  if skin then
    skin = "local skang = require 'skang'\nlocal " .. name .. " = require '" .. name .. "'\n" .. skin
    if nil == mainSkin._NAME then mainSkin = _M end
  end
  _M.DEFAULT_SKANG = skin

  --_G[_M._NAME] = _M	-- Stuff it into a global of the same name.
			-- Not such a good idea to stomp on global name space.
			-- It's also redundant coz we get stored in package.loaded[_M._NAME] anyway.
			-- This is why module() is broken.
  _M.savedEnvironment = savedEnvironment
  -- NOTE - setfenv() wont work if the environment it refers to is a C function.  Worse, getfenv() returns the global environment, so we can't tell.
  if isLua then
    -- setfenv() sets the environment for the FUNCTION, stack level deep.
    -- The number is the stack level -
    --   0 running thread, 1 current function, 2 function that called this function, etc
    setfenv(level, _M)	-- Use the result for the modules internal global environment, so they don't need to qualify internal names.
			-- Dunno if this causes problems with the do ... end style of joining modules.  It does.  So we need to restore in moduleEnd().
			-- Next question, does this screw with the environment of the skang module?  No it doesn't, coz that's set up at require 'skang' time.
  end

  print('Loaded module ' .. _M._NAME .. ' version ' .. _M.VERSION .. ', ' .. _M.COPYRIGHT .. '.\n  ' .. _M.VERSION_DESC)

  return _M
end


--[[ Parse command line parameters.

This is done in two parts.  Skang will do an initial scan and tokenise,
then each module gets a chance to pull it's own Things from the result.

Make the command line parameter getting MUCH more intelligent, try to support the common 
command line interfaces -

arg value
a value
/arg value
/a value
--arg value
--a value
-a value
-ab  ('a' and 'b' are both shortcuts.)
arg=value
a=value
arg1=value1&arg2=value2
arg1=value1|arg2=value2
a=value1&a=value2
+arg/-arg  (Can't support this generically.)

Ignore /,-,--,& except as arg introducers. Use = as value introducer.  Expect 
arg or a.  If type is String, expect a value.  If type is integer, and next token is 
not an integer, increment current value, otherwise expect integer value.  If type is
boolean, value beginning with T, t, F, f, etc is true, otherwise value is false, unless 
next token starts with an introducer, then value is true.

TODO - Finish supporting all of the above.
         These all need deeper parsing, but we dunno if they might have been inside quoted strings from the shell.
           arg=value			Shell.
           arg1=value1&arg2=value2	For URLs.
           arg1=value1|arg2=value2	Can't remember why, probably the old skang multivalue syntax.
       Test it all.
       Skang command line should have standardish stuff, like --version, --help, --help module.thing.
         Lua does these already, might be no need to do them ourselves -
           -e 'some code'.
           -i go interactive after running the script.
           -v version.
           - read from stdin non interactively.
         LuaJIT also has this -
           -- stop processing options.
]]

ARGS = {}
lua = ''
command = ''


-- Do an initial scan and tokenise of the command line arguments.
scanArguments = function (args)
  if args then
    lua = args[-1]
    command = args[0]
    for i, v in ipairs(args) do
      local pre = ''
      if '--' == string.sub(v, 1, 2) then pre = '--';  v = string.sub(v, 3, -1) end
      if '-'  == string.sub(v, 1, 1) then pre = '-';   v = string.sub(v, 2, -1) end
      if '+'  == string.sub(v, 1, 1) then pre = '+';   v = string.sub(v, 2, -1) end
      -- TODO - Make this the opposite of the directory separator for what ever platform we are running on.
      --        Which Lua can't figure out I think.
      if '/'  == string.sub(v, 1, 1) then pre = '/';   v = string.sub(v, 2, -1) end
      if '='  == string.sub(v, 1, 1) then pre = '=';   v = string.sub(v, 2, -1) end
      if '&'  == string.sub(v, 1, 1) then pre = '&';   v = string.sub(v, 2, -1) end
      if '|'  == string.sub(v, 1, 1) then pre = '|';   v = string.sub(v, 2, -1) end
      if '' ~= v then ARGS[i] = {pre, v} end
    end
  end
end

parseType = function (module, thingy, v, value)
  if 'string' == thingy.types[1] then
    if value then
      module[v[2] ] = value[2]
      value[2] = nil	-- Mark it as used.
    else
      print('ERROR - Expected a string value for ' .. thingy.names[1])
    end
  end

  if 'number' == thingy.types[1] then
    if value then
      -- If the introducer is '-', then this should be a negative number.
      if '-' == value[1] then value[1] = '';  value[2] = '-' .. value[2] end
      -- Only parse the next value as a number if it doesn't have an introducer.
      if ('' == value[1]) or ('=' == value[1]) then
        value[2] = tonumber(value[2])
        if value[2] then
          module[v[2] ] = value[2]
          value[2] = nil	-- Mark it as used.
        else
          print('ERROR - Expected a number value for ' .. thingy.names[1])
        end
      else
        module[v[2] ] = module[v[2] ] + 1
      end
    else
      print('ERROR - Expected a number value for ' .. thingy.names[1])
    end
  end

  if 'function' == thingy.types[1] then
    local args = {}
    -- TODO - Should allow more than one argument, but would need to pass in ARGS and i.
    if 2 == #thingy.types then
      if value then
        -- TODO - Should check the type of the arguments.
        args[#args + 1] = value[2]
        module[v[2] ](args[1])
        value[2] = nil	-- Mark it as used.
      else
        print('ERROR - Expected an argument for ' .. thingy.names[1])
      end
    else
      module[v[2] ]()
    end
  end

  if 'boolean' == thingy.types[1] then
    if value then
      -- Only parse the next value as a boolean if it doesn't have an introducer.
      if ('' == value[1]) or ('=' == value[1]) then
        module[v[2] ] = isBoolean(value[2])
        value[2] = nil	-- Mark it as used.
      else
        module[v[2] ] = true
      end
    else
      print('ERROR - Expected a boolean value for ' .. thingy.names[1])
    end
  end
end

pullArguments = function (module)
  -- Look for our command line arguments.
  local metaMum = getmetatable(module)
  if metaMum and metaMum.__self then
    for i, v in ipairs(ARGS) do
      if v[2] then
        local thingy = metaMum.__self.stuff[v[2] ]
        -- Did we find one of ours?
        if thingy then
          parseType(module, thingy, v, ARGS[i + 1])
          v[2] = nil		-- Mark it as used.
        else
          -- Didn't find one directly, check for single letter matches in '-abc'.
          for k, w in pairs(metaMum.__self.stuff) do
            if 1 == #w.names[1] then
              for j = 1, #v[2] do
                if string.sub(v[2], j, 1) == w.names[1] then
                  if 1 == j then
                    v[2] = string.sub(v[2], 2, -1)
                    if 'boolean' == w.types[1] then module[v[2] ] = true end
                  elseif #v[2] == j then 
                    v[2] = string.sub(v[2], 1, j - 1)
                    -- The one at the end is the only one that could have a following value.
                    parseType(module, w, v, ARGS[i + 1])
                  else
                    v[2] = string.sub(v[2], 1, j - 1) .. string.sub(v[2], j + 1, -1)
                    if 'boolean' == w.types[1] then module[v[2] ] = true end
                  end
                  if '' == v[2] then v[2] = nil end	-- Mark it as used.
                end
              end
            end
          end
        end
      end
    end
  end
end

-- Restore the environment, and grab paramateres from standard places.
moduleEnd = function (module)
  -- See if there is a properties file, and run it in the modules environment.
  local properties, err = loadfile(module._NAME .. '.properties')
  if properties then
    setfenv(properties, getfenv(2))
    properties()
  elseif 'cannot open ' ~= string.sub(err, 1, 12) then
    print("ERROR - " .. err)
  end

  pullArguments(module)

  -- Run the main skin, which is the first skin that is defined.  In theory, the skin from the main module.
  if mainSkin == module then
    print("RUNNING SKIN FOR " .. module._NAME)
    local skin, err = loadstring(module.DEFAULT_SKANG)
    if skin then
      setfenv(skin, getfenv(2))
      skin()
    else
      print("ERROR - " .. err)
    end
  end

  if module.isLua then setfenv(2, module.savedEnvironment) end
end


-- Call this now so that from now on, this is like any other module.
local _M = moduleBegin('skang', 'David Seikel', 'Copyright 2014 David Seikel', '0.1', '2014-03-27 02:57:00')

-- This works coz LuaJIT automatically loads the jit module.
if type(jit) == 'table' then
  print('Skang is being run by ' .. jit.version .. ' under ' .. jit.os .. ' on a ' .. jit.arch)
else
  print('Skang is being run by Lua version ' .. _VERSION)
end

scanArguments(arg)


function printTableStart(table, space, name)
    print(space .. name .. ": ")
    print(space .. "{")
    printTable(table, space .. "  ")
    print(space .. "}")
    if '' == space then print('') end
end

function printTable(table, space)
    if nil == table then return end
    for k, v in pairs(table) do 
	if type(v) == "table" then
	    if v._NAME then
	      print(space .. "SKANG module " .. v._NAME .. ";")
	    else
	      printTableStart(v, space, k)
	    end
	elseif type(v) == "string" then
	    print(space .. k .. ': "' .. v .. '";')
	elseif type(v) == "function" then
	    print(space .. "function " .. k .. "();")
	elseif type(v) == "userdata" then
	    print(space .. "userdata " .. k .. ";")
	elseif type(v) == "boolean" then
	    if (v) then
		print(space .. "boolean " .. k .. " TRUE ;")
	    else
		print(space .. "boolean " .. k .. " FALSE ;")
	    end
	else
	    print(space .. k .. ": " .. v .. ";")
	end
    end
end


csv2table = function (csv)
  local result = {}
  local i = 1

  for v in string.gmatch(csv, ' *([^,]+)') do
    result[i] = v
    i = i + 1
  end
  return result
end


shiftLeft = function (tab)
  local result = tab[1]
  table.remove(tab, 1)
  return result
end


-- My clever boolean check, this is the third language I've written this in.  B-)
-- true   1 yes ack  ok   one  positive absolutely affirmative  'ah ha' 'shit yeah' 'why not'
local isTrue  = 't1aopswy'
-- false  0 no  nack nope zero negative nah 'no way' 'get real' 'uh uh' 'fuck off' 'bugger off'
local isFalse = 'f0bgnuz'
isBoolean = function (aBoolean)
  local result = false

  if type(aBoolean) ~= 'nil' then
    -- The default case, presence of a value means it's true.
    result = true
    if     type(aBoolean) == 'boolean'  then result = aBoolean
    elseif type(aBoolean) == 'function' then result = aBoolean()
    elseif type(aBoolean) == 'number'   then result = (aBoolean ~= 0)
    elseif type(aBoolean) == 'string'   then
      if '' == aBoolean then
        result = false
      else
	if 1 == string.find(string.lower(aBoolean), '^[' .. isTrue  .. ']') then result = true end
	if 1 == string.find(string.lower(aBoolean), '^[' .. isFalse .. ']') then result = false end
      end
    end
  end
  return result
end


--[[ Thing Java package

matrix-RAD had Thing as the base class of everything.

Each "users session" (matrix-RAD term that came from Java
applets/servlets) has a ThingSpace, which is a tree that holds
everything else.  It holds the class cache, commands, loaded modules,
variables and their values, widgets and their states.  In matrix-RAD I
built BonsiaTree and LeafLike, for the old FDO system I built dumbtrees.

Other Thing things are -
    get/set	The getter and setter.
    number	No idea how this was useful.
    skang	The owning object, a Skang (actually got this, called module for now).
    owner	The owning object, a String (module._NAME).
    clas	Class of the Thing, a Class.  (pointless)
    type	Class of the Thing, a String.  (pointless)
    realType	Real Class of the Thing, a String.  (pointless)
    myRoot	ThingSpace we are in, a ThingSpace.

    Also various functions to wrap checking the security, like canDo, canRead, etc.
]]


--[[ Stuff Java package

In matrix-RAD Stuff took care of multi value Things, like database rows.

Stuff is an abstract class that gets extended by other classes, like
SquealStuff, which was the only thing extending it.  It dealt with the
basic "collection of things" stuff.  Each individual thing was called a
stufflet.  A final fooStuff would extend SquealStuff, and include an
array of strings called "stufflets" that at least named the stufflets,
but could also include metadata and links to other Stuffs.

There was various infrastructure for reading and writing Stuff, throwing
rows of Stuff into grids, having choices of Stuff, linking stufflets to
individual widgets, having default Stuffs for windows, validating
Stuffs, etc.

In Lua, setting up stuff has been folded into the general Thing stuff.

]]


--[[  Thing structure -

In the following, meta(table) is short for getmetatable(table).

In Lua everything is supposed to be a first class value, and variables are just places to store values.
All variables are in fact stored in tables, even if it's just the local environment table.
Any variable that has a value of nil, doesn't actually exist.  That's the definition.
While non table things can have metatables, Lua can only set metatables on tables, C has no such restriction.
meta(table).__index and __newindex only work on table entries that don't exist.
  __index(table, key)           is called if table.key is nil.
    Though if __index is a table, then try __index[key].
  __newindex(table, key, value) is called if table.key is nil.
    Though if __newindex is a table, then try __newindex[key] = value.
Using both __index and __newindex, and keeping the actual values elsewhere, is called a proxy table.
meta(table).__call(table, ...) is called when trying to access table as a function - table(...).

It's worth repeating -
All variables in Lua are in some table somewhere, even if it's just the global environment table.
Metatables are only associated vith values, not variables.
Lua can only attach metatables to values that are tables, but C can attach metatables to any value.


A Thing is a managed variable stored in a parent proxy table, which is usually empty.
So the values stored in this Thing are actually stored in meta(parent)__values[thing].
  parent[thing]          ->  __index   (parent, thing)         ->  meta(parent).__values[thing]
  parent[thing] = value  ->  __newindex(parent, thing, value)  ->  meta(parent).__values[thing] = value


Each Thing has a description table that includes -
  names		- An array of names, the first one is the "official" name.
  types		- An array of types, the first one is the "official" type.
  help		- A descriptive text for humans to read.
  default	- The default value.
  widget	- A default widget definition.
  required	- If the Thing is required.
  isValid	- A function that tests if the Thing is valid.
  errors	- Any errors related to the Thing.
  isKeyed	- Is this a parent for Things that are stored under an arbitrary key.
  stuff		- An array of descriptions for sub Things, so Things that are tables can have their own Things.
  and other things that aren't actually used yet.
All things that a description doesn't have should be inherited from the Thing table.
  setmetatable(aStuff, {__index = Thing})
Descriptions should be able to be easily shared by various Things.


A parent's metatable has __self, which is it's own description.
A parent is free to use it's own name space for anything it wants.
Only the variables it specifies as managed Things are dealt with here.


Things that are tables are treated differently, in two different ways even.
Ordinary table Things are basically treated recursively, the table is a parent, and it gets it's own Things.
There is also 'Keyed' type table Things, where the keys to this table are arbitrary, but we still want to store Things in it.
In this case, when a table is assigned to this Keyed Thing, via a new key, a new table Thing is created by copying the parent Thing description.


TODO - 
    test.foo  ->  test.__index(test, 'foo')  ->  test.__values[foo];  if that's nil, and test.stuff[foo], then return an empty table instead?
    Do we still need a parent pointer?
      Should be in __values I guess.
	__values[key].value
	__values[key].parent
    Weak references might help in here somewhere.
    Maybe try looking in the skang table for Things that are not found?
    Maybe put Things in the skang table that are unique from modules?
      I think this is what matrix-RAD Collisions was all about.
]]

-- There is no ThingSpace, or Stuff, now it's all just in this meta table.
local Thing =
{
-- Default Thing values.
  names = {'unknown'},
  help = 'No description supplied.',	-- help text describing this Thing.
  default = '',			-- The default value.  This could be a funcion, making this a command.
  types = {},			-- A list of types.  The first is the type of the Thing itself, the rest are for multi value Things.  Or argument types for commands.
  required = false,		-- Is this thing is required.  TODO - Maybe fold this into types somehow, or acl?
  widget = '',			-- Default widget command arguments for creating this Thing as a widget.
--  acl = '',			-- Access Control List defining security restrains.
--  boss = '',			-- The Thing or person that owns this Thing, otherwise it is self owned.

  action = 'nada',		-- An optional action to perform.
  tell = '',			-- The skang command that created this Thing.
  pattern = '.*',		-- A pattern to restrict values.

  isKeyed = false,		-- Is this thing an arbitrarily Keyed table?
  isReadOnly = false,		-- Is this Thing read only?
  isServer = false,		-- Is this Thing server side?
  isStub = false,		-- Is this Thing a stub?
  isStubbed = false,		-- Is this Thing stubbed elsewhere?

  hasCrashed = 0,		-- How many times this Thing has crashed.

  append = function (self,data)	-- Append to the value of this Thing.
  end,

  stuff = {},			-- The sub things this Thing has, for modules, tables, and Keyed tables.
  errors = {},			-- A list of errors returned by isValid().

  isValid = function (self, parent)	-- Check if this Thing is valid, return resulting error messages in errors.
    -- Anything that overrides this method, should call this super method first.
    local name = self.names[1]
    local metaMum = getmetatable(parent)
    local value = metaMum.__values[name]
    local mum = metaMum.__self.names[1]

    local t = type(value) or 'nil'
    self.errors = {}
    -- TODO - Naturally there should be formatting functions for stuffing Thing stuff into strings, and overridable output functions.
    if 'nil' == t then
      if self.required then table.insert(self.errors, mum .. '.' .. name .. ' is required!') end
    else
      if 'widget' == self.types[1] then
      -- TODO - Should validate any attached Thing.
      elseif self.types[1] ~= t then table.insert(self.errors, mum .. '.' .. name .. ' should be a ' .. self.types[1] .. ', but it is a ' .. t .. '!')
      else
        if 'number' == t then value = '' .. value end
        if ('number' == t) or ('string' == t) then
          if 1 ~= string.find(value, '^' .. self.pattern .. '$') then table.insert(self.errors, mum .. '.' .. name .. ' does not match pattern "' .. self.pattern .. '"!') end
        end
      end
    end

    for k, v in pairs(self.stuff) do
      if not v:isValid(value) then
        for i, w in ipairs(v.errors) do
          table.insert(self.errors,  w)
        end
      end
    end

    return #(self.errors) == 0
  end,

  remove = function (self)	-- Delete this Thing.
  end,
}


getStuffed = function (parent, key)
  local metaMum = getmetatable(parent)
  local thingy

  if metaMum and metaMum.__self then
    thingy = metaMum.__self.stuff[key]

    if not thingy then
      -- Deal with getting a table entry.
      if metaMum.__values[key] then
	thingy = getmetatable(metaMum.__values[key]).__self
      end
    end
  end
  return metaMum, thingy
end

local Mum = 
{

__index = function (parent, key)
  -- This only works for keys that don't exist.  By definition a value of nil means it doesn't exist.

  -- First see if this is a Thing.
  local metaMum, thingy = getStuffed(parent, key)

  if thingy then
    return metaMum.__values[thingy.names[1] ] or thingy.default
  end

  -- Then see if we can inherit it from Thing.
  return Thing[key]
end,

__newindex = function (parent, key, value)
  -- This only works for keys that don't exist.  By definition a value of nil means it doesn't exist.

  -- First see if this is a Thing.
  local metaMum, thingy = getStuffed(parent, key)

  if not thingy then
    if metaMum.__self.isKeyed then
      -- Deal with setting a new Keyed table entry.
      local newThing = copy(parent, key)
      local newSelf = getmetatable(newThing).__self
      rawset(metaMum.__values, key, newThing)
      thingy = {}
      for k, v in pairs(newSelf) do
        thingy[k] = v
      end
      thingy.names={key}
      thingy.types={'table'}
      setmetatable(thingy, {__index = Thing})	-- To pick up isValid, pattern, and the other stuff by default.
    end
  end

  if thingy then
    local name = thingy.names[1]
    local oldMum

    if 'table' == type(value) then
      -- Coz setting it via metaMum screws with the __index stuff somehow.
      local oldValue = metaMum.__values[name]
      if 'table' == type(oldValue) then
        oldMum = getmetatable(oldValue)
        if oldMum then
	  -- TODO - This SHOULD work, but doesn't -
          --setmetatable(value, oldMum)
          -- Instead we do this -
          -- Clear out any values in the old table.
          for k, v in pairs(oldMum.__values) do
            oldMum.__values[k] = nil
          end
          for k, v in pairs(value) do
            local newK = oldMum.__self.stuff[k]
            if newK then newK = newK.names[1] else newK = k end
            oldMum.__values[newK] = v
          end
        end
      end
    end
    if nil == oldMum then metaMum.__values[name] = value end
    -- NOTE - invalid values are still stored, this is by design.
    if not thingy:isValid(parent) then
      for i, v in ipairs(thingy.errors) do
        print('ERROR - ' .. v)
      end
    end
    -- TODO - Go through it's linked things and set them to.
    -- Done, don't fall through to the rawset()
    return
  end

  rawset(parent, key, value)		-- Stuff it normally.
end,

__call = function (func, ...)
  return thingasm(func, ...)
end,

}

newMum = function ()
  local result = {}
--[[ From an email by Mike Pall -
"Important: create the metatable and its metamethods once and reuse
the _same_ metatable for _every_ instance."

This is for LuaJIT, he's the author, and concerns performance.

TODO - Which is the exact opposite of what we are doing.  Perhaps we can fix that?
]]
  for k, v in pairs(Mum) do
    result[k] = v
  end
  result.__self = {stuff={}}
  result.__values = {}
  return result
end


-- skang.thingasm() Creates a new Thing, or changes an existing one.
--[[ It can be called in many different ways -

It can be called with positional arguments - (names, help, default, types, widget, required, acl, boss)
Or it can be called with a table           - {names, help, pattern='.*', acl='rwx'}

The first argument can be another Thing (the parent), or a string list of names (see below).

It can be called by itself, with no parent specified -
    thingasm('foo', 'help text)
In this case the surrounding Lua environment becomes the parent of foo.
   If the first argument (or first in the table) is a string, then it's this form.
All others include the parent as the first argument, which would be a table.

It can be called by calling the parent as a function -
    foo('bar', 'some help', types='table')	-- ___call(foo, 'bar', ...)  And now foo is the parent.
    foo.bar{'baz', types='Keyed'}		-- thingasm({foo.bar, 'baz', ...})
    foo.bar.baz{'field0'}			-- thingasm({foo.bar.baz, 'field0'})
    foo.bar.baz{'field1'}
]]

-- names	- a comma seperated list of names, aliases, and shortcuts.  The first one is the official name.
--                If this is not a new thing, then only the first one is used to look it up.
--                So to change names, use skang.thingasm{'oldName', names='newName,otherNewName'}
thingasm = function (names, ...)
  local params = {...}
  local new = false
  local parent
  local set = true

  -- Check how we were called, and re arrange stuff to match.
  if 0 == #params then
    if ('table' == type(names)) then			-- thingasm{...}
      params = names
      names = shiftLeft(params)
      if 'table' == type(names) then			-- thingasm{parent, 'foo', ...}
        parent = names
        names = shiftLeft(params)
      end
    end							-- thingasm("foo") otherwise
  else
    if 'table' == type(names) then
      parent = names
      if 'string' == type(...) then    params = {...}	-- C or __call(table, string, ..)
      elseif 'table' == type(...) then params = ...	-- __call(table, table)
      end
      names = shiftLeft(params)
    end							-- thingasm('foo', ...) otherwise
  end

  -- Break out the names.
  names = csv2table(names)
  local name = names[1]
  local oldNames = {}

  -- TODO - Double check this comment - No need to bitch and return if no names, this will crash for us.

  -- Grab the environment of the calling function if no parent was passed in.
  parent = parent or getfenv(2)
  local metaMum = getmetatable(parent)
  -- Coz at module creation time, Thing is an empty table, or in case this is for a new parent.
  if nil == metaMum then
    metaMum = newMum()
    metaMum.__self.names = {parent._NAME or 'NoName'}
    if parent._NAME then metaMum.__self.types = {'Module'} end
    setmetatable(parent, metaMum)
  end

  local thingy = metaMum.__self.stuff[name]
  if not thingy then				-- This is a new Thing.
    new = true
    thingy = {}
    thingy.names = names
    thingy.stuff = {}
    setmetatable(thingy, {__index = Thing})	-- To pick up isValid, pattern, and the other stuff by default.
  end

  -- Pull out positional arguments.
  thingy.help		= params[1] or thingy.help
  thingy.default	= params[2] or thingy.default
  local types		= params[3] or table.concat(thingy.types or {}, ',')

  -- Pull out named arguments.
  for k, v in pairs(params) do
    if 'string' == type(k) then
      if     'types' == k then types = v
      elseif 'names' == k then
        oldNames = thingy.names
        thingy.names = cvs2table(v)
      elseif 'required' == k then
        if isBoolean(v) then  thingy.required = true end
      else                  thingy[k] = v
      end
    end
  end

  -- Find type, default to string, then break out the other types.
  local typ = type(thingy.default)
  if 'nil' == typ then typ = 'string' end
  if 'function' == typ then types = typ .. ',' .. types end
  if '' == types then types = typ end
  thingy.types = csv2table(types)

  if 'widget' == thingy.types[1] then
    set = false
    local args, err = loadstring('return ' .. thingy.widget)
    if args then
      setfenv(args, parent)
      thingy.Cwidget = widget(parent.window, args())
--print('\nNO IDEA WHY this does isValid() three times on the action, and the first one being a string.')
      parent.W[name] = thingy
    else
      print("ERROR - " .. err)
    end
  end

  -- Deal with Keyed and tables.
  if 'Keyed' == thingy.types[1] then
    set = false
    thingy.types[1] = 'table'
    thingy.isKeyed = true
  end
  if 'table' == thingy.types[1] then
    -- Default in this case becomes a parent.
    if '' == thingy.default then thingy.default = {} end
    local thisMum = newMum()
    thisMum.__self = thingy
    setmetatable(thingy.default, thisMum)
  end

  if 'userdata' == thingy.types[1] then
    set = false
  end

  -- Remove old names, then stash the Thing under all of it's new names.
  for i, v in ipairs(oldNames) do
    metaMum.__self.stuff[v] = nil
  end
  for i, v in ipairs(thingy.names) do
    metaMum.__self.stuff[v] = thingy
  end

  -- This triggers the Mum.__newindex metamethod above.  If nothing else, it triggers thingy.isValid()
  if new and set then parent[name] = thingy.default end
end


fixNames = function (module, name)
  local stuff = getmetatable(module)
  if stuff then
    stuff.__self.names[1] = name
    for k, v in pairs(stuff.__self.stuff) do
      if 'table' == v.types[1] then
        local name = v.names[1]
        print(name .. ' -> ' .. name)
        fixNames(stuff.__values[name], name)
      end
    end
  end
end


copy = function (parent, name)
  local result = {}
  local stuff = getmetatable(parent).__self.stuff

  for k, v in pairs(stuff) do
    local temp = {}
    for l, w in pairs(v) do
      temp[l] = w
    end
    temp[1] = table.concat(temp.names, ',')
    temp.names = nil
    temp.types = table.concat(temp.types, ',')
    temp.errors = nil
    thingasm(result, temp)
  end
  getmetatable(result).__self.names = {name}

-- TODO - Should we copy values to?

  return result
end

module = function (name)
  _G[name] = require(name)
  return _G[name]
end

stuff = function (aThingy, aStuff)
  return getmetatable(aThingy).__self.stuff[aStuff]
end


get = function (stuff, key, name)
  local result
  if name then
    local thingy = getmetatable(stuff)
    if thingy then
      local this = thingy.__self.stuff[key]
      if this then result = this[name] end
    end
  else
    result = stuff[key]
  end
  return result
end


reset = function (stuff, key, name)
  if name then
    local thingy = getmetatable(stuff)
    if thingy then
      local this = thingy.__self.stuff[key]
      if this then this[name] = nil end
    end
  else
    stuff[key] = nil
  end
end


set = function (stuff, key, name, value)
  if 'nil' ~= type(value) then
    local thingy = getmetatable(stuff)
    if thingy then
      local this = thingy.__self.stuff[key]
      if this then this[name] = value end
    end
  else
    -- In this case, value isn't there, so we are reusing the third argument as the value.
    stuff[key] = name
  end
end


-- Get our C functions installed into skang.
-- This has to be after thingasm is defined.
local GuiLua = require 'GuiLua'


thingasm('module,l',	'Load a module.',				module, 'file')
thingasm('get',		'Get the current value of an existing Thing or metadata.',	get,	'thing,key,name')
thingasm('reset',	'Reset the current value of an existing Thing or metadata.',	reset,	'thing,key,name')
thingasm('set',		'Set the current value of an existing Thing or metadata.',	set,	'thing,key,name,data')

thingasm('nada',	'Do nothing.',	function () --[[ This function intentionally left blank. ]] end)



-- Widget wrappers.
-- TODO - Fix this up so we don't need .W
local widgets = {}
--thingasm{widgets, 'window', 'The window.', types='userdata'}
thingasm{widgets, 'W', 'Holds all the widgets', types='Keyed'}
widgets.W{'Cwidget', 'The widget.', types='userdata'}
widgets.W{'action,a', 'The action for the widget.', 'nada()', types='string'}
widgets.W{'text,t', 'The text for the widget.', '', types='string'}
local aIsValid = function (self, parent)
  local result = Thing.isValid(self, parent)

  if result then
    local value = parent[self.names[1] ]
    if 'userdata' == type(parent.Cwidget) then
--print('NEW ACTION - ' .. self.names[1] .. ' = ' .. value .. '   ' .. type(parent.Cwidget))
      if ('action' == self.names[1]) and ('nada()' ~= value) then  action(parent.Cwidget, value) end
      if ('text'   == self.names[1]) and (''       ~= value) then  text  (parent.Cwidget, value) end
    end
  end
  return result
end

widgets.W{'look,l', 'The look of the widget.', types='string'}
--[[
widgets.W{'colour,color,c', 'The  colours of the widget.', types='table'}
widgets.W.c{'red,r',   'The red   colour  of the widget.', 255, types='number'}
widgets.W.c{'green,g', 'The green colour  of the widget.', 255, types='number'}
widgets.W.c{'blue,b',  'The blue  colour  of the widget.', 255, types='number'}
widgets.W.c{'alpha,a', 'The alpha colour  of the widget.', 255, types='number'}
-- At this point we want to change widgets.W.c() to go to a different __call, coz right now it's going to the Mum.__call, which wraps thingasm.
-- TODO - Keep an eye on this if we change to a single Mum, instead of the shallow copy we use now.
local wMum = getmetatable(widgets.W.c)
wMum.__call = function (func, ...)
  return Colour(func, ...)
end
]]

--[[  TODO - Widgets doing the right thing, in the right place.

GOAL - GuiLua.c -> _on_click()  -> lauL_dostring(L, wid->action)
       We want this action to be performed in the environment of the module that created the widget.
       Or the one that set the action.
       NOTE - Can't refer to local variables from within actions.

  purkle
    local purkle = require 'purkle'
    local win = skang.window(....
      win = copy(widgets ....
      win.window = Cwindow( ....
        lightuserdata *winFang
    skang.thingasm{win, 'button1' ....  types = 'widget', widget='"button", ....
      args = loadstring('"button", ....
      setfenv(args, win)
      thingy.Cwidget = widget(win.window, args())
        winFang *(win.window)
        Widget->data = L
        Widget->obj "clicked" callback = _on_click, Widget *
        lightuserdata *widget
      parent.W[name] = thingy
    win.W.button1.action = 'purkle.say(' ....
      aIsValid(....
        action(button1, ....
          Widget->action = ....

Store the name of the module that created a window in winFang.
Store the winFang a widget is in in Widget.
]]

window = function(w, h, title, name)
  local caller = getfenv(2)._NAME
  name = name or 'myWindow'
  local win = {}
  win = copy(widgets, name)
  local wMum, wThingy = getStuffed(win.W, 'a')
  wThingy.isValid = aIsValid
  local wMum, wThingy = getStuffed(win.W, 't')
  wThingy.isValid = aIsValid
  win.window = Cwindow(caller, w, h, title, name)
  return win
end

thingasm{'window',	'Specifies the size and title of the application Frame.', window, 'number,number,string', acl="GGG"}


-- TODO - Some function stubs, for now.  Fill them up later.
skang = function (name)
end

thingasm('skang',	'Parse the contents of a skang file or URL.',	skang,	'URL')


moduleEnd(_M)

end

-- Boss is the person that owns a Thing.

--[[  The original Skang parameters and commands.
	public final static String MY_USAGE[][] = 
	{
		{"skinURL", "skinURL", "Y", "s", null, "URL of skin file.", "", "RI-"},
		{"debug", "debug", "N", "", "0", "Set debugging level to :\n\t-1 - errors and warnings only (-q)\n\t0 - basic information\n\t1 - advanced information (-v)\n\t2 - trace functions\n\t3 - trace protocol\n\t4 - dump packets + stuff\n\t5 - detail", "", ""},
		{"browser", "browser", "N", "", "mozilla %f", "Browser to run.", "", ""},
		{"downloaddir", "downloadDir", "N", "", "download", "Download directory.", "", ""},
		{"sessionID", "sessionID", "N", "", null, "SessionID from servlet.", "", ""},
		{"JSESSIONID", "JSESSIONID", "N", "", null, "JSESSIONID from servlet engine.", "", ""},
		{"servletpath", "servletPath", "N", "", "matrix_rad", "Servlet path.", "", ""},
		{"servletport", "servletPort", "N", "", null, "Servlet port.", "", ""},
		{"servletsslport", "servletSSLPort", "N", "", null, "Servlet SSL port.", "", ""},
		{"HTML", "HTML", "N", "", "false", "Output to HTML?", "", ""},
		{"PHP", "PHP", "N", "", "false", "Output though the PHP wrapper", "", ""},
		{"inbrowser", "inBrowser", "N", "", "true", "Run in browser window?", "", ""},
		{"SSL", "SSL", "N", "", null, "Dummy to avoid a web server bug.", "", ""},
		{"NOSSL", "NOSSL", "N", "", null, "Dummy to avoid a web server bug.", "", ""},
		{"corporate", "corporate", "N", "", null, "Are we doing corporate shit?", "", ""},
		{"", "", "", "", "", "", "", ""}
	};
	public final static String MY_SKANG[][] = 
	{
--		{"module", "addModule", "file,data", "Load a module.", "", ""},
		{"append", "appendThing", "name,data", "Append to the current value of a Thing.", "", ""},
		{"#!java", "bash", "name,name,name,name,name,name,name", "A not so clever unix script compatability hack.", "", ""},
		{"pending", "pendingDoThing", "action", "Do an action when you are ready.", "", ""},
		{"applet", "doIfApplet", "action", "Only do this if we are an applet.", "", ""},
		{"application", "doIfApplication", "action", "Only do this if we are an application.", "", ""},
		{"corporateshit", "doIfCorporateShit", "action", "Only do this if we are doing corporate shit.", "", ""},
		{"realworld", "doIfRealWorld", "action", "Only do this if we are in the real world.", "", ""},
		{"servlet", "doIfServlet", "action", "Only do this if we are a servlet.", "", ""},
		{"do", "doThing", "action", "Do this action.", "", ""},
		{"grab", "getFile", "URL", "Grab a file from a URL.", "", ""},
--		{"get", "getThing", "name", "Get the current value of an existing thing.", "", ""},
		{"gimmeskin", "gimmeSkin", "", "Returns the modules default skin.", "", ""},
		{"help", "helpThing", "file", "Show help page.", "", ""},
--		{"nada", "nothing", "data", "Does nothing B-).", "", ""},
		{"postshow", "postShowThings", "URL,name", "POST the values of all Things to the URL, show the returned content.", "", ""},
		{"post", "postThings", "URL", "POST the values of all Things to the URL, return the content.", "", ""},
		{"postparse", "postParseThings", "URL", "POST the values of all Things to the URL, parse the returned content.", "", ""},
		{"quiet", "quiet", "", "Output errors and warnings only.", "", ""},
		{"remove", "removeThing", "name", "Remove an existing thing.", "", ""},
		{"sethelp", "setHelp", "name,data", "Change the help for something.", "", ""},
--		{"set", "setThing", "name,data", "Set the current value of an existing Thing.", "", ""},
--		{"skang", "skangRead", "URL", "Parse the contents of a skang file or URL.", "", ""},
--		{"quit", "startQuit", "", "Quit, exit, remove thyself.", "", ""},
		{"stopwhinging", "stopWhinging", "", "Clear all messages.", "", ""},
		{"tell", "tellThing", "name", "Returns details of an existing Thing.", "", ""},
		{"togglebug", "toggleIgnoreBrowserBug", "", "Toggle ignorance of a certain browser bug.", "", ""},
		{"verbose", "verbose", "", "Output advanced information.", "", ""},
		{"", "", "", "", "", ""}
]]

--[[ The original SkangAWT parameters and commands.
	public final static String MY_USAGE[][] = 
	{
		{"", "", "", "", "", "", "", ""}
	};
	public final static String MY_SKANG[][] = 
	{
		{"taction", "tactionWidget", "name,action", "Set the alternative action for a widget.", "", ""},
		{"action", "actionWidget", "name,action", "Set the action for a widget.", "", ""},
		{"pane", "addPane", "name,x,y,w,h,data", "Add a pane to the current module.", "", ""},
		{"widget", "addWidget", "name,type,lx,ly,lw,lh,data,data", "Add a widget to the current skin.", "", ""},
		{"checkboxgroup", "checkBoxGroup", "number", "Make the next 'number' Checkboxes part of a check box group.", "", ""},
--		{"clear", "clearWidgets", "", "The current skin is cleared of all widgets.", "", ""},
		{"colour", "colourWidget", "name,r,g,b,alpha,r,g,b,alpha", "Set widget's background and foreground colour.", "", "GGG"},
		{"doaction", "doWidget", "name", "Do a widgets action.", "", "GGG"},
		{"disable", "disableWidget", "name", "Disable a widget.", "", "GGG"},
		{"enable", "enableWidget", "name", "Enable a widget.", "", "GGG"},
		{"hide", "hideWidget", "name", "Hide a widget.", "", "GGG"},
		{"hideall", "hideAllWidgets", "name,lx,ly,lw,lh", "Hide all widgets.", "", "GGG"},
		{"look", "lookWidget", "name,normal,ghost,active,toggle", "Set the current look of an existing widget.", "", "GGG"},
		{"mask", "maskWidget", "name,data", "Set the mask for a widget.", "", ""},
		{"onmouse", "onMouse", "name,data", "Do something on mouse hover.", "", ""},
		{"offmouse", "offMouse", "name,data", "Do something off mouse hover.", "", ""},
		{"popup", "popupWidget", "name,data,data,data,data", "Create a popup.", "", "GGG"},
		{"readonly", "readOnlyWidget", "name", "Make a widget read only.", "", "GGG"},
		{"writeonly", "writeOnlyWidget", "name", "Make a widget write only.", "", "GGG"},
		{"satori", "satori", "x,y", "Give me the developers menu.", "", "GGG"},
		{"showloginwindow", "showLoginWindow", "", "Show user login window.", "", "GGG"},
		{"show", "showWidget", "name", "Show a widget.", "", "GGG"},
--		{"window", "setSkangFrame", "x,y,name", "Specifies the size and title of the application Frame.", "", "GGG"},
		{"stuff", "stuffWidget", "name,data", "Set the stuff for a widget's pane.", "", ""},
		{"stufflet", "stuffWidget", "name,data,data", "Set the stufflet for a widget.", "", ""},
		{"stufflist", "stuffListWidget", "name,data", "List the stuff in this widget.", "", ""},
		{"stuffload", "stuffLoadWidget", "name,data,data", "Load the stuff for a widget.", "", ""},
		{"stuffsave", "stuffSaveWidget", "name,data,data", "Save the stuff for a widget.", "", ""},
		{"stuffdelete", "stuffDeleteWidget", "name,data,data", "Delete the stuff for a widget.", "", "SSS"},
		{"stuffclear", "stuffClearWidget", "name,data", "Clear the stuff for a widget.", "", "SSS"},
		{"rowtowidgets", "rowToWidgets", "name", "Copy Grid row to matching widgets.", "", ""},
		{"widgetstorow", "widgetsToRow", "name,data", "Copy matching widgets to Grid row.", "", ""},
		{"clearrow", "clearRow", "name", "Clear Grid row and matching widgets.", "", ""},
		{"clearrowwidgets", "clearRowWidgets", "name", "Clear only the Grid row matching widgets.", "", ""},
		{"", "", "", "", "", ""}
	};
]]


--[[ security package

Java skang could run as a stand alone applicion, as an applet in a web
page, or as a servlet on a web server.  This was pretty much all
transparent to the user.  The security system reflected that.  Lua skang
wont run in web pages, but can still have client / server behaviour. 
The general idea was, and still is, that the GUI is the client side (in
web page, in extantz GUI) that sends values back to the server side
(servlet, actual Lua package running as a separate process, or the world
server for in world scripts).  Client side can request that server side
runs commands.  Serevr side can send values and commands back to the
client.  Mostly it all happenes automatically through the ACLs.

Bouncer is the Java skang security manager, it extended the Java
SecurityManager.  Lua has no such thing, though C code running stuff in
a sandbox does a similar job.  Fascist is the Java security supervisor,
again should go into the C sandbox.

Human is used for authenticating a human, Puter for authenticating a
computer, Suits for corporate style authentication, and they all
extended Who, the base authentication module.

For now, I have no idea how this all translates into Lua, but putting
this here for a reminder to think about security during the design
stage.


This is the old Java ACL definition -
	acl - access control list.
Owner is usually the person running the Thingspace.
RWX~,---,Rwxgroup1,r--group2,r-xgroup3,rw-group4,--X~user1
rwx~ is for the owner.  The second one is the default.  The rest are per group or per user.
Capital letters mean that they get access from the network to.
--- No access at all.
RWX Full access.
R-- Read only access.
r-x Read and execute, but only locally.
rw- Read and write a field, but don't execute a method.
-w- A password.
-a- An append only log file.
-A- An append only log file on the server.
Ri- read, but only set from init (ei. skinURL not set from properties or skang files).
RI- As above, but applet.init() can set it too.
--x Thing is both method and field, only execution of the method is allowed.
--p Run as owner (Pretend).
--P Run across the network as owner (can run in applet triggered by server).
s-- Read only, but not even visible to applets.
sss Only visible to servlets and applications.
--S Send to servlet to execute if applet, otherwise execute normally.
S-- Read only, but ignore local version and get it from server.
ggg GUI Thing, only visible to Applets and applications.
GGG GUI Thing, but servlets can access them across the net.

For servlet only modules from an applet, the applet only loads the skanglet class, using it for all
access to the module.


Lua Security best practices -

  From an email by Mike Pall -

"The only reasonably safe way to run untrusted/malicious Lua scripts is
to sandbox it at the process level. Everything else has far too many
loopholes."

http://lua-users.org/lists/lua-l/2011-02/msg01595.html
http://lua-users.org/lists/lua-l/2011-02/msg01609.html
http://lua-users.org/lists/lua-l/2011-02/msg01097.html
http://lua-users.org/lists/lua-l/2011-02/msg01106.html

So that's processes, not threads like LuaProc does.  B-(

However, security in depth is good, so still worthwhile looking at it from other levels as well.

General solution is to create a new environment, which we are doing
anyway, then whitelist the safe stuff into it, instead of blacklisting
unsafe stuff.  Coz you never know if new unsafe stuff might exist.

Different between 5.1 (setfenv) and 5.2 (_ENV)

http://lua-users.org/wiki/SandBoxes -

------------------------------------------------------
-- make environment
local env =  -- add functions you know are safe here
{
  ipairs = ipairs,
  next = next,
  pairs = pairs,
  pcall = pcall,
  tonumber = tonumber,
  tostring = tostring,
  type = type,
  unpack = unpack,
  coroutine = { create = coroutine.create, resume = coroutine.resume, 
      running = coroutine.running, status = coroutine.status, 
      wrap = coroutine.wrap },
  string = { byte = string.byte, char = string.char, find = string.find, 
      format = string.format, gmatch = string.gmatch, gsub = string.gsub, 
      len = string.len, lower = string.lower, match = string.match, 
      rep = string.rep, reverse = string.reverse, sub = string.sub, 
      upper = string.upper },
  table = { insert = table.insert, maxn = table.maxn, remove = table.remove, 
      sort = table.sort },
  math = { abs = math.abs, acos = math.acos, asin = math.asin, 
      atan = math.atan, atan2 = math.atan2, ceil = math.ceil, cos = math.cos, 
      cosh = math.cosh, deg = math.deg, exp = math.exp, floor = math.floor, 
      fmod = math.fmod, frexp = math.frexp, huge = math.huge, 
      ldexp = math.ldexp, log = math.log, log10 = math.log10, max = math.max, 
      min = math.min, modf = math.modf, pi = math.pi, pow = math.pow, 
      rad = math.rad, random = math.random, sin = math.sin, sinh = math.sinh, 
      sqrt = math.sqrt, tan = math.tan, tanh = math.tanh },
  os = { clock = os.clock, difftime = os.difftime, time = os.time },
}

-- run code under environment [Lua 5.1]
local function run(untrusted_code)
  if untrusted_code:byte(1) == 27 then return nil, "binary bytecode prohibited" end
  local untrusted_function, message = loadstring(untrusted_code)
  if not untrusted_function then return nil, message end
  setfenv(untrusted_function, env)
  return pcall(untrusted_function)
end

-- run code under environment [Lua 5.2]
local function run(untrusted_code)
  local untrusted_function, message = load(untrusted_code, nil, 't', env)
  if not untrusted_function then return nil, message end
  return pcall(untrusted_function)
end
------------------------------------------------------

Also includes a table of safe / unsafe stuff.


While whitelisting stuff, could also wrap unsafe stuff to make them more safe.

print()		-> should reroute to our output widgets.
rawget/set()	-> don't bypass the metatables, but gets tricky and recursive.
require		-> don't allow bypassing the sandbox to get access to restricted modules
package.loaded	-> ditto


Other things to do -

debug.sethook() can be used to call a hook every X lines, which can help with endless loops and such.
Have a custom memory allocater, like edje_lua2 does.

------------------------------------------------------
------------------------------------------------------

The plan -

  Process level -
    Have a Lua script runner C program / library.
    It does the LuaProc thing, and the edje_lua2 memory allocater thing.
    Other code feeds scripts to it via a pipe.
      Unless they are using this as a library.
    It can be chrooted, ulimited, LXC containered, etc.
    It has an internal watchdog thread.
    The truly paranoid can have a watchdog timer process watch it.
      Watches for a "new Lua state pulled off the queue" signal.
      This could be done from the App that spawned it.

    It runs a bunch of worker threads, with a queue of ready Lua states.
    Each Lua state being run has lua_sethook() set to run each X lines, AND a watchdog timer set.
      If either is hit, then the Lua state is put back on the queue.
      (Currently LuaProc states go back an the queue when waiting for a "channel message", which does a lua_yeild().)
      NOTE - apparently "compiled code" bypasses hooks, though there's an undocumented LuaJIT compile switch for that.  http://lua-users.org/lists/lua-l/2011-02/msg01106.html
      EFL is event based.
      LSL is event based.
      LuaSL is event based.
      Java skang is event based, with anything that has long running stuff overriding runBit().
        Coz Java AWT is event based, the "events" are over ridden methods in each widget class.
      Should all work if we keep this as event based.
      An "event" is a bit of Lua script in a string, at Input trust level usually.
    Configurable for this script runner is -
      IP address & port, or pipe name.
      Security level to run at, defaults to Network.
      Number of worker threads, defaults to number of CPUs.
      Memory limit per Lua state.
      Lines & time per tick for Lua states.

  Different levels of script trust -
    System	-  the local skang and similar stuff.
		-> No security at all.
    App		-  Lua scripts and C from the running application.
		-> Current module level security.
		    Each has it's own environment, with no cross environment leakage.
		    Runs in the Apps process, though that might be the script runner as a library.
		    Or could be skang's main loop.
    Local	-  Lua scripts and skang files sent from the client apps running on the same system.
		-> As per App.
		    Runs in a script runner, maybe?  Or just the Apps script runner.
		    Limit OS and file stuff, the client app can do those itself.
    Network	-  Lua scripts and skang files sent from the network.
		-> Runs in a script runner.
		    Option to chroot it, ulimit it, etc.
		    Heavily Lua sandboxed via environment.
		    It can access nails, but via network derived credentials.
		    Can have very limited local storage, not direct file access though, but via some sort of security layer.
		    Can have network access.
		    Can have GUI access, but only to it's own window.
    Config	-  Lua scripts run as configuration files.
		-> Almost empty local environment, can really only do math and set Things.
    Input	-  Lua scripts run as a result of hitting skang widgets on the other end of a socket.
		-> As per Config, but can include function calls.
		   Also would need sanitizing, as this can come from the network.
    Microsoft	-  Not to be trusted at all.
    Apple	-  Don't expect them to trust us.

  NOTE - edje_lua2 would be roughly Local trust level.

  So we need to be able to pass Lua between processes -
    Have to be able to do it from C, from Lua, and from Lua embedded in C.
    edje_lua2	- uses Edje messages / signals.
    LuaSL	- uses Ecore_Con, in this case a TCP port, even though it's only local.
		    LuaSL mainloop for it's scripts is to basically wait for these messages from LuaProc.
		    Which yield's until we get one.
		    Eventually gets Lua code as a string -> loadstring() -> setfenv() -> pcall()
		    The pcall returns a string that we send back as a LuaProc message.
    Extantz	- we want to use stdin/stdout for the pipe, but otherwise LuaSL style semantics.

  Hmm, Extantz could run external skang modules in two ways -
    Run the module as a separate app, with stdin/stdout.
    Require the module, and run it internally.
  Stuff like the in world editor and radar would be better as module, since they are useless outside Extantz?
    Radar is useless outside Extantz, editor could be used to edit a local file.
  Stuff like woMan would be better off as a separate application, so it can start and stop extantz.

]]


--[[
The main loop.
  A Lua skang module is basically a table, with skang managed fields.
  Once it calls moduleEnd(), it's expected to have finished.
    test.lua is currently an exception, it runs test stuff afterwards.
  So their code just registers Things and local variables.
  Some of those Things might be functions for manipulating internal module state.
    A C module has it's variables managed outside of it by Lua.
    So would have to go via LUA_GLOBALSINDEX to get to them.
    Unless they are userdata, which are C things anyway.
    Though it's functions are obviously inside the C module.
  Normal Lua module semantics expect them to return to the require call after setting themselves up.
    So test.lua is really an aberation.

  Soooo, where does the main loop go?
  The skang module itself could define the main loop.
  Should not actually call it though, coz skang is itself a module.

  In Java we had different entry points depending on how it was called.
  If it was called as an applet or application, it got it's own thread with a mainloop.
    That main loop just slept and called runBit() on all registered modules.
  If it was loaded as a module, it bypassed that stuff.
  I don't think Lua provides any such mechanism.
  In theory, the first call to moduleBegin would be the one that was started as an application.
  So we could capture that, and test against it in moduleEnd to find when THAT module finally got to the end.
  THEN we can start the main loop (test.lua being the exception that fucks this up).
    Though test.lua could include a runBits() that quits the main loop, then starts the main loop at the very end once more?
  The problem with this is applications that require skang type modules.
    The first one they require() isn't going to return.
  Eventually skang itself will get loaded.  It looks at the "arg" global, which SHOULD be the command line.
  Including the name of the application, which we could use to double check.
    Extantz / script runner would set this arg global itself.

  Skang applications have one main loop per app.
  Skang modules use the main loop of what ever app called them.
  Non skang apps have the option of calling skangs main loop.
  Extantz starts many external skang apps, which each get their own main loop.
  Extantz has it's own Ecore main loop.
  LuaSL still has one main loop per script.
  LSL scripts get their own main loop, but LSL is stupid and doesn't have any real "module" type stuff.

What does skang main loop actually do?
  In Java it just slept for a bit, then called runBit() from each module, and the only module that had one was watch.
  Actually better off using Ecore timers for that sort of thing.
  Skang main loop has nothing to do?  lol
  Well, except for the "wait for LuaProc channel messages" thing.
  Widget main loop would be Ecore's main loop.

  Extantz loads a skang module.
    Module runs in extantz script runner.
    Module runs luaproc message main loop from skang.
    Evas / Elm widgets send signals, call C callbacks.
    Extantz sends Lua input scripts via luaproc messages to module.
  Extantz runs separate skang module.
    Module runs in it's own process.
    Module runs it's own message loop on the end of stdin.
    Evas / Elm widgets send signals, call C callbacks.
    Extantz sends Lua Input scripts to module via stdout.
  Module runs stand alone.
    Module runs in it's own process.
    Module has to have widget start Ecore's main loop.
    Module runs it's own message loop, waiting for widget to send it messages.
    Evas / Elm widgets send signals, call C callbacks.
    Widget sends Lua Input scripts to module.

Alternate plan -
  Skang has no main loop, modules are just tables.
    OK, so sometimes skang has to start up an Ecore main loop.
      With either Ecore_Con, or Evas and Elm.
  skang.message(string)
    Call it to pass a bit of Lua to skang, which is likely Input.
  Widget twiddles happen in Ecore's main loop, via signals and call backs.
  Eventually these call backs hit the widgets action string.
  Send that action string to skang.message().

  Extantz loads a skang module.
    Extantz has a skang Lua state.
    Module is just another table in skang.
    Widget -> callback -> action string -> skang.message()
  Extantz runs separate skang module.
    Skang module C code runs an Ecore main loop.
    Module is just another table in skang.
    Skang C uses Ecore_Con to get messages from Extantz' Ecore_Con.
    Widget -> callback -> action string -> Ecore_Con -> skang.message()
   OOORRRR ....
    Skang runs a main loop reading stdin.
    Widget -> callback -> action string -> stdout -> skang.message()
  Module runs stand alone.
    Skang module C code runs an Ecore main loop.
    Module is just another table in skang.
    Widget -> callback -> action string -> skang.message()
  Extantz loads a skang module from the network.
    Skang module runs on the server with it's own Ecore main loop somehow.
    Module is just another table in skang.
    Extantz uses Ecore_Con to get messages from Extantz' Ecore_Con, via TCP.
    Widget -> callback -> action string -> Ecore_Con -> skang.message()
   OOORRRR ....
    Remember, these are untrusted, so that's what the script runner is for.
    Skang module runs in the script runner, with some sort of luaproc interface.
    Widget -> callback -> action string -> Ecore_Con -> luaproc -> skang.message()

  Skang running as a web server.
    Skang runs an Ecore main loop.
    Skang has an Ecore_Con server attached to port 80.
    Skang loads modules as usual.
    Skang responds to specific URLs with HTML forms generated from Skang files.
    Skang gets post data back, which kinda looks like Input.  B-)

  Extantz runs a LuaSL / LSL script from the network.
    Send this one to the LuaSL script runner.
    Coz LSL scripts tend to have lots of copies with the same name in different objects.
    Could get too huge to deal with via "just another table".
    In this case, compiling the LSL might be needed to.

]]


--[[ TODO
  NOTE that skang.thingasm{} doesn't care what other names you pass in, they all get assigned to the Thing.


  Widget -
    Should include functions for actually dealing with widgets, plus a way
    of creating widgets via introspection.  Should also allow access to
    widget internals via table access.  Lua code could look like this -

    foo = widget('label', 0, "0.1", 0.5, 0, 'Text goes here :")
    -- Method style.
    foo:colour(255, 255, 255, 0, 0, 100, 255, 0)
    foo:hide()
    foo:action("skang.load(some/skang/file.skang)")
    -- Table style.
    foo.action = "skang.load('some/skang/file.skang')"
    foo.colour.r = 123
    foo.look('some/edje/file/somewhere.edj')
    foo.help = 'This is a widget for labelling some foo.'

    Widgets get a type as well, which would be label, button, edit, grid, etc.
    A grid could even have sub types - grid,number,string,button,date.
    A required widget might mean that the window HAS to have one.
    Default for a widget could be the default creation arguments - '"Press me", 1, 1, 10, 50'.

    skang.thingasm{'myButton', types='Button', rectangle={1, 1, 10, 50}, title='Press me', ...}

	skang.thingasm('foo,s,fooAlias', 'Foo is a bar, not the drinking type.', function () print('foo') end, '', '"button", "The foo :"' 1, 1, 10, 50')
	myButton = skang.widget('foo')	-- Gets the default widget creation arguments.
	myButton:colour(1, 2, 3, 4)
	-- Use generic positional / named arguments for widget to, then we can do -
	myEditor = skang.widget{'foo', "edit", "Edit foo :", 5, 15, 10, 100, look='edit.edj', colour={blue=20}, action='...'}
	-- Using the Thing alias stuff, maybe we can do the "first stage tokenise" step after all -
	myEditor = skang.widget{'foo', "edit", "Edit foo :", 5, 15, 10, 100, l='edit.edj', c={b=20}, a='...'}
	myEditor:colour(1, 2, 3, 4, 5, 6, 7, 8)
	myButton = 'Not default'	-- myEditor and foo change to.  Though now foo is a command, not a parameter, so maybe don't change that.
	-- Though the 'quit' Thing could have a function that does quitting, this is just an example of NOT linking to a Thing.
	-- If we had linked to this theoretical 'quit' Thing, then pushing that Quit button would invoke it's Thing function.
	quitter = skang.widget(nil, 'button', 'Quit', 0.5, 0.5, 0.5, 0.5)
	quitter:action('quit')

    For widgets with "rows", which was handled by Stuff in skang, we could
    maybe use the Lua concat operator via metatable.  I think that works by
    having the widget (a table) on one side of the concat or the other, and
    the metatable function gets passed left and right sides, then must
    return the result.  Needs some experimentation, but it might look like
    this -

    this.bar = this.bar .. 'new choice'
    this.bar = 'new first choice' .. this.bar


    coordinates and sizes -

    Originally skang differentiated between pixels and character cells,
    using plain integers to represent pixels, and _123 to represent
    character cells.  The skang TODO wanted to expand that to percentages
    and relative numbers.  We can't use _123 in Lua, so some other method
    needs to be used.  Should include those TODO items in this new design.

    Specifying character cells should be done as strings - "123"

    Percentages can be done as small floating point numbers between 0 and 1,
    which is similar to Edje.  Since Lua only has a floating point number
    type, both 0 and 1 should still represent pixels / character cells -

    0.1, 0.5, "0.2", "0.9"

    Relative numbers could be done as strings, with the widget to be
    relative to, a + or -, then the number.  This still leaves the problem
    of telling if the number is pixels or character cells.  Also, relative
    to what part of the other widget?  Some more thought needs to be put
    into this.

    Another idea for relative numbers could be to have a coord object with
    various methods, so we could have something like -

    button:bottom(-10):right(5)	-- 10 pixels below the bottom of button, 5 pixels to the right of the right edge of button.
    button:width("12")		-- 12 characters longer than the width of button.

    But then how do we store that so that things move when the thing they are
    relative to moves?


   Squeal -
     Squeal was the database driver interface for SquealStuff, the database
     version of Stuff.  Maybe we could wrap esskyuehl?  Not really in need of
     database stuff for now, but should keep it in mind.
     For SquealStuff, the metadata would be read from the SQL database automatically.

     squeal.database('db', 'host', 'someDb', 'user', 'password')  ->  Should return a Squeal Thing.
       local db = require 'someDbThing'	  ->  Same as above, only the database details are encodode in the someDbThing source, OR come from someDbThing.properties.
     db:getTable('stuff', 'someTable')	  ->  Grabs the metadata, but not the rows.
     db:read('stuff', 'select * from someTable'}  ->  Fills stuff up with several rows, including setting up the metadata for the columns.
       stuff[1].field1                    ->  Is a Thing, with a proper metatable, that was created automatically from the database meta data.
     stuff:read('someIndex')		  ->  Grabs a single row that has the key 'someIndex', or perhaps multiple rows since this might have SQL under it.
       stuff = db:read('stuff', 'select * from someTable where key='someIndex')

     stuff:write()			  ->  Write all rows to the database table.
     stuff:write(1)			  ->  Write one row  to the database table.
     stuff:stuff('field1').isValid = someFunction	-- Should work, all stuff[key] share the same Thing description.
     stuff:isValid(db)			  ->  Validate the entire Thing against it's metadata at least.
     window.stuff = stuff		  ->  Window gets stuff as it's default 'Keyed' table, any widgets with same names as the table fields get linked.
     grid.stuff   = stuff		  ->  Replace contents of this grid widget with data from all the rows in stuff.
     choice.stuff = stuff		  ->  As in grid, but only using the keys.
     widget.stuff = stuff:stuff('field1')	  ->  This widget gets a particular stufflet.
       widget would have to look up getmetatable(window.stuff).parent.  Or maybe this should work some other way?

     In all these cases above, stuff is a 'Keyed' table that has a Thing metatable, so it has sub Things.
       Should include some way of specifyings details like key name, where string, etc.
         getmetatable(stuff).__keyName
         getmetatable(stuff).__squeal.where
       And a way to link this database table to others, via the key of the other, as a field in this Stuff.
         stuff:stuff('field0').__link = {parent, key, index}
       In Java we had this -

public class PersonStuff extends SquealStuff
{

...

	public final static String FULLNAME = "fullname";

	public static final String keyField = "ID";       // Name of key field/s.
	public static final String where    = keyField + "='%k'";
	public static final String listName = "last";
	public static final String tables   = "PEOPLE";
	public static final String select   = null;
	public static final String stufflets[] =
	{
		keyField,
		"PASSWD_ID|net.matrix_rad.squeal.PasswdStuff|,OTHER",
		"QUALIFICATION_IDS|net.matrix_rad.people.QualificationStuff|,OTHER",
		"INTERESTING_IDS|net.matrix_rad.people.InterestingStuff|,OTHER",
		"title",
		"first",
		"middle",
		"last",
		"suffix",

...

		FULLNAME + "||,VARCHAR,512"
	};
}

]]


-- Gotta check out this _ENV thing, 5.2 only.  Seems to replace the need for setfenv().  Seems like setfenv should do what we want, and is more backward compatible.
--   "_ENV is not supported directly in 5.1, so its use can prevent a module from remaining compatible with 5.1.
--   Maybe you can simulate _ENV with setfenv and trapping gets/sets to it via __index/__newindex metamethods, or just avoid _ENV."
--   LuaJIT doesn't support _ENV anyway.
