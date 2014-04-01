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


--[[ Stuff package

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


   Squeal -
     squeal.database('db', 'host', 'someDb', 'user', 'password')  ->  Should return a module.
       local db = require 'someDbThing'	  ->  Same as above, only the database details are encodode in the someDbThing source, OR come from someDbThing.properties.
     db:getTable('stuff', 'someTable')	  ->  Grabs the metadata, but not the rows.
     db:read('stuff', 'select * from someTable'}  ->  Fills stuff up with several rows, including setting up the metadata for the columns.
       stuff[1].field1                    ->  Is a Thing, with a stuff in the stuff metatable, that was created automatically from the database meta data.
     stuff:read('someIndex')		  ->  Grabs a single row that has the key 'someIndex', or perhaps multiple rows since this might have SQL under it.
       stuff = db:read('stuff', 'select * from someTable where key='someIndex')

     stuff:write()			  ->  Write all rows to the database table.
     stuff:write(1)			  ->  Write one row  to the database table.
     stuff:stuff('field1').isValid = someFunction	-- Should work, all stuff[key] shares the same stuff.
     stuff:isValid(db)			  ->  Validate the entire stuff against it's metadata at least.
     window.stuff = stuff		  ->  Window gets stuff as it's default stuff, any widgets with same names as the table fields get linked.
     grid.stuff   = stuff		  ->  Replace contents of this grid widget with data from all the rows in stuff.
     choice.stuff = stuff		  ->  As in grid, but only using the keys.
     widget.stuff = stuff:stuff('field1')	  ->  This widget gets a particular stufflet.
       widget would have to look up getmetatable(window.stuff).parent.  Or maybe this should work some other way?

     In all these cases above, stuff is a table that has a Thing metatable, so it has stuff.
       It is also a Stuff.
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


A Thing is a managed variable stored in a parent proxy table, which is usually empty.
So the values stored in this Thing are actually stored in meta(parent)__values[thing].
  parent[thing]          ->  __index   (parent, thing)         ->  meta(parent).__values[thing]
  parent[thing] = value  ->  __newindex(parent, thing, value)  ->  meta(parent).__values[thing] = value


A Stuff is a description table of a Thing that includes -
  names		- An array of names, the first one is the "official" name.
  types		- An array of types, the first one is the "official" type.
  help		- A descriptive text for humans to read.
  default	- The default value.
  widget	- A default widget definition.
  required	- If the Thing is required.
  isValid	- A function that tests if the Thing is valid.
  errors	- Any errors related to the Thing.
  isKeyed	- Is this thing itself a parent for Stuff that is stored under an arbitrary key.
  stuff		- An array of Stuff's for sub Things, so Things that are tables can have their own Things.
  and other things that aren't actually used yet.
All things that a Stuff doesn't have should be inherited from the Thing table.
Stuff's should be able to be easily shared by various Things.


A parent's metatable has __self, which is it's own Stuff.
A parent is free to use it's own name space for anything it wants.
Only the variables it specifies as managed Things are dealt with here.


TODO - 
    test.foo  ->  test.__index(test, 'foo')  ->  test.__values[foo];  if that's nil, and test.stuff[foo], then return an empty table instead?
    stuff.s = {a='foo'}  ->  changes a, deletes everything else, or should.
    Got rid of Stuff.parent, but do we still need a parent pointer?
      Should be in __values I guess.
	__values[key].value
	__values[key].parent
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

Thing.isKeyed = false		-- Is this thing an arbitrarily Keyed table?
Thing.isReadOnly = false	-- Is this Thing read only?
Thing.isServer = false		-- Is this Thing server side?
Thing.isStub = false		-- Is this Thing a stub?
Thing.isStubbed = false		-- Is this Thing stubbed elsewhere?

Thing.hasCrashed = 0		-- How many times this Thing has crashed.

Thing.append = function (self,data)	-- Append to the value of this Thing.
end

Thing.stuff = {}		-- The sub things this Thing has, for modules, tables, and Keyed tables.
Thing.errors = {}		-- A list of errors returned by isValid().

Thing.isValid = function (self, parent)	-- Check if this Thing is valid, return resulting error messages in errors.
  -- Anything that overrides this method, should call this super method first.
  local name = self.names[1]
  local mumThing = getmetatable(parent)
  local value = mumThing.__values[name]

  local t = type(value) or 'nil'
  self.errors = {}
  -- TODO - Naturally there should be formatting functions for stuffing Thing stuff into strings, and overridable output functions.
  if 'nil' == t then
    if self.required then table.insert(self.errors, mumThing.__self.names[1] .. '.' .. name .. ' is required!') end
  else
    if self.types[1] ~= t then table.insert(self.errors, mumThing.__self.names[1] .. '.' .. name .. ' should be a ' .. self.types[1] .. ', but it is a ' .. t .. '!')
    else
      if 'number' == t then value = '' .. value end
      if ('number' == t) or ('string' == t) then
        if 1 ~= string.find(value, '^' .. self.pattern .. '$') then table.insert(self.errors, mumThing.__self.names[1] .. '.' .. name .. ' does not match pattern "' .. self.pattern .. '"!') end
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
end

Thing.remove = function (self)	-- Delete this Thing.
end


Thing.__index = function (parent, key)
  -- This only works for keys that don't exist.  By definition a value of nil means it doesn't exist.
  -- TODO - Java skang called isValid() on get().  On the other hand, doesn't seem to call it on set(), but calls it on append().
  --        Ah, it was doing isValid() on setStufflet().

  -- First see if this is a Thing.
  local mumThing = getmetatable(parent)
  local thingy

  if mumThing and mumThing.__self then
    thingy = mumThing.__self.stuff[key]
    if thingy then
      local name = thingy.names[1];
      return mumThing.__values[name] or thingy.default
    end
  end

  -- Then see if we can inherit it from Thing.
  return Thing[key]
end

Thing.__newindex = function (parent, key, value)
  -- This only works for keys that don't exist.  By definition a value of nil means it doesn't exist.
  local mumThing = getmetatable(parent)

  if mumThing and mumThing.__self then
    -- This is a proxy table, the values never exist in the real table.  In theory.

    -- Find the Thing and get it done.
    local thingy = mumThing.__self.stuff[key]

    if not thingy then
      -- Deal with setting a new Stuff[key].
      if mumThing.__self.isKeyed and (nil == mumThing.__values[key]) then
        local newThing = copy(parent, key)
	rawset(mumThing.__values, key, newThing)
	thingy = {names={key}, types={'table'}, parent=newThing, stuff=getmetatable(newThing).__self.stuff, }
        setmetatable(thingy, {__index = Thing})	-- To pick up isValid, pattern, and the other stuff by default.
	mumThing.__self.stuff[key] = thingy
      end
    end

    if thingy then
      local name = thingy.names[1]
      local valueMeta

      if 'table' == type(value) then
        -- Coz setting it via mumThing screws with the __index stuff somehow.
        local oldValue = mumThing.__values[name]
        if 'table' == type(oldValue) then
          valueMeta = getmetatable(oldValue)
          if valueMeta then
            -- TODO - This wont clear out any values in the old table that are not in the new table.  Should it?
            for k, v in pairs(value) do
              local newK = valueMeta.__self.stuff[k]
              if newK then newK = newK.names[1] else newK = k end
              valueMeta.__values[newK] = v
            end
          end
        end
      end
      if nil == valueMeta then mumThing.__values[name] = value end
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
  end

  rawset(parent, key, value)		-- Stuff it normally.
end

Thing.__call = function (func, ...)
  return thingasm(func, ...)		-- (func, {...})
end


-- skang.thingasm() Creates a new Thing, or changes an existing one.
--[[ It can be called in many different ways -

It can be called with positional arguments - (names, help, default, types, widget, required, acl, boss)
Or it can be called with a table           - {names, help, pattern='...', acl='rwx'}

The first argument can be another Thing (the parent), or a string list of names (see below).

It can be called by itself, with no parent specified -
    thingasm('foo', 'help text)

In this case the surrounding Lua environment becomes the parent of foo.
   If the first argument (or first in the table) is a string, then it's this form.
All others include the parent as the first argument, which would be a table.

It can be called by calling the parent as a function -
    foo('bar', 'some help', types='table')	-- ___call(foo, 'bar', ...)  And now foo is the parent.
    foo.bar{'baz', types='Stuff'}		-- thingasm({foo.bar, 'baz', ...})
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
  local mumThing = getmetatable(parent)
  if nil == mumThing then
    mumThing = {}
    setmetatable(parent, mumThing)
  end
  -- Coz at module creation time, Thing is an empty table, and setmetatable(mumThing, {__index = Thing}) doesn't do the right thing.
  if nil == mumThing.__self then
    mumThing.__self = {stuff={}}
    -- Seems this does not deal with __index and __newindex, and also screws up stuff somehow.
--    setmetatable(mumThing, {__index = Thing})
    mumThing.__self.names = {parent._NAME or 'NoName'}
    if parent._NAME then mumThing.__self.types = {'Module'} end
    mumThing.__values = {}
    mumThing.__index = Thing.__index
    mumThing.__newindex = Thing.__newindex
  end

  local thingy = mumThing.__self.stuff[name]
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

  if 'Keyed' == thingy.types[1] then
    thingy.types[1] = 'table'
    thingy.isKeyed = true
  end
  if 'table' == thingy.types[1] then
    if '' == thingy.default then thingy.default = {} end
    setmetatable(thingy.default, 
      {
        __self = thingy,
        __values = {},
        __index = Thing.__index,
        __newindex = Thing.__newindex,
        __call = Thing.__call, 
      })
  end

  -- Remove old names, then stash the Thing under all of it's new names.
  for i, v in ipairs(oldNames) do
    mumThing.__self.stuff[v] = nil
  end
  for i, v in ipairs(thingy.names) do
    mumThing.__self.stuff[v] = thingy
  end

  -- This triggers the Thing.__newindex metamethod above.  If nothing else, it triggers thingy.isValid()
  if new and not mumThing.__self.isKeyed then parent[name] = thingy.default end
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

thingasm('get',	'Get the current value of an existing Thing or metadata.',	get,	'thing,key,name')
thingasm('reset',	'Reset the current value of an existing Thing or metadata.',	reset,	'thing,key,name')
thingasm('set',	'Set the current value of an existing Thing or metadata.',	set,	'thing,key,name,data')


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

thingasm('nada',	'Do nothing.',					nada)
thingasm('clear',	'The current skin is cleared of all widgets.',	clear)
thingasm('window',	'The size and title of the application Frame.',	window, 'x,y,name', nil, nil, 'GGG')
thingasm('module',	'Load a module.',				module, 'file,acl')
thingasm('skang',	'Parse the contents of a skang file or URL.',	skang,	'URL')
thingasm('quit',	'Quit, exit, remove thyself.',			quit)


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
