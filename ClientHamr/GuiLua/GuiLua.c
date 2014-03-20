/*  GuiLua - a GUI library that implements matrix-RAD style stuff.

Provides the skang and widget Lua packages.

This should be a library in the end, but for now it's just an
application that is a test bed for what goes into the library.  In the
initial intended use case, several applications will be using this all at
once, with one central app hosting all the GUIs.

Basically this should deal with "windows" and their contents.  A
"window" in this case is hosted in the central app as some sort of
internal window, but the user can "tear off" those windows, then they
get their own OS hosted window.  This could be done by the hosting app
sending the current window contents to the original app as a skang file.

Between the actual GUI and the app might be a socket, or a stdin/out
pipe.  Just like matrix-RAD, this should be transparent to the app. 
Also just like matrix-RAD, widgets can be connected to variable /
functions (C or Lua), and any twiddlings with those widgets runs the
function / changes the variable, again transparent to the app, except
for any registered get/set methods.

This interface between the GUI and the app is "skang" files, which are
basically Lua scripts.  The GUI and the app can send skang files back
and forth, usually the app sends actual GUI stuff, and usually the GUI
sends variable twiddles or action calls.  Usually.

To start with, this will be used to support multiple apps hosting their
windows in extantz, allowing the big viewer blob to be split up into
modules.  At some point converting LL XML based UI shit into skang could
be done.  Also, this should be an exntension to LuaSL, so in-world
scripts can have a poper GUI for a change.


NOTES and TODOs -

Making these packages all a sub package of skang seems like a great idea.

See if I can use LuaJIT FFI here.  Since this will be a library, and
skang apps could be written in C or Lua, perhaps writing this library to
be FFI friendly instead of the usual Lua C binding might be the way to
go?

For the "GUI hosted in another app" case, we will need some sort of
internal window manager running in that other app.

This might end up running dozens of Lua scripts, and could use the LuaSL
Lua script running system.  Moving that into this library might be a
sane idea I think?  Or prehaps a separate library that both LuaSL and
GuiLua use?

Raster wants a method of sending Lua tables around as edje messages. 
Between C, Edje, Edje Lua, and Lua.  Sending between threads, and across
sockets.  Using a new edje message type, or eet for sockets, was
suggested, but perhaps Lua skang is a better choice?

Somehow access to the edje_lua2.c bindings should be provided.  And
bindings to the rest of EFL when they are done.  Assuming the other EFL
developers do proper introspection stuff, or let me do it.

The generic Lua binding helper functions I wrote for edje?lua2.c could
be used here as well, and expanded as discussed on the E devs mailing
list.  This would include the thread safe Lua function stuff copied
into the README.

There will eventually be a built in editor, like the zen editor from
matrix-RAD.  It might be a separate app.

NAWS should probably live in here to.  If I ever get around to writing
it.  lol

The pre tokenized widget structure thingy I had planned in the
matrix-RAD TODO just wont work, as it uses symbols.  On the other hand,
we will be using Lua tables anyway.  B-)

*/


/* coordinates and sizes

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

*/


/* thing package

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

Get/set variables would be done here, though the widget package, for
instance, would override this to deal with the UI side, and call the
parents function to deal with the variable side -

foo:set('stuff')
bar = foo:get()

Also, since skang Lua scripts should be defined as modules, we can use
module semantics -

local other = require('otherPackageName')
other.foo = 'stuff'
bar = other.foo

*/


/* stuff & squeal packages

In matrix-RAD Stuff took care of multi value Things, like database rows. 
I'm not sure this is needed here, since Lua has nice tables.  B-)

Squeal was the database driver interface for SquealStuff, the database
version of Stuff.  Maybe we could wrap esskyuehl?  Not really in need of
database stuff for now, but should keep it in mind.

*/


/* skang package

Currently this is in skang.lua, but should bring this in here later.

*/


/* widget package

Should include functions for actually dealing with widgets, plus a way
of creating widgets via introspection.  Should also allow access to
widget internals via table access.  Lua code could look like this -

foo = widget.label(0, "0.1", 0.5, 0, 'Text goes here :")
-- Method style.
foo:colour(255, 255, 255, 0, 0, 100, 255, 0)
foo:hide()
foo:action("skang.load(some/skang/file.skang)")
-- Table style.
foo.action = "skang.load('some/skang/file.skang')"
foo.colour.r = 123
foo.look('some/edje/file/somewhere.edj')
foo.help = 'This is a widget for labelling some foo.'

For widgets with "rows", which was handled by Stuff in skang, we could
maybe use the Lua concat operator via metatable.  I think that works by
having the widget (a table) on one side of the concat or the other, and
the metatable function gets passed left and right sides, then must
return the result.  Needs some experimentation, but this might look like
this -

this.bar = this.bar .. 'new choice'
this.bar = 'new first choice' .. this.bar

*/



/* introspection

As detailed in README, EFL introspection doesn't seem to really be on
the radar, but I might get lucky, or I might have to write it myself. 
For quick and dirty early testing, I'll probably write a widget package
that has hard coded mappings between some basic "label", "button", etc.
and ordinary elementary widgets.  Proper introspection can come later.

*/
