-- Wrapping the entire module in do .. end helps if people just join a bunch of modules together, which apparently is popular.
-- By virtue of the fact we are stuffing our result into package.loaded[], just plain running this works as "loading the module".
--   TODO - Except for the "passing the name in as ..." part.  B-(
do	-- Only I'm not gonna indent this.

local skang = require 'skang'
--On the other hand, having 'Copyright 2014 David Seikel' here makes the copyright self documenting.  B-)
local _M = skang.moduleBegin('test', 'David Seikel', '2014', '0.0', '2014-03-19 14:01:00', [[
#!skang test.skang     -- This is Lua, so this might not work.

-- There's an implied local this = require 'test'
-- There's an implied local skang = require 'skang'

local widget = require 'widget'
-- local other = require 'otherPackageName'

skang.clear
skang.window(200, 200, "G'day planet.")

quitter = widget.button('Quit', 0.5, 0.5, 0.5, 0.5)
quitter:action('quit')   -- 'quit' is looked up in ThingSpace.commands, and translated into the Lua 'skang.quit()'.

--other.foo = 'stuff'
this.bar = 'things'
this.func(1, 'two')
]])


print('code')

-- A variable that is private to this module.
local foo

skang.newParam(_M, 'bar', "Required", "Shortcut", "Default", "Help text")

-- We can use inline functions if we don't need the function internally.
skang.newCommand(_M, 'func', 'number,data', 'Help Text', function (arg1, arg2)
-- do something here
end)


print('Ending soon')

skang.moduleEnd(_M)

end


-- Test it.
local skang = require 'skang'
local test = package.loaded['test']
print('End ' .. test.bar .. ' ' .. test.VERSION .. ' ' .. skang.ThingSpace.commands.func.help)
