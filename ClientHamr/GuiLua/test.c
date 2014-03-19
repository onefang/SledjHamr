/* Should be a Lua module, roughly the same as -

local skang = require('skang')
local result = {};
result.author = 'onefang'
result.version = '0.72 alpha 2004-11-19 16:28:00'
local bar
-- The first argument would be the name of a local variable / method.  Which could be accessed via _G?
-- Not sure if we could use a global bar, AND use it directly.
result.bar = skang.newParam('bar', "Required", "Shortcut", "Default", "Help text")
result.func = skang.newCommand('number,data', 'Help Text', function (arg1, arg2)
-- do something here
end)

-- do something here

return result;

*/



/* NOTES -

From http://www.inf.puc-rio.br/~roberto/pil2/chapter15.pdf

"Well-behaved C libraries should export one function called
luaopen_modname, which is the function that require tries to call after
linking the library.  In Section 26.2 we will discuss how to write C
libraries."

The "modname" bit is replaced by the name of the module.  Though if the
module name includes a hyphen, the "require" function strips out the
hyphen and the bit before it.

Though it seems that chapter 26 is not in the same place?

*/