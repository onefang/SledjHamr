-- Wrapping the entire module in do .. end helps if people just join a bunch of modules together, which apparently is popular.
-- By virtue of the fact we are stuffing our result into package.loaded[], just plain running this works as "loading the module".
do	-- Only I'm not gonna indent this.

local skang = require 'skang'
local _M = skang.moduleBegin('test', nil, 'Copyright 2014 David Seikel', '0.1', '2014-03-27 03:57:00', [[
#!/usr/bin/env skang     -- Lua allows this shell hack.

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
local fool

skang.thing('fooble,f', 'Help text goes here', 1, 'number', '"edit", "The fooble:", 1, 1, 10, 50', true)
skang.thing('bar', 'Help text', "Default")
skang.thing('foo')

-- We can use inline functions if we don't need the function internally.
skang.thing('ffunc', 'Help Text', function (arg1, arg2)
  print('Inside test.ffunc(' .. arg1 .. ', ' .. arg2 .. ')')
end, 'number,string')

print('Ending soon')
skang.moduleEnd(_M)

end


-- Test it.
local skang = require 'skang'
local test = require 'test'
local test_c = require 'test_c'

print('End ' .. test.bar .. ' ' .. test.VERSION .. ' ' .. skang.things.ffunc.help .. ' ->> ' .. skang.things.f.action)
print('foo = ' .. test.foo .. ' ->> ' .. skang.things.foo.help)
print('cfunc  ->> ' .. skang.things.cfunc.help)
test.ffunc('one', 2)
test_c.cfunc(0, 'zero')
--skang.things.ffunc('seven', 'aight')
print('')

test.f = 42
print('f is now ' .. test.fooble .. ' ' .. test.f .. ' ' .. skang.things.f.help .. ' ' .. skang.things.fooble.help)
test.f = nil
print('f is now ' .. test.fooble .. ' ' .. test.f)
test.fooble = 42
print('f is now ' .. test.fooble .. ' ' .. test.f)
test.fooble = nil
print('f is now ' .. test.fooble .. ' ' .. test.f)
print('')

print(skang.isBoolean(true))
print(skang.isBoolean(1))
print(skang.isBoolean('1'))
print(skang.isBoolean('true'))
print(skang.isBoolean('Yep'))
print(skang.isBoolean('?'))
print(skang.isBoolean(test))
print(skang.isBoolean(function (a) return true end))
print('')
print(skang.isBoolean(false))
print(skang.isBoolean(nil))
print(skang.isBoolean(0))
print(skang.isBoolean(''))
print(skang.isBoolean('0'))
print(skang.isBoolean('false'))
print(skang.isBoolean('Nope'))
print(skang.isBoolean(function (a) return false end))
print('')

-- Make it required, even though it was anyway.
skang.thing{'f', required = true}
-- First, disable the default value, so we see "is required" errors.
-- Coz using the above syntax means that default is never passed to skang.thing, since it's nil.
skang.things.f.default = nil
test.fooble = 42
test.fooble = 'Should fail.'
test.fooble = 42
test.fooble = nil
test.fooble = nil
test.fooble = 42
test.fooble = true
test.f = 42
test.f = nil
