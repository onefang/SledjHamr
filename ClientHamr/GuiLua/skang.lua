-- TODO - This should be in C, but so far development has been quite rapid doing it in Lua.

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


-- There is no ThingSpace, or Stuff, now it's all just in this meta table.  Predefined here coz moduleBegin references Thing.
local Thing = {}


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

  -- Save the callers environment.
  local savedEnvironment = getfenv(level)

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
  -- TODO - If there's no skin passed in, try to find the file skin .. '.skang' and load that instead.
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

-- Restore the environment.
moduleEnd = function (module)
  -- TODO - Look for _NAME.properties, and load it into the modules Things.
  -- TODO - Parse command line parameters at some point.
  --        http://stackoverflow.com/questions/3745047/help-locate-c-sample-code-to-read-lua-command-line-arguments
  if module.isLua then setfenv(2, module.savedEnvironment) end
end

-- Call this now so that from now on, this is like any other module.
local _M = moduleBegin('skang', 'David Seikel', 'Copyright 2014 David Seikel', '0.1', '2014-03-27 02:57:00')
-- TODO - While it is possible to get LuaJIT version info, need to load the 'jit' module, which wont work so well if we are not in LuaJIT.
--        local jit = require 'jit';  jit.version;  jit.version_num;  jit.os;  jit.arch
print('Skang is running under Lua version ' .. _VERSION)



function printTableStart(table, space, name)
    print(space .. name .. ": ")
    print(space .. "{")
    printTable(table, space .. "  ")
    print(space .. "}")
end

function printTable(table, space)
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


