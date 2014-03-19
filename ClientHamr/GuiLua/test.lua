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
