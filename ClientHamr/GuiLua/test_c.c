/* Should be a Lua module, roughly the same as test.lua

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

http://www.lua.org/pil/26.2.html  doesn't say much really, and is for
Lua 5.0

*/
