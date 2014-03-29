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

-- TODO - Could have a table of tables, and ipair through the top level, passing the inner ones to skang.thing{}.

skang.thing('fooble,f', 'Help text goes here', 1, nil, '"edit", "The fooble:", 1, 1, 10, 50', true)
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
local copy = skang.copy(test, 'copy')

print('End ' .. test.bar .. ' ' .. test.VERSION .. ' ' .. skang.get(test, 'ffunc', 'help') .. ' ->> ' .. skang.get(test, 'f', 'action'))
print('foo = ' .. test.foo .. ' ->> ' .. skang.get(test, 'foo', 'help'))
print('cfunc  ->> ' .. skang.get(test_c, 'cfunc', 'help'))
test.ffunc('one', 2)
test_c.cfunc(0, 'zero')
print('')

test.f = 42
print('f is now ' .. test.fooble .. ' ' .. test.f)
print('copy_f is now ' .. copy.fooble .. ' ' .. copy.f)
copy.f = 24
print('f is now ' .. test.fooble .. ' ' .. test.f)
print('copy_f is now ' .. copy.fooble .. ' ' .. copy.f)
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
skang.set(test, 'f', 'required', true)
-- Disable the default value, so we see "is required" errors.
skang.reset(test, 'f', 'default')
test.fooble = 42
test.fooble = 'Should fail.'
test.fooble = 42
test.fooble = nil
test.fooble = nil
test.fooble = 42
test.fooble = true
test.f = 42
test.f = nil
print('')

skang.set(test, 'f', 'required', false)
test.f = 42
test.f = nil
skang.set(test, 'f', 'default', 999)
test.f = 42
test.f = nil
print(test.fooble .. ' ' .. test.f)
print(skang.get(test, 'f', 'default'))
print('')

local stuff = {}
stuff.t = {}

skang.thing{'a', module=stuff, help = 'A test stufflet'}
skang.thing{'b', module=stuff.t, help = 'A sub stufflet'}
skang.thing{'c', module=stuff.t, help = 'Another sub stufflet'}
print('*********************************')
skang.fixNames(skang, 'skang')
skang.fixNames(test, 'test')
skang.fixNames(test_c, 'test_c')
skang.fixNames(stuff, 'stuff')
skang.fixNames(stuff.t, 't')
print('*********************************')

print(skang.get(stuff, 'a', 'help'))
print(skang.get(stuff.t, 'b', 'help'))
print(skang.get(stuff.t, 'c', 'help'))
skang.thing{'baz,b', module=test, help = 'A test stufflet for test'}
print(skang.get(test, 'b', 'help'))
print(skang.get(test, 'f', 'help'))
stuff.a = '1'
stuff.t.b = '2'
stuff.t.c = '3'
test.b = '422222'
test.f = 5
test_c.cbar = '666'
-- This one doesn't actually exist.
test_c.bar = '7'
print('')

print(skang.get(stuff, 'a'))
print(skang.get(stuff.t, 'b'))
print(skang.get(stuff.t, 'c'))
print(skang.get(test, 'b'))
print(skang.get(test, 'baz'))
print(skang.get(test, 'f'))
print(skang.get(test, 'fooble'))
print(skang.get(test_c, 'cbar'))
print(skang.get(test_c, 'bar'))
print('')

print(stuff.a)
print(stuff.t.b)
print(stuff.t.c)
print(test.b)
print(test.baz)
print(test.f)
print(test.fooble)
print(test_c.cbar)
print(test_c.bar)
print(test_c.c)
print(test_c.cfooble)
