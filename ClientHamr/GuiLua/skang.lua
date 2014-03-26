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


-- There is no ThingSpace, now it's all just in this table, and meta table.  Predefined here coz moduleBegin references Thing.
things = {}
Thing = {}


-- Trying to capture best practices here for creating modules, especially since module() is broken and deprecated.
moduleBegin = function (name, author, copyright, version, timestamp, skin)
  local _M = {}	-- This is what we return to require().
  local level = 2

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

  -- TODO - Should parse in an entire copyright message, and strip that down into bits, to put back together.
  _M.AUTHOR = author
  _M.COPYRIGHT = copyright .. ' ' .. author
  -- TODO - Translate the version number into a version string.
  _M.VERSION = version .. ' lookup version here ' .. timestamp
  -- TODO - If there's no skin passed in, try to find the file skin .. '.skang' and load that instead.
  _M.DEFAULT_SKANG = skin


  --_G[_M._NAME] = _M	-- Stuff it into a global of the same name.
			-- Not such a good idea to stomp on global name space.
			-- It's also redundant coz we get stored in package.loaded[_M._NAME] anyway.
			-- This is why module() is broken.

  setmetatable(_M, Thing)
  _M.savedEnvironment = savedEnvironment
  -- setfenv() sets the environment for the FUNCTION, stack level deep.
  -- The number is the stack level -
  --   0 running thread, 1 current function, 2 function that called this function, etc
  setfenv(level, _M)	-- Use the result for the modules internal global environment, so they don't need to qualify internal names.
			-- Dunno if this causes problems with the do ... end style of joining modules.  It does.  So we need to restore in moduleEnd().
			-- Next question, does this screw with the environment of the skang module?  No it doesn't, coz that's set up at require 'skang' time.

  return _M
end

-- Restore the environment.
moduleEnd = function (module)
  setfenv(2, module.savedEnvironment)
end

-- Call this now so that from now on, this is like any other module.
local _M = moduleBegin('skang', 'David Seikel', '2014', '0.0', '2014-03-19 19:01:00')


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


--[[ TODO - Users might want to use two or more copies of this module.  Keep that in mind.  local a = require 'test', b = require 'test' might handle that though?
    Not unless skang.thing() knows about a and b, which it wont.
    Both a and b get the same table, not different copies of it.
    Perhaps clone the table if it exists?  Only clone the parameters, the rest can be linked back to the original.
    Then we have to deal with widgets linking to specific clones.
    Actually, not sure matrix-RAD solved that either.  lol
]]

Thing.action = 'nada'		-- An optional action to perform.
Thing.tell = ''			-- The skang command that created this Thing.

Thing.isReadOnly = false	-- Is this Thing read only?
Thing.isServer = false		-- Is this Thing server side?
Thing.isStub = false		-- Is this Thing a stub?
Thing.isStubbed = false		-- Is this Thing stubbed elsewhere?

Thing.hasCrashed = 0		-- How many times this Thing has crashed.

Thing.append = function (self,data)	-- Append to the value of this Thing.
end

Thing.errors = {}		-- A list of errors returned by isValid().

Thing.isValid = function (self)	-- Check if this Thing is valid, return resulting error messages in errors.
  -- Anything that overrides this method, should call this super method first.
  local value = self.value
  self.errors = {}
  if 'nil' == type(value) then
    if self.required then table.insert(self.errors, self.names[1] .. ' is required!') end
  else
    if self.types[1] ~= type(value) then table.insert(self.errors, self.names[1] .. ' should be a ' .. self.types[1] .. ', but it is a ' .. type(value) .. '!') end
  end
  -- TODO - Should check for matching mask, and anything else.
  return #(self.errors) == 0
end

Thing.remove = function (self)	-- Delete this Thing.
end

Thing.__index = function (table, key)
  -- This only works for keys that don't exist.  By definition a value of nil means it doesn't exist.
  local thing = things[key]
  -- First see if this is a Thing.
  -- TODO - Java skang called isValid() on get().  On the other hand, doesn't seem to call it on set(), but calls it on append().
  --        Ah, it was doing isValid() on setStufflet().
  if thing then return thing.value or thing.default end

  -- Then see if we can inherit it from Thing.
  thing = Thing[key]
  if thing then return thing end

  -- If all else fails, return nil.
  return nil
end

Thing.__newindex = function (table, key, value)
  -- This only works for keys that don't exist.  By definition a value of nil means it doesn't exist.
  local thing = things[key]

  if thing then
    local name = thing.names[1]
    -- This is a proxy table, the values never exist in the real table.
    thing.value = value
    if 'function' == type(value) then
      thing.func = value
      local types = ''
      for i, v in ipairs(thing.types) do
	if 1 ~= i then types = types .. v .. ', ' end
      end
      print(thing.module._NAME .. '.' .. name .. '(' .. types ..  ') -> ' .. thing.help)
    else
      -- NOTE - invalid values are still stored, this is by design.
      if not thing:isValid() then
	for i, v in ipairs(thing.errors) do
	  print('ERROR - ' .. v)
	end
      end
--      print(thing.types[1] .. ' ' .. thing.module._NAME .. '.' .. name .. ' = ' .. (value or 'nil') .. ' -> ' .. thing.help)
      -- TODO - Go through it's linked things and set them to.
    end
  else
    rawset(table, key, value)		-- Stuff it normally.
  end