--[[ Thing package

matrix-RAD had Thing as the base class of everything.  Lua doesn't have
inheritance as such, but an inheritance structure can be built using
Lua's meta language capabilities.  I think we still need this sort of
thing.  Java inheritance and interfaces where used.  There's quite a few
variations of OO support has been written for Lua, maybe some of that
could be used?  http://lua-users.org/wiki/ObjectOrientedProgramming

Other useful links -

http://lua-users.org/wiki/ClassesViaModules (not in the above for some reason.
http://lua-users.org/wiki/MetamethodsTutorial
http://lua-users.org/wiki/MetatableEvents

http://lua-users.org/wiki/MechanismNotPolicy
http://www.inf.puc-rio.br/~roberto/pil2/chapter15.pdf
http://lua-users.org/lists/lua-l/2011-10/msg00485.html
http://lua-users.org/wiki/LuaModuleFunctionCritiqued

On the other hand, Thing as such might just vanish and merge into
various Lua and metatable things.  Seems that's what is going on.  We
didn't really need much OO beyond this anyway.

Each "users session" (matrix-RAD term that came from Java
applets/servlets) has a ThingSpace, which is a tree that holds
everything else.  It holds the class cache, commands, loaded modules,
variables and their values, widgets and their states.  In matrix-RAD I
built BonsiaTree and LeafLike, for the old FDO system I built dumbtrees. 
Perhaps some combination of the two will work here?  On the other hand,
with Lua tables, who needs trees?  lol

Since skang Lua scripts should be defined as modules, we can use
module semantics instead of get/set -

local other = require('otherPackageName')
other.foo = 'stuff'
bar = other.foo

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


--[[ TODO
  NOTE that skang.thing{} doesn't care what other names you pass in, they all get assigned to the thing.


  Stuff -
      test{'foo', key='blah', field0='something', field1=1, ...}  ->  skang.things.foo.key='blah'  skang.things.foo.field='something' ...
      test{'foo', bar='...', baz='..'}  ->  __call(test, {'foo', ...})  ->  skang.stuff{'foo', ...}
    What we really want though is something like (if foo is defined as a table) -
      ->  skang.things.foo.value = {key='blah', field0='something', field1=1, ...}
    or even (if foo is defined as a Stuff) -
      ->  skang.things.foo.value[blah] = {key = 'blah', field0='something', field1=1, ...}
    Stuff is kinda rigid though, it's fields are fixed, not an everything goes Lua table.  So it needs to be defined.
    Reusing Thing semantics, with aliases.
    Also, let's NOT mix skang.thing{} semantics with test{} semantics, make it a different function.
    Then we can avoid conflicts with Thing.* names, and having to sort out where to put things.
      skang.thing{'foo', stuff={{'key', 'The Stuff key', types='string', required=true},
                                {'field0,b' ... other Thing like stuff ... acl=''},
                                {'field1', 'field1 is a stufflet', 42, 'number'}} }
      skang.thing('foo', stuff=someOtherThing)
      skang.thing('foo', stuff=skang.new(someOtherThing, 'pre'))
      skang.things.foo.things.field0
      skang.things.foo.things.field1
      test.foo[blah].field1


  Widget -
    Merging widgets might work to.  B-)
    This does make the entire "Things with the same name link automatically" deal work easily, since they ARE the same Thing.

    Widgets get a type as well, which would be label, button, edit, grid, etc.
    A grid could even have sub types - grid,number,string,button,date.  B-)
    A required widget might mean that the window HAS to have one.
    Default for a widget could be the default creation arguments - '"Press me", 1, 1, 10, 50'.

	skang.thing('foo,s,fooAlias', 'Foo is a bar, not the drinking type.', function () print('foo') end, '', '"button", "The foo :"' 1, 1, 10, 50')
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


On the other hand, might be better to now think of modules as a type?
How much of OO are we willing to go with here?  lol
In Lua 5.1 and LuaJIT (?) metatables have no __type.
What about userdata?  We could hijack the type() function, and dig deeper if it's a userdata, or even if it's a Thing?


  Things as a metatable field -
    In this plan test.__something is short for metatable(test).__something.

    C can set metatables to non table values.  NOTE - values, not variables, so likely useless.

    __index is used to look up things that don't exist in a table.
    If table isn't actually a table, then use __index as well.
    Actually, in that case, the environment is the "table".
    Either way, if we get to __index, and it's a function, call the function.
    If __index is not a function, then use __index as a table to start the lookup all over again.

    __newindex is similar, it's used if table.key is nil, AND __newindex exists.
    Otherwise, it just sets table.key.
    Again, if __newindow is a table, then start the lookup all over again with that.

    metatable(module).__stuff
    metatable(module).__values

    The Thing is stuffed in a metatable.  It can be just a reference to an existing Thing elsewhere.
    So module is free to be full of random variables that are anything.
    If module.foo is a table, it can have it's own metatable(foo).
    Such a table will use Thing as a proxy table via __index and __newindex, as it does now.

    thing(test, 'foo', 'table'  ...) -> setmetatable(thing, {__index=Thing}); thing stuff goes into thing;  setmetatable(foo,  thing);  test.__stuff[foo] = thing;  test.__values[foo] = {};
    thing(test.foo, 'bar', ...)      -> setmetatable(thing, {__index=Thing});                               setmetatable(foo,  thing);   foo.__stuff[bar] = thing;   foo.__values[bar] = thing.default
    test.foo      ->  test.__index(test, 'foo')        ->  test.__values[foo];  if that's nil, and test.__stuff[foo], then return an empty table instead?
    test.foo(a)   ->  test.__index(test, 'foo')        ->  test.__values[foo](a)
    test.foo(a)   ->  test.__index(test, 'foo')        ->  test.__values[foo](a) (but it's not a function)  ->  test.__values[foo].__call(test.__values[foo], a)
	Doesn't seem useful.
	If test.foo is a Thing then that might be -> test.foo.__call(test.foo, a)  ->  could do something useful with this.
]]



-- Default things values.
-- help		- help text describing this Thing.
-- default	- the default value.  This could be a funcion, making this a command.
-- types	- a comma separated list of types.  The first is the type of the Thing itself, the rest are for multi value Things.  Or argument types for commands.
-- widget	- default widget command arguments for creating this Thing as a widget.
-- required	- "boolean" to say if this thing is required.  TODO - Maybe fold this into types somehow, or acl?
-- acl		- Access Control List defining security restrains.
-- boss		- the Thing or person that owns this Thing, otherwise it is self owned.
Thing.names = {'unknown'}
Thing.help = 'No description supplied.'
Thing.default = ''
Thing.types = {}
Thing.required = false
--Thing.acl = ''
--Thing.boss = ''

Thing.action = 'nada'		-- An optional action to perform.
Thing.tell = ''			-- The skang command that created this Thing.
Thing.pattern = '.*'		-- A pattern to restrict values.

Thing.isReadOnly = false	-- Is this Thing read only?
Thing.isServer = false		-- Is this Thing server side?
Thing.isStub = false		-- Is this Thing a stub?
Thing.isStubbed = false		-- Is this Thing stubbed elsewhere?

Thing.hasCrashed = 0		-- How many times this Thing has crashed.

Thing.append = function (self,data)	-- Append to the value of this Thing.
end

Thing.things = {}		-- The sub things this Thing has, for modules and Stuff.
Thing.errors = {}		-- A list of errors returned by isValid().

Thing.__stuff = {}
Thing.__values = {}

Thing.isValid = function (self, module)	-- Check if this Thing is valid, return resulting error messages in errors.
  -- Anything that overrides this method, should call this super method first.
  local name = self.names[1]
  local modThing = getmetatable(module)
  local thingy = modThing.__stuff[name]
  local key = thingy.names[1];
  local value = modThing.__values[key]

  local t = type(value)
  self.errors = {}
  -- TODO - Naturally there should be formatting functions for stuffing Thing stuff into strings, and overridable output functions.
  if 'nil' == t then
    if self.required then table.insert(self.errors, modThing.names[1] .. '.' .. name .. ' is required!') end
  else
    if self.types[1] ~= t then table.insert(self.errors, modThing.names[1] .. '.' .. name .. ' should be a ' .. self.types[1] .. ', but it is a ' .. type(value) .. '!')
    else
      if 'number' == t then value = '' .. value end
      if ('number' == t) or ('string' == t) then
        if 1 ~= string.find(value, '^' .. self.pattern .. '$') then table.insert(self.errors, modThing.names[1] .. '.' .. name .. ' does not match pattern "' .. self.pattern .. '"!') end
      end
    end
  end
  -- TODO - Should go through self.__stuff[*]:isValid(self) as well.
  return #(self.errors) == 0
end

Thing.remove = function (self)	-- Delete this Thing.
end


Thing.__index = function (module, key)
  -- This only works for keys that don't exist.  By definition a value of nil means it doesn't exist.
  -- TODO - Java skang called isValid() on get().  On the other hand, doesn't seem to call it on set(), but calls it on append().
  --        Ah, it was doing isValid() on setStufflet().
  -- TODO - Call thingy.func() if it exists.

  -- First see if this is a Thing.
  local modThing = getmetatable(module)
  local thingy

  if modThing then
    thingy = modThing.__stuff[key]
    if thingy then
      local name = thingy.names[1];
      return modThing.__values[name] or thingy.default
    end
  end

  -- Then see if we can inherit it from Thing.
  thingy = Thing[key]
  return thingy
end

Thing.__newindex = function (module, key, value)
  -- This only works for keys that don't exist.  By definition a value of nil means it doesn't exist.
  local modThing = getmetatable(module)

  if modThing then
    -- This is a proxy table, the values never exist in the real table.
    local thingy = modThing.__stuff[key]
    if thingy then
      local name = thingy.names[1]
      modThing.__values[name] = value
      if 'function' == type(value) then
        thingy.func = value
      else
        -- NOTE - invalid values are still stored, this is by design.
        if not thingy:isValid(module) then
	  for i, v in ipairs(thingy.errors) do
	    print('ERROR - ' .. v)
	  end
        end
        -- TODO - Go through it's linked things and set them to.
      end
    end
  else
    rawset(module, key, value)		-- Stuff it normally.
  end
end

    -- TODO - Seemed like a good idea at the time, but do we really need it?
--Thing.__call = function (func, ...)
--	return func.func(...)
--    end


-- skang.thing() Creates a new Thing, or changes an existing one.
-- It can be called with positional arguments - (names, help, default, types, widget, required, acl, boss)
-- Or it can be called with a table           - {names, help, pattern='...', acl='rwx'}
-- names	- a comma seperated list of names, aliases, and shortcuts.  The first one is the official name.
--                If this is not a new thing, then only the first one is used to look it up.
--                So to change names, use skang.thing{'oldName', names='newName,otherNewName'}
thing = function (names, ...)
  local params = {...}
  local new = false

  -- Check if it was called as a table, and pull the names out of the table.
  if 'table' == type(names) then
    params = names
    names = params[1]
    table.remove(params, 1)
  end

  -- Break out the names.
  names = csv2table(names)
  local name = names[1]
  local oldNames = {}

  -- TODO - Double check this comment - No need to bitch and return if no names, this will crash for us.

  -- Grab the environment of the calling function if no module was passed in.
  local module = (params.module or params[8]) or getfenv(2)
  local modThing = getmetatable(module)
  if nil == modThing then
    modThing = {}
    modThing.names = {module._NAME}
    setmetatable(module, modThing)
  end
  -- Coz at module creation time, Thing is an empty table, and setmetatable(modThing, {__index = Thing}) doesn't do the same thing.
  -- Also, module might not be an actual module, this might be Stuff.
  if nil == modThing.__stuff then
--    setmetatable(modThing, {__index = Thing})
    modThing.__stuff = {}
    modThing.__values = {}
    modThing.__index = Thing.__index
    modThing.__newindex = Thing.__newindex
  end

  local thingy = modThing.__stuff[name]
  if not thingy then	-- This is a new Thing.
    new = true
    thingy = {}
    thingy.module = module
    thingy.names = names
    setmetatable(thingy, {__index = Thing})
  end

  -- Pull out positional arguments.
  thingy.help		= params[1] or thingy.help
  thingy.default	= params[2] or thingy.default
  local types		= params[3] or table.concat(thingy.types or {}, ',')
  thingy.widget		= params[4] or thingy.widget
  thingy.required	= params[5] or thingy.required
  thingy.acl		= params[6] or thingy.acl
  thingy.boss		= params[7] or thingy.boss

  -- Pull out named arguments.
  for k, v in pairs(params) do
    if 'string' == type(k) then
      if     'types' == k then types = v
      elseif 'names' == k then
        oldNames = thingy.names
        thingy.names = cvs2table(v)
      else                  thingy[k] = v
      end
    end
  end

  thingy.required = isBoolean(thingy.required)

  -- Find type, default to string, then break out the other types.
  local typ = type(thingy.default)
  if 'nil' == typ then typ = 'string' end
  if types then types = typ .. ',' .. types else types = typ end
  thingy.types = csv2table(types)

  -- Remove old names, then stash the Thing under all of it's new names.
  for i, v in ipairs(oldNames) do
    modThing.__stuff[v] = nil
  end
  for i, v in ipairs(thingy.names) do
    modThing.__stuff[v] = thingy
  end

  -- This triggers the Thing.__newindex metamethod above.  If nothing else, it triggers thingy.isValid()
  if new then module[name] = thingy.default end
end


copy = function (module, name)
  local result = {}
  local thingy = {}
  local modThing = getmetatable(module)

  for k, v in pairs(Thing) do
    thingy[k] = v
  end

  thingy.__values = {}
  for k, v in pairs(modThing.__values) do
    thingy.__values[k] = v
  end

  thingy.__stuff = modThing.__stuff
  thingy.names = {name}
  setmetatable(thingy, {__index = Thing})
  setmetatable(result, thingy)

  return result
end


get = function (stuff, key, name)
  local result
  if name then
    local thingy = getmetatable(stuff)
    if thingy then
      local this = thingy.__stuff[key]
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
      local this = thingy.__stuff[key]
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
      local this = thingy.__stuff[key]
      if this then this[name] = value end
    end
  else
    -- In this case, value isn't there, so we are reusing the third argument as the value.
    stuff[key] = name
  end
end


-- TODO - Some function stubs, for now.  Fill them up later.
nada = function () end

clear = function ()
end
window = function (width, height, title)
end

module = function (name)
end
skang = function (name)
end
quit = function ()
end


-- Create our Things here.
-- TODO - For some odd reason, these first three fuck up thing() if we create them before the above stubs.
thing('get',	'Get the current value of an existing Thing or metadata.',	get,	'thing,key,name')
thing('reset',	'Reset the current value of an existing Thing or metadata.',	reset,	'thing,key,name')
thing('set',	'Set the current value of an existing Thing or metadata.',	set,	'thing,key,name,data')
thing('nada',	'Do nothing.',					nada)
thing('clear',	'The current skin is cleared of all widgets.',	clear)
thing('window',	'The size and title of the application Frame.',	window, 'x,y,name', nil, nil, 'GGG')
thing('module',	'Load a module.',				module, 'file,acl')
thing('skang',	'Parse the contents of a skang file or URL.',	skang,	'URL')
thing('quit',	'Quit, exit, remove thyself.',			quit)


moduleEnd(_M)

end

-- NOTE - We have swapped acl and boss around from the Java version, since boss was usually blank.
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
again should go inot the C sandbox.

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
]]


-- Gotta check out this _ENV thing, 5.2 only.  Seems to replace the need for setfenv().  Seems like setfenv should do what we want, and is more backward compatible.
--   "_ENV is not supported directly in 5.1, so its use can prevent a module from remaining compatible with 5.1.
--   Maybe you can simulate _ENV with setfenv and trapping gets/sets to it via __index/__newindex metamethods, or just avoid _ENV."
--   LuaJIT doesn't support _ENV anyway.
