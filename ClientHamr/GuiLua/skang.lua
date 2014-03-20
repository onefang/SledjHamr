-- TODO - This should be in C, but so far development has been quite rapid doing it in Lua.
--[[
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


-- Trying to capture best practices here for creating modules, especially since module() is broken and deprecated.

-- Wrapping the entire module in do .. end helps if people just join a bunch of modules together, which apparently is popular.
-- By virtue of the fact we are stuffing our result into package.loaded[], just plain running this works as "loading the module".
--   TODO - Except for the "passing the name in as ..." part.  B-(
do	-- Only I'm not gonna indent this.


local skangModuleBegin = function (name, author, copyright, version, timestamp, skin)
    local _M = {}			-- This is what we return to require().
    _M._NAME = name			-- Catch the module name passed in from require(), so that the file name can change.
    _M._M = _M			-- So that references to _M below the setfenv() actually go to the real _M.
    _M._PACKAGE = string.gsub(_M._NAME, "[^.]*$", "")	-- Strip the name down to the package name.

    _M.AUTHOR = author
    _M.COPYRIGHT = copyright .. ' ' .. author
    _M.VERSION = version .. ' lookup version here ' .. timestamp
    -- TODO - If there's no skin passed in, try to find the file skin .. '.skang' and load that instead.
    _M.DEFAULT_SKANG = skin

    package.loaded[_M._NAME] = _M	-- Stuff the result into where require() can find it, instead of returning it at the end.
				-- Returning it at the end does the same thing.
				-- This is so that we can have all the module stuff at the top.
				-- Should do this before any further require(), so that circular references don't blow out.

    --_G[_M._NAME] = _M		-- Stuff it into a global of the same name.
				-- Not such a good idea to stomp on global name space.
				-- It's also redundant coz we get stored in package.loaded[_M._NAME] anyway.
				-- This is why module() is broken.

    --local _G = _G			-- Stop stuff from here leaking into the callers _G.
				-- Also works around the setfenv() below discarding all the old globals.
				-- Though now we have to use _G.foo to access globals.

    -- An alternative to the local _G is to declare as local ALL our imports here.  May be worthwhile to do both this and local _G?
    -- basic, is it called "basic", might be called "base"?  Might have to include individual names.
    _M.print = print
    _M.getfenv = getfenv
    _M.setfenv = setfenv
    _M.require = require
    _M.coroutine = coroutine	-- Apparently this is part of basic, but it comes in it's own table anyway.
    _M.package = package
    _M.string = string
    _M.table = table
    _M.math = math
    _M.io = io
    _M.os = os
    _M.debug = debug

    _M.savedEnvironment = getfenv(3)
    -- setfenv() sets the environment for the FUNCTION, but we are not in a function.
    -- Though if we are being require()ed, then require() calls a loader, which calls us, hence we are the function.
    -- The number is the stack level -
    --   0 running thread, 1 current function, 2 function that called this function, etc
    setfenv(3, _M)			-- Use the result for our internal global environment, so we don't need to qualify our internal names.
				-- So the below "_M.bar" becomes "bar".  Which might not be what we want, since we are using 
				-- _M.bar for the Thing, not the local bar the Thing wraps.  So we leave _M.bar as is, coz we have _M._M above.  B-)
				-- Since _M is empty at this point, we loose the other globals, but that's we why declare local copies of stuff above.
				-- Dunno if this causes problems with the do ... end style of joining modules.  It does.
				-- Next question, does this screw with the environment of the skang module?  No it doesn't, coz that's set up at require 'skang' time.

    return _M
end

-- This is so the setfenv() stack count above is correct, and we can access ThingSpace in the final moduleBegin() version below, and STILL use this for ourselves.  lol
local smb = function (name, author, copyright, version, timestamp, skin)
    local result = skangModuleBegin(name, author, copyright, version, timestamp, skin)
    return result
end

local _M = smb(..., 'David Seikel', '2014', '0.0', '2014-03-19 19:01:00', nil)

ThingSpace = {}
ThingSpace.cache = {}
ThingSpace.commands = {}
ThingSpace.modules = {}
ThingSpace.parameters = {}
ThingSpace.widgets = {}

moduleBegin = function (name, author, copyright, version, timestamp, skin)
    local result = skangModuleBegin(name, author, copyright, version, timestamp, skin)
    ThingSpace.modules[name] = {module = _M, name = name, }
    return result
end

module = function ()
end
load = function ()
end
clear = function ()
end
window = function (width, height, title)
end
quit = function ()
end

-- skang.newParam stashes the default value into _M['bar'], and the details into ThingSpace.parameters['bar'].
-- Actually, if it's not required, and there's no default, then skip setting _M['bar'].
-- Could even use _index to skip setting it if it's not required and there is a default.
-- Also should add a metatable, and __newindex() that passes all setting of this variable to skang so it can update other stuff like linked widgets.
-- TODO - Users might want to use two or more copies of this module.  Keep that in mind.  local a = require 'test', b = require 'test' might handle that though?
--   Not unless skang.newParam() knows about a and b, which it wont.
--   Both a and b get the same table, not different copies of it.
--   Perhaps clone the table if it exists?  There is no Lua table cloner, would have to write one.  Only clone the parameters, the rest can be linked back to the original.
--   Then we have to deal with widgets linking to specific clones.
--   Actually, not sure matrix-RAD solved that either.  lol
-- This could even be done with an array of these arguments, not including the _M.
newParam = function (module, name, required, shortcut, default, help)
    module[name] = default
    ThingSpace.parameters[name] = {module = module, name = name, required = required, shortcut = shortcut, default = default, help = help, }
    print(name .. ' -> ' .. shortcut .. ' -> ' .. help)
end

-- skang.newCommand stashes the function into _M['func'], and stashes it all (including the function) into ThingSpace.commands['func'].
-- TODO - Could use _call so that ThingSpace.commands['foo'](arg) works.
newCommand = function (module, name, types, help, func)
    module[name] = func
    ThingSpace.commands[name] = {module = module, name = name, help = help, func = func, }
    print(name .. '(' .. types ..  ') -> ' .. help)
end


-- TODO - Some function stubs, for now.  Fill them up later.
module = function (name)
end
clear = function ()
end
window = function (width, height, title)
end
load = function (name)
end
get = function (name)
end
set = function (name, value)
end
quit = function ()
end

newCommand(_M, 'module',	'name',		'',	module)
newCommand(_M, 'clear',		'',		'',	clear)
newCommand(_M, 'window',	'w,h,name',	'',	window)
newCommand(_M, 'load',		'name',		'',	load)
newCommand(_M, 'get',		'name',		'',	get)	-- This should be in the modules, not actually here.
newCommand(_M, 'set',		'name,data',	'',	set)	-- This should be in the modules, not actually here.
newCommand(_M, 'quit',		'',		'',	quit)


-- Restore the environment.
moduleEnd = function (module)
    setfenv(2, module.savedEnvironment)
end

moduleEnd(_M)

end




-- Gotta check out this _ENV thing, 5.2 only.  Seems to replace the need for setfenv().  Seems like setfenv should do what we want, and is more backward compatible.
--   "_ENV is not supported directly in 5.1, so its use can prevent a module from remaining compatible with 5.1.
--   Maybe you can simulate _ENV with setfenv and trapping gets/sets to it via __index/__newindex metamethods, or just avoid _ENV."
--[[ This is a Lua version of what module() does.  Apparently the _LOADED stuff is needed somehow, even though it's a local?  Think that was bogus.

local _LOADED = package.loaded
function _G.module (modname, ...)
  local ns = _LOADED[modname]
  if type(ns) ~= "table" then
    ns = findtable (_G, modname)
    if not ns then
      error (string.format ("name conflict for module '%s'", modname))
    end
    _LOADED[modname] = ns
  end
  if not ns._NAME then
    ns._NAME = modname
    ns._M = ns
    ns._PACKAGE = gsub (modname, "[^.]*$", "")
  end
  setfenv (2, ns)
  for i, f in ipairs (arg) do
    f (ns)
  end
end

]]
