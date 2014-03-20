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

    -- TODO - Should parse in an entire copyright message, and strip that down into bits, to put back together.
    _M.AUTHOR = author
    _M.COPYRIGHT = copyright .. ' ' .. author
    -- TODO - Translate the version number into a version string.
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


--[[
Thing = {}
Thing.__index = Thing	-- So this can be used as the metatable for Things.
So in newParam and newCommand -
    setmetatable(module, Thing)
]]



-- skang.newParam stashes the default value into _M['bar'], and the details into ThingSpace.parameters['bar'].
-- Actually, if it's not required, and there's no default, then skip setting _M['bar'].
-- Could even use _index to skip setting it if it's not required and there is a default.
-- Also should add a metatable, and __newindex() that passes all setting of this variable to skang so it can update other stuff like linked widgets.
-- TODO - If it's not required, and there's no default, then skip setting _M['bar'].
-- TODO - Could even use __index to skip setting it if it's not required and there is a default.
-- TODO - Should add a metatable, and __newindex() that passes all setting of this variable to skang so it can update other stuff like linked widgets.
-- TODO - Users might want to use two or more copies of this module.  Keep that in mind.  local a = require 'test', b = require 'test' might handle that though?
--   Not unless skang.newParam() knows about a and b, which it wont.
--   Both a and b get the same table, not different copies of it.
--   Perhaps clone the table if it exists?  There is no Lua table cloner, would have to write one.  Only clone the parameters, the rest can be linked back to the original.
--   Then we have to deal with widgets linking to specific clones.
--   Actually, not sure matrix-RAD solved that either.  lol
-- TODO - This could even be done with an array of these arguments, not including the _M.
newParam = function (module, name, required, shortcut, default, help, acl, boss)
    module[name] = default
    ThingSpace.parameters[name] = {module = module, name = name, required = required, shortcut = shortcut, default = default, help = help, acl = acl, boss = boss, }
    print(name .. ' -> ' .. shortcut .. ' -> ' .. help)
end

-- skang.newCommand stashes the function into _M['func'], and stashes it all (including the function) into ThingSpace.commands['func'].
-- TODO - Could use __call so that ThingSpace.commands['foo'](arg) works.
newCommand = function (module, name, types, help, func, acl, boss)
    module[name] = func
    ThingSpace.commands[name] = {module = module, name = name, help = help, func = func, acl = acl, boss = boss, }
    print(name .. '(' .. types ..  ') -> ' .. help)
end


-- TODO - Some function stubs, for now.  Fill them up later.
module = function (name)
end
clear = function ()
end
window = function (width, height, title)
end
skang = function (name)
end
get = function (name)
end
set = function (name, value)
end
quit = function ()
end

newCommand(_M, 'module',	'file,acl',	'Load a module.',	module)
newCommand(_M, 'clear',		'',		'The current skin is cleared of all widgets.',			clear)	-- Was in SkangAWT in Java.
newCommand(_M, 'window',	'x,y,name',	'Specifies the size and title of the application Frame.',	window, 'GGG')	-- Was in SkangAWT in Java.
newCommand(_M, 'skang',		'URL',		'Parse the contents of a skang file or URL.',	skang)
newCommand(_M, 'get',		'name',		'Get the current value of an existing thing.',	get)	-- This should be in the modules, not actually here?
newCommand(_M, 'set',		'name,data',	'Set the current value of an existing Thing.',	set)	-- This should be in the modules, not actually here?
newCommand(_M, 'quit',		'',		'Quit, exit, remove thyself.',	quit)


-- Restore the environment.
moduleEnd = function (module)
    setfenv(2, module.savedEnvironment)
end

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
		{"nada", "nothing", "data", "Does nothing B-).", "", ""},
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


-- Gotta check out this _ENV thing, 5.2 only.  Seems to replace the need for setfenv().  Seems like setfenv should do what we want, and is more backward compatible.
--   "_ENV is not supported directly in 5.1, so its use can prevent a module from remaining compatible with 5.1.
--   Maybe you can simulate _ENV with setfenv and trapping gets/sets to it via __index/__newindex metamethods, or just avoid _ENV."
--[[ This is a Lua version of what module() does.  Apparently the _LOADED stuff is needed somehow, even though it's a local?  Think that was bogus.