end

    -- TODO - Seemed like a good idea at the time, but do we really need it?
--Thing.__call = function (func, ...)
--	return func.func(...)
--    end


-- skang.thing() stashes the default value into _M['bar'], and the details into things['bar'].
-- names	- a comma seperated list of names, aliasas, and shortcuts.  The first one is the official name.
-- help		- help text describing this Thing.
-- default	- the default value.  This could be a funcion, making this a command.
-- types	- a comma separated list of types.  The first is the type of the Thing itself, the rest are for multi value Things.  Or argument types for commands.
-- widget	- default widget command arguments for creating this Thing as a widget.
-- required	- "boolean" to say if this thing is required.  TODO - Maybe fold this into types somehow, or acl?
-- acl		- Access Control List defining security restrains.
-- boss		- the Thing or person that owns this Thing, otherwise it is self owned.
thing = function (module, names, help, default, types, widget, required, acl, boss)
  -- Break out the names.
  local n = {}
  local i = 1
  for v in string.gmatch(names, '([^,]+)') do
    n[i] = v
    i = i + 1
  end
  local name = n[1]

  -- Find type, default to string, then break out the other types.
  local t = {type(default)}
  if 'nil' == t[1] then t[1] = 'string' end
  i = 2
  if types then
    for v in string.gmatch(types, '([^,]+)') do
      t[i] = v
      i = i + 1
    end
  else
    types = ''
  end

  -- Set it all up.
  -- TODO - might want to merge into pre existing Thing instead of over writing like this.  OOOR clone so there's a second copy.
  local thing = {module = module, names = n, help = help, default = default, types = t, widget = widget, required = isBoolean(required), acl = acl, boss = boss, }
  setmetatable(thing, Thing)
  -- Stash the Thing under all of it's names.
  for i, v in ipairs(thing.names) do
    things[v] = thing
  end
  -- This triggers the Thing.__newindex metamethod above.
  module[name] = default
end

--[[ TODO - It might be worth it to combine parameters and commands, since in Lua, functions are first class types like numbers and strings.
        Merging widgets might work to.  B-)
	This does make the entire "Things with the same name link automatically" deal work easily, since they ARE the same Thing.

        Parameter gets a type, which might help since Lua is untyped, versus Java being strongly typed.
        Widgets get a type as well, which would be label, button, edit, grid, etc.
	    A grid could even have sub types - grid,number,string,button,date.  B-)

	Required commands makes no sense, but can just be ignored.
	A required widget might mean that the window HAS to have one.

	Default for a command would be the actual function.
	Default being a function makes this Thing a command.
	Default for a widget could be the default creation arguments - '"Press me", 1, 1, 10, 50'

	skang.thing(_M, 'foo,s,fooAlias', 'Foo is a bar, not the drinking type.', function () print('foo') end, nil, '"button", "The foo :"' 1, 1, 10, 50')
	myButton = skang.widget('foo')	-- Gets the default widget creation arguments.
	myButton:colour(1, 2, 3, 4)
	myEditor = skang.widget('foo', "edit", "Edit foo :", 5, 15, 10, 100)
	myEditor:colour(1, 2, 3, 4, 5, 6, 7, 8)
	myButton = 'Not default'	-- myEditor and _M.foo change to.  Though now _M.foo is a command, not a parameter, so maybe don't change that.
	-- Though the 'quit' Thing could have a function that does quitting, this is just an example of NOT linking to a Thing.
	-- If we had linked to this theoretical 'quit' Thing, then pushing that Quit button would invoke it's Thing function.
	quitter = skang.widget(nil, 'button', 'Quit', 0.5, 0.5, 0.5, 0.5)
	quitter:action('quit')
]]


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

thing(_M, 'nada',	'Do nothing.',					nada)
thing(_M, 'clear',	'The current skin is cleared of all widgets.',	clear)
thing(_M, 'window',	'The size and title of the application Frame.',	window, 'x,y,name', nil, nil, 'GGG')
thing(_M, 'module',	'Load a module.',				module, 'file,acl')
thing(_M, 'skang',	'Parse the contents of a skang file or URL.',	skang,	'URL')
thing(_M, 'quit',	'Quit, exit, remove thyself.',			quit)


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
		{"get", "getThing", "name", "Get the current value of an existing thing.", "", ""},
		{"gimmeskin", "gimmeSkin", "", "Returns the modules default skin.", "", ""},
		{"help", "helpThing", "file", "Show help page.", "", ""},
		{"nada", "nothing", "data", "Does nothing B-).", "", ""},
		{"postshow", "postShowThings", "URL,name", "POST the values of all Things to the URL, show the returned content.", "", ""},
		{"post", "postThings", "URL", "POST the values of all Things to the URL, return the content.", "", ""},
		{"postparse", "postParseThings", "URL", "POST the values of all Things to the URL, parse the returned content.", "", ""},
		{"quiet", "quiet", "", "Output errors and warnings only.", "", ""},
		{"remove", "removeThing", "name", "Remove an existing thing.", "", ""},
		{"sethelp", "setHelp", "name,data", "Change the help for something.", "", ""},
		{"set", "setThing", "name,data", "Set the current value of an existing Thing.", "", ""},
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
